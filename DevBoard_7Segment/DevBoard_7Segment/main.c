/*
 * DevBoard_7Segment.c
 *
 * Created: 05.03.2018 06:23:35
 * Author : Hm
 */ 

#define F_CPU					16000000

#define DISP_CNT_UP_SPEED		50

#define ALERT_SYS_TEMP			23

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <string.h>

#include "DevBoardSegment.h"
#include "Hardware Libs/hard_init.h"
#include "Hardware Libs/i2cmaster.h"
#include "Hardware Libs/sts3x.h"
#include "Hardware Libs/tmp_102.h"
#include "Hardware Libs/mcp23017.h"
#include "Hardware Libs/rx8564.h"
#include "Hardware Libs/uart.h"
#include "build_info.h"

#include "LED Font/led_font.h"

typedef struct				
{
	/*
	*	Zähler Tabelle für das incrementieren vom Encoder
	*/
	const int8_t Table[16];
	
	/*
	*	Aktueller Zählerstand vom Encoder
	*/
	int8_t		 result;
	
	/*
	*	Letzter Zählerstand vom Encoder (darf nicht weiter benutzt werden!)
	*/
	int8_t		 Last;
	
}enc_t;
volatile enc_t encoder=		
{
	.Table = {0,0,-1,0,0,0,0,1,1,0,0,0,0,-1,0,0},	
};

typedef struct				
{
	/*
	*	I2C ist noch beschäftigt?
	*/
	uint8_t i2cBusy :1;
	
	/*
	*	Systemtemperatur
	*/
	uint8_t temp;
	
	/*
	*	Zähler für das nächste Zeichen
	*/
	uint16_t scroll;
	
	/*
	*	Scrollen ist beendet
	*/
	uint8_t scrollIsRdy;
	
}sys_t;
volatile sys_t sys;

typedef struct				
{
	uint8_t isInit;
}eep_t;
eep_t eep EEMEM;
eep_t eepRAM;


rx8564_t rtc;

void timerInit( void )
{	
	TCCR1B |= ((1<<WGM12) | (1<<CS10)); //CTC Mode 1

	TIMSK  |= (1<<OCIE1A); // OutputCompareEnable
	
	OCR1A   = ( (uint16_t)( F_CPU / 1 / 8000 ) );	
}

uint16_t checkMaxValue( uint16_t val , uint16_t max )
{
	if( val > max )
	{
		return 0;
	}
	return val;
}

void readTemperature( void )
{
	tmp102.actual = tmp102_get_temp();
	sts3x.actual = sts3x_get_temp();
				
	sys.temp = ( tmp102.actual + sts3x.actual ) / 2; 
		
	if ( sts3x.actual < sts3x.lowest )
	{
		sts3x.lowest = sts3x.actual;
	}
	if ( tmp102.actual < tmp102.lowest)
	{
		tmp102.lowest = tmp102.actual;
	}
		
		
	if ( sts3x.actual > sts3x.highest )
	{
		sts3x.highest = sts3x.actual;
	}
	if ( tmp102.actual > tmp102.highest )
	{
		tmp102.highest = tmp102.actual;
	}		
}

void setTime( uint8_t *buff )
{
	#define TIME_OFFSET 35

	uint32_t _delay = 0;
	
	rtcGetData(&rtc);
 	buff[0] = bcdToDec( rtc.hour );
 	buff[1] = bcdToDec( rtc.minute );
	
	for ( uint8_t x = 0 ; x < 2 ; x++ )
	{
		encoder.result = 0;
		
		while (1)
		{
			buff[x] += encoder.result;
			
			if (encoder.result)
			{
				encoder.result = 0;
				_delay = 0;
			}
			
			switch(x)
			{
				case 0: {buff[x] = checkMaxValue(buff[x],24);}break;
				case 1: {buff[x] = checkMaxValue(buff[x],59);}break;
			}
			
			if ( _delay++ < 10000 )
			{
				timeDecToRam( buff[0] , buff[1] );
			}
			else if ( _delay > 40000 )
			{
				switch(x)
				{
					case 0:
					{
						timeDecToRam( DISP_SPACE_CHAR , buff[1] );
					}break;
					
					case 1:
					{
						timeDecToRam( buff[0] , DISP_SPACE_CHAR );
					}break;
				}
			}
			_delay = checkMaxValue(_delay,65000);

			if(!(ENC_SWITCH_PORT & 1<<ENC_SWITCH_bp))
			{
				_delay_ms(25);
				if ((ENC_SWITCH_PORT & 1<<ENC_SWITCH_bp))
				{
					break;
				}
			}
		}
	}
	
	for ( uint8_t i = 0 ; i < 4 ; i++)
	{
		disp.dig[i] = ' ';
	}
	
	sys.i2cBusy = 1;
	rtcSetTime( buff[0] , buff[1] , 0 );
	sys.i2cBusy = 0;
}

void checkForSetTime( void )
{
	uint8_t buff[3] = "";
	
	if ( ! (ENC_SWITCH_PORT & (1<<ENC_SWITCH_bp) ) )
	{
		_delay_ms(100);
		while ( ( ! (ENC_SWITCH_PORT & 1<<ENC_SWITCH_bp) ) )
		{
			for ( uint8_t i = 0 ; i < 4 ; i++ )
			{
				disp.dig[i] = '0';
			}
			
			_delay_ms(150);
			
			for ( uint8_t i = 0 ; i < 4 ; i++ )
			{
				disp.dig[i] = ' ';
			}
			
			_delay_ms(150);
			
		}
		setTime( buff );
	}	
}

void setRelais(uint8_t Kx)
{	
	/*
	*	Setze Relais
	*/
	RELAIS_PORT1_PORT = (Kx<<2);
	RELAIS_PORT2_PORT = ((Kx&0xC0)>>4);
}

void eepLoad( eep_t *e )
{
	if ( eeprom_read_byte( &e->isInit ) != 0xA2 )
	{
		eepRAM.isInit = 0x00;
		eeprom_write_byte( &e->isInit , 0xA2 );
	}
	else
	{
		eepRAM.isInit = 0x01;
	}
}

void scrollMessage( char *msg , uint16_t delay_ms , uint8_t wait )
{
	uint8_t leng = strlen( msg );
	static int8_t pos = -DIGITS;
	uint8_t w_ = wait;
	
	selectFont( (char**) charMap );
	
	do 
	{
		if ( sys.scroll >= delay_ms && !sys.scrollIsRdy )
		{
			for ( int8_t ramPos = 0 ; ramPos < DIGITS ; ramPos++ )
			{
				if ( (pos + ramPos) < 0 )
				{
					disp.dig[ramPos] = ' '; // Leerzeichen
				}
				else if ( (pos + ramPos) > leng - 1 )
				{
					disp.dig[ramPos] = ' '; // Leerzeichen
				}
				else
				{
					disp.dig[ramPos] = msg[ ramPos + pos ];
				}
			}

			if( pos == leng )
			{
				pos = -DIGITS;
				sys.scrollIsRdy = 1;
				w_ = 0;
				break;
			}
			else
			{
				pos += 1;
			}	
		
			sys.scroll	= 0;
		}		
	} while ( w_ );
}

void testHardware( void )
{
	uint8_t tmp = 0;

	selectFont( (char**) charMap );

	disp.dig[0] = '-';
	disp.dig[1] = '-';
	disp.dig[2] = '-';
	disp.dig[3] = '-';
	
	_delay_ms(500);
	
	while ( ! ( SWITCH_PIN & 1<<SWITCH_ENTER_bp ) ) {}
	
	while ( SWITCH_PIN & 1<<SWITCH_ENTER_bp )
	{
		selectFont( (char**) segments );
		
		for (uint8_t x = '0' ; x < '8' ; x++)
		{
			disp.dig[0] = x;
			disp.dig[1] = x;
			disp.dig[2] = x;
			disp.dig[3] = x;
			setRelais( tmp = 1<<( x - '0' ) );
			_delay_ms(500);
		}tmp = 0;
		
		clearDigits();
		
		setRelais( 0 );
		VIB_ON;
		_delay_ms(40);
		VIB_OFF;
		
		sys.i2cBusy = 1;
		for ( uint8_t x = 0 ; x < 5 ; x++ )
		{
			refEnable(1);
			_delay_ms(50);
			refEnable(0);
			_delay_ms(50);
		}
		sys.i2cBusy = 0;
		
		sys.scrollIsRdy = 0;
		scrollMessage( "8888" , 2500 , 1 );
		sys.scrollIsRdy = 0;
		
		uart_puts( "**HeschDevBoard_7Segment**\r\n");
		uart_puts( "Support by J.H - Elec.\r\n");
		uart_puts( "www.jh-elec.de\r\n" );
		uart_puts( "Ver.: " );
		uart_puts( buildVer() );
		uart_puts( "\r\n" );
		uart_puts( "Testmode..\r\n" );
		uart_puts( "\r\n" );
	}
}

char out[60] = "";

int main(void)
{
	hardware_init();
	uart_init( UART_BAUD_SELECT( 9600 , F_CPU ) );
	
	eepLoad( &eep );
	i2c_init();
	mcp23017_init();
	sts3x_init();
	timerInit();

	sei();

	uart_puts( "**HeschDevBoard_7Segment**\r\n");
	uart_puts( "Support by J.H - Elec.\r\n");
	uart_puts( "www.jh-elec.de\r\n" );
	uart_puts( "Ver.: " );
	uart_puts( buildVer() );

	checkForSetTime();
	
	if ( ! (SWITCH_PIN & 1<<SWITCH_ENTER_bp) )
	{
		testHardware();
	}		
	
	strcpy( out , "Ver:");
	strcat( out , buildVer() );
	scrollMessage( out , 3000 , 1 );
	
	/*
	*	Wird gesetzt damit direkt nach dem einschalten
	*	das erste mal die Temperatur + Uhrzeit gelesen wird.
	*/
	sys.scrollIsRdy = 1;

	while (1) 
    {	
			
		if ( sys.scrollIsRdy )
		{
			sys.i2cBusy = 1; // Multiplexing unterbrechen
			readTemperature(); 
			rtcGetData( &rtc );
			sys.i2cBusy = 0; // Multiplexing wieder freigeben
			sys.scrollIsRdy = 0; // Es darf wieder gescrollt werden
		}
		
 		strcpy( out , tempToStr( sts3x.actual , 0 ) );
 		strcat( out , " - " );
 		strcat( out , tempToStr( tmp102.actual , 1 ) );
 		strcat( out , " - " );
 		strcat( out , timeBcdToStr( rtc.hour , rtc.minute , rtc.second ) );				
		scrollMessage( out , 2000 , 0 );
		
    }
}

/* live the CPU?*/
ISR(TIMER1_COMPA_vect)
{
	static uint32_t stateLED = 0;
	sys.scroll++;
	
	/*
	*	Status Anzeige
	*/
	stateLED = checkMaxValue( ++stateLED , 10e3 );
	
	if ( stateLED <= 200 )
	{
		BC(LED_HEARTBEAT_PORT,LED_HEARTBEAT_bp);
	}
	else if ( ( stateLED >= 400 ) && ( stateLED <= 600 ) )
	{
		BS(LED_HEARTBEAT_PORT,LED_HEARTBEAT_bp);
	}
	else if ( ( stateLED >= 1000 ) && ( stateLED <= 1400 ) )
	{
		BC(LED_HEARTBEAT_PORT,LED_HEARTBEAT_bp);
	}
	else if ( ( stateLED >= 1600 ) && ( stateLED <= 1800 ) )
	{
		BS(LED_HEARTBEAT_PORT,LED_HEARTBEAT_bp);
	}
	
	/*
	*	Drehencoder auswertung.
	*/
	encoder.Last = ( ( encoder.Last << 2 ) & 0x0F );
	if (ENC_PIN & 1<<ENC_B)
	{
		encoder.Last |= 2;
	}
	if (ENC_PIN & 1<<ENC_A)
	{
		encoder.Last |= 1;
	} 	
	encoder.result += encoder.Table[encoder.Last];	
	
	if ( !sys.i2cBusy )
	{
		muxDigits();	
	}
}