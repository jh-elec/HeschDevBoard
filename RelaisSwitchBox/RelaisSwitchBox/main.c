/*
 * DevBoard_7Segment.c
 *
 * Created: 05.03.2018 06:23:35
 * Author : Hm
 */ 

#define F_CPU					16000000

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <stdio.h>

#include "hard_def.h"
#include "Hardware Libs/hard_init.h"
#include "Hardware Libs/uart.h"
#include "build_info.h"
#include "cmd.h"


char out[20] = "";


void *cmdRelais	( void *ptr , void *ptr_ );
void *uartTest	( void *ptr , void *ptr_ );


const cmdTable_t cmdTab[] =
{
	{"Relais Handler:" 			, 	"-RELais"		, 	cmdRelais	},
	{"UART Handler:"			,	"-TEST"			,	uartTest	},
};

cmd_t cmd =
{
	.table	= cmdTab,
	.tabLen = sizeof( cmdTab ) / sizeof( *cmdTab ),
	.raw 	= &raw
};

void *cmdRelais( void *ptr , void *ptr_ )
{ 
 	uint8_t kx;
 	uint8_t state;
	char para[10] = "";
	 	 
	// -RELais:2,1,#21;
 	kx		= atoi( cmdGetPara( &cmd , para , ptr , 0 ) ); 
 	state	= atoi( cmdGetPara( &cmd , para , ptr , 1 ) );
	
	uart_puts( "Relais: " );
	uart_putc( kx + '0' );
	
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
			uart_puts( " unset" );
					
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
			uart_puts( " set" );
		}break;
	}
	
	uart_puts( "\r\n" );

	return NULL;
}

void *uartTest( void *ptr , void *ptr_ )
{
	char buff[30]="";
	uart_puts( "Loopback.: " );
	uart_puts( cmdGetPara( &cmd , buff , ptr , 0 ) );
	uart_puts( "\r\n" );
	
	return NULL;
}

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

char *readRingBuff( char *stream )
{
	static uint8_t index = 0;
	
	/*
	*	Neustes Byte aus dem Ringpuffer abholen
	*/
	uint16_t c = uart_getc();
	
	/*
	*	Wenn keine neuen Daten vorhanden, direkt wieder zurück!
	*/
	if ( c & UART_NO_DATA)
	{
		return NULL;
	}
	
	/*
	*	Ist ein Fehler aufgetreten?
	*/
	if ( c > UART_NO_DATA)
	{
		index = 0; 
		return NULL;
	}
	
	/*
	*	Status und Error Bits ausmaskieren
	*/
	c &= 0x00FF;
	
	/*
	*	Übertragungsende?
	*/
	if (c == '\n' || c == '\r')
	{
		*( stream + index ) = '\0';
		index = 0; 
		
		cmdCntPara( &cmd , stream );
		
		return stream;
	}
	
	/*
	*	Daten in unseren neuen Puffer zwischen speichern
	*/
	if (index >= UART_RX_BUFFER_SIZE)
	{
		index = 0;
	}
	else
	{
		*( stream + index++ ) = (uint8_t)c;	
	}
	
	return NULL;
}



int main(void)
{
	hardware_init();
	uart_init( UART_BAUD_SELECT( 9600 , F_CPU ) );
	
	timerInit();

	sei();

	uart_puts( "**HeschDevBoard_7Segment**\r\n");
	uart_puts( "Support by J.H - Elec.\r\n");
	uart_puts( "www.jh-elec.de\r\n" );
	uart_puts( "Ver.: " );
	uart_puts( buildVer() );
	uart_puts( "\r\n\n" );
	
	while (1) 
    {	
		char		*stream = readRingBuff( out );
		const char	*cmdPtr = NULL;
		if ( stream != NULL )
		{
			cmdPtr = cmdGetName( &cmd , stream );
			if ( cmdPtr != NULL  )
			{
				uart_puts( "****************************\r\n" );

				uart_puts( cmdPtr );
				uart_puts( "\r\n" );	
			
				if ( cmd.raw->paraNumb != 0 )
				{
 					uart_puts( "Anzahl Parameter.: " );
 					uart_puts( itoa( cmd.raw->paraNumb , NULL , 10 ) );
 					uart_puts( "\r\n" );			
				}
		
				void (*funcPtr)(void*,void*) = cmdGetFunc( &cmd , stream );
				if ( funcPtr != NULL )
				{
					funcPtr( stream , NULL );
				}
			
				char crc[5] = "";
				char *crcPtr = cmdGetCRC( crc , stream );
				if ( crcPtr != NULL )
				{
					uart_puts( "CRC.: " );
					uart_puts( crcPtr );
					uart_puts( "\r\n" );
				}
			
				memset( ( char * ) out , 0 , strlen( out ) );
			
				uart_puts( "****************************\r\n" );				
			}
			else
			{
				uart_puts( "no command\r\n" );
			}
		}		
    }
}

/* live the CPU?*/
ISR(TIMER1_COMPA_vect)
{
	static uint32_t stateLED = 0;
	
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
}