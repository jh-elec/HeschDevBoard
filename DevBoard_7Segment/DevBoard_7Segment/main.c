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
#include <stdbool.h>
#include <stdio.h>

#include "DevBoardSegment.h"
#include "Hardware Libs/hard_init.h"
#include "Hardware Libs/i2cmaster.h"
#include "Hardware Libs/sts3x.h"
#include "Hardware Libs/tmp102.h"
#include "Hardware Libs/mcp23017.h"
#include "Hardware Libs/rx8564.h"
#include "Hardware Libs/uart.h"
#include "build_info.h"
#include "cmd.h"
#include "LED Font/led_font.h"
#include "Switch.h"

char out[60] = "";

typedef struct				
{
	/*
	*	Z�hler Tabelle f�r das incrementieren vom Encoder
	*/
	const int8_t Table[16];
	
	/*
	*	Aktueller Z�hlerstand vom Encoder
	*/
	int8_t		 result;
	
	/*
	*	Letzter Z�hlerstand vom Encoder (darf nicht weiter benutzt werden!)
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
	*	I2C ist noch besch�ftigt?
	*/
	uint8_t i2cBusy :1;
	
	/*
	*	Systemtemperatur
	*/
	uint8_t temp;
	
	/*
	*	Z�hler f�r das n�chste Zeichen
	*/
	uint16_t scroll;
	
	/*
	*	Scrollen ist beendet
	*/
	uint8_t scrollIsRdy;
	
	struct  
	{
		uint16_t error;
		uint16_t okay;
	}crc;
	
	uint16_t cmdCounter;
		
}sys_t;
volatile sys_t sys;

typedef struct				
{
	uint8_t isInit;
}eep_t;
eep_t eep EEMEM;
eep_t eepRAM;

rx8564_t rtc;
cmd_t cmd;

bool DebugModeEnable;

void		checkTempSwell	( void )
{
	sys.temp = tmp102.actual;
	
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

uint8_t		cmdPing			( cmd_t *c )
{
	cmdBuildAnswer( &cmd , ID_PING , DATA_TYP_STRING , 0 , 4 , (uint8_t*)"ping" );
	cmdSendAnswer( &cmd );	

	return 0;
}

uint8_t		cmdGetVersion	( cmd_t *c )
{
	uint8_t *version = (uint8_t*)buildVer();
	
	cmdBuildAnswer( &cmd , ID_VERSION , DATA_TYP_STRING , 0 , 17 , version );
	cmdSendAnswer( &cmd );
	
	return 0;
}

uint8_t		cmdDisplay		( cmd_t *c )
{
	selectFont( (char**) charMap );
	
	switch ( *c->DataPtr++ )
	{
		case 0:
		{
			selectFont( (char**) segments );
			for ( uint8_t x = 0 ; x < 7 ; x++ )
			{
				disp.dig[0] = (x + '0');
				disp.dig[1] = (x + '0');
				disp.dig[2] = (x + '0');
				disp.dig[3] = (x + '0');
				_delay_ms(500);
			}			
		}break;
		
		case 1:
		{
			for ( uint8_t x = 0 ; x < 4 ; x++ )
			{
				disp.dig[x] = (*c->DataPtr++) + '0';
			}				
			_delay_ms(1500);
		}break;
	}


	
	cmdBuildAnswer( &cmd , 2 , DATA_TYP_UINT8 , 0 , 0 , NULL );
	cmdSendAnswer( &cmd );
		
	return 0;
}

uint8_t		cmdSetTime		( cmd_t *c )
{
	if (c->DataLength != 3) return 1;
	
	uint8_t hour	= *c->DataPtr++;
	uint8_t minute	= *c->DataPtr++;
	uint8_t secound = *c->DataPtr;
	
	rtcSetTime(hour , minute ,secound );
	
	cmdBuildAnswer( &cmd , 3 , DATA_TYP_UINT8 , 0 , 0 , NULL );
	cmdSendAnswer( &cmd );
	
	return 0;
}

uint8_t		cmdReadRtc		( cmd_t *c )
{	
	uint8_t buff[] =
	{
		rtcBcdToDec( rtc.hour	),
		rtcBcdToDec( rtc.minute ),
		rtcBcdToDec( rtc.second ),
		
		rtcBcdToDec( rtc.day	),
		rtcBcdToDec( rtc.month	),
		rtcBcdToDec( rtc.year	),
	};
	
	cmdBuildAnswer( &cmd , 4 , DATA_TYP_UINT8 , 0 , sizeof(buff) , buff );
	cmdSendAnswer( &cmd );
		
	return 0;
}

uint8_t		cmdRelais		( cmd_t *c )
{
	uint16_t		kx;
	uint8_t			state;
	
	kx		= c->DataPtr[0];
	state	= c->DataPtr[1];

	switch ( state )
	{
		case 0 :
		{
			if ( kx < 7 )
			{
				RELAIS_PORT1_PORT &= ~( 1 << ( ( kx ) + 1 ) );
			}
			else
			{
				RELAIS_PORT2_PORT &= ~(1 << ( kx - 5 ));
			}
		}break;
		
		case 1 :
		{
			if ( kx < 7 )
			{
				RELAIS_PORT1_PORT |= ( 1 << ( ( kx ) + 1 ) );
			}
			else
			{
				RELAIS_PORT2_PORT |= 1 << ( kx - 5 );
			}
		}break;
		
		case 2 :
		{
			RELAIS_PORT1_PORT = kx << 2;
			RELAIS_PORT2_PORT = ( kx & 0xC0 ) >> 4;
			
		}break;
	}

	uint8_t buff[] =
	{
		kx,
		state,
	};

	cmdBuildAnswer( &cmd , 5 , DATA_TYP_UINT8 , 0 , sizeof(buff) , buff );
	cmdSendAnswer( &cmd );

	return 0;
}

uint8_t		cmdGetTemp		( cmd_t *c )
{
	int8_t buff[] =
	{		
		sts3x.actual,
		tmp102.actual,
		
		sts3x.highest,
		sts3x.lowest,
				
		tmp102.highest,
		tmp102.lowest,
	};
	
	cmdBuildAnswer( &cmd , 6, DATA_TYP_INT8 , 0 , sizeof(buff) , (uint8_t*)buff );
	cmdSendAnswer( &cmd );
	
	return 0;
}

uint8_t		cmdGetState		( cmd_t *c )
{
	uint8_t buff[] =
	{
		/*
		*	CRC Fehler
		*/
		sys.crc.error & 0x00FF,
		((sys.crc.error & 0xFF00) >> 8),

		
		/*
		*	Kommandoz�hler
		*/
		sys.cmdCounter & 0x00FF,
		((sys.cmdCounter & 0xFF00) >> 8),
		
		MCUCSR,	
	};
	
	cmdBuildAnswer( &cmd , 7 , DATA_TYP_UINT8 , 0 , sizeof(buff) , buff );
	cmdSendAnswer( &cmd );
	
	return 0;
}

uint8_t		cmdSetDebugMode ( cmd_t *c )
{
	if ( DebugModeEnable == false )
	{
		DebugModeEnable = true;
		cmdBuildAnswer( &cmd , ID_APPLICATION , DATA_TYP_STRING , 0 , 13 , (uint8_t*)"Debug_Mode_En");
		for (uint8_t x = 0 ; x < 4 ; x++)
		{
			disp.dig[x] = '0';
		}
	}
	else
	{
		DebugModeEnable = false;
		cmdBuildAnswer( &cmd , ID_APPLICATION , DATA_TYP_STRING , 0 , 13 , (uint8_t*)"Debug_Mode_Di");
	}
	
	cmdSendAnswer( &cmd );
	
	return 0;
}

uint8_t		cmdDebug		( cmd_t *c )
{

	return 0;
}

const cmdFuncTab_t cmdTab[] =
{
	{	cmdPing			},//0
	{	cmdGetVersion	},//1
	{	cmdDisplay		},//2
	{	cmdSetTime		},//3
	{	cmdReadRtc		},//4
	{	cmdRelais		},//5
	{	cmdGetTemp		},//6
	{	cmdGetState		},//7
	{	cmdSetDebugMode	},//8
	{	cmdDebug		},//9
};



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

void DecimalToBinaerStr( uint8_t uiValue , char *s )
{
	uint8_t tmp = uiValue;
	
	for ( uint8_t i = 0 ; i < 8 ; i++ )
	{
		if ( tmp & 0x80 )
		{
			s[i] = '1';
		}
		else
		{
			s[i] = '0';
		}
		tmp <<=1;
	}s[8] = '\0';
}

uint8_t DebugModeThousend = 0;

uint8_t *streamPtr		= NULL;
uint8_t	streamIn[128]	= "";

uint8_t RelaisState = 0;

Switch_t Switch;

int main(void)
{
	hardware_init();
<<<<<<< .mine
	uartInit( UART_BAUD_SELECT( 19200 , F_CPU ) );
||||||| .r47
	uart_init( UART_BAUD_SELECT( 19200 , F_CPU ) );
=======
	
	SwitchInit( &PORTB , 0b00001111 , &Switch );
	
	uart_init( UART_BAUD_SELECT( 19200 , F_CPU ) );
>>>>>>> .r50
	
	eepLoad( &eep );
	i2c_init();
	mcp23017_init();

	sts3xInit();
	timerInit();
	cmdInit( &cmd );

	sei();
	
	cmdBuildAnswer( &cmd , ID_APPLICATION , DATA_TYP_STRING , 0 , 4 , (uint8_t*)"Boot");
	cmdSendAnswer( &cmd );		

	cmdGetVersion( &cmd );

	while (1) 
    {			
		streamPtr	= uartReadRingBuff( streamIn , 0 );

		if ( streamPtr != NULL )
		{
			uint8_t ParserState = cmdParse( streamPtr , &cmd );
			
			switch ( ParserState )
			{
				case 0:
				{
					cmdTab[cmd.MessageID].fnc( &cmd );
					sys.cmdCounter++;
					RelaisState ^= 1<<0;
					if ( RelaisState & 1<<0 )
					{
						RELAIS_PORT2_PORT &= ~(RELAIS_7_PORT2 | RELAIS_8_PORT2);
						RELAIS_PORT1_PORT = (RELAIS_1_PORT1 | RELAIS_2_PORT1 | RELAIS_3_PORT1 | RELAIS_4_PORT1);
					}
					else
					{
	 					RELAIS_PORT1_PORT = (RELAIS_5_PORT1 | RELAIS_6_PORT1);
						RELAIS_PORT2_PORT = (RELAIS_7_PORT2 | RELAIS_8_PORT2);
					}
									
				}break;
				
				case 3:
				{
					uartReadRingBuff( NULL , 0x01 );
					cmdBuildAnswer(&cmd,ID_APPLICATION,DATA_TYP_UINT8,4,0,NULL);
					cmdSendAnswer(&cmd);
				}
			}	
		}

		if ( sys.scrollIsRdy && DebugModeEnable == false )
		{
			sys.i2cBusy = 1; // Multiplexing unterbrechen
			
			sts3x.actual	= sts3xGetTemp();
			tmp102.actual	= tmp102GetTemp();
			checkTempSwell();
			
			rtcGetData( &rtc );
			
			sys.i2cBusy = 0; // Multiplexing wieder freigeben
			sys.scrollIsRdy = 0; // Es darf wieder gescrollt werden
		}
		
		if ( DebugModeEnable == false )
		{
			char tmp[6] = "";
 			strcpy( out , buildTemperatureString( tmp , sts3x.actual , 0 ) );
 			strcat( out , " - " );
 			strcat( out , buildTemperatureString( tmp , tmp102.actual , 1 ) );
 			strcat( out , " - " );
 			strcat( out , timeBcdToStr( rtc.hour , rtc.minute , rtc.second ) );
 			scrollMessage( out , 1500 , 0 );			
		}
		else
		{
			disp.dig[0] = ((sys.cmdCounter / 1000) % 10) + '0';
			disp.dig[1] = ((sys.cmdCounter / 100) % 10) + '0';
			disp.dig[2] = ((sys.cmdCounter / 10) % 10) + '0';
			disp.dig[3] = (sys.cmdCounter  % 10) + '0';
		}
	

			
			if ( Switch.Info & 1<<0 )
			{
				Switch.Info &= ~(1<<0);
				DebugModeEnable ^= true;
			}else if ( Switch.Info & 1<<2 )
			{
				Switch.Info &= ~(1<<2);
				sys.cmdCounter--;
			}else if ( Switch.Info & 1<<3 )
			{
				Switch.Info &= ~(1<<3);
				sys.cmdCounter++;
			}else if ( Switch.Info & 1<<1 )
			{
				Switch.Info &= ~(1<<1);
				cmd_t TestRelais;
				static int8_t Relais = 1;
				static uint8_t SetState = 0x01;
				uint8_t Buffer[] = { Relais , SetState };
				TestRelais.DataPtr = Buffer;
				cmdRelais( &TestRelais );
				
				if ( Relais < 8 )
				{
					Relais++;
					SetState = 1;
				}
				else
				{
					Relais = 0;
					SetState = 2;
				}
			}
   }
}

/* live the CPU?*/
ISR(TIMER1_COMPA_vect)
{
	static uint32_t stateLED = 0;
	sys.scroll++;
	
	SwitchRead( &Switch , &PORTB );
	
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