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


char streamIn[64] = "";

void		*cmdRelais	( void *ptr , void *ptr_ )			
{ 
 	uint8_t kx;
 	uint8_t state;
	char para[10] = "";
	 	 
	/*	Relais Handler
	*	Einzel Bit Modus: 
	*				Kommando: -k:1,0; // Relais 1 , rücksetzen
	*						  -k:1,1; // Relais 1 , setzen
	*/
 	kx		= atoi( cmdGetPara( para , ptr , 0 ) ); 
 	state	= atoi( cmdGetPara( para , ptr , 1 ) );
		
	uart_puts( cmdGetPara( para , ptr , 1 ) );
		
	if ( state > 2 )
	{
		//return NULL;
	}
		
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
		
		/*	Relais Handler
		*	Byte Modus: 
		*				Kommando: -k:170,3;
		*	Hier können direkt mehrere Relais mit einem Byte gesetzt werden
		*/
		case 2 :
		{
			uart_puts( "\r\n**Byte Modus**\r\n" );
			RELAIS_PORT1_PORT = kx << 2;
			RELAIS_PORT2_PORT = ( kx & 0xC0 ) >> 4;
		}break;
	}

	return ( char * ) '0';
}
	
void		*help		( void *ptr , void *ptr_ )			
{
	static char helpBuff[250] = "";
	
	uart_puts( "Ver.: " );
	uart_puts( buildVer() );
	uart_puts( "\r\n" );
	
	uart_puts( cmdHelp( helpBuff ) );
	
	return NULL;
}

const		cmdTable_t	cmdTab[] =							
{
	{"Relais:" 	, 	"-k"	, 	cmdRelais ,	":RELAIS,0=OFF|1=ON|2=ByteMode,#CRC[OPTIONAL]"	},
	{"Help  :"	,	"-h"	,	help	  ,	NULL											},
};



void		timerInit( void )								
{	
	TCCR1B |= ((1<<WGM12) | (1<<CS10)); //CTC Mode 1

	TIMSK  |= (1<<OCIE1A); // streamInputCompareEnable
	
	OCR1A   = ( (uint16_t)( F_CPU / 1 / 8000 ) );	
}

uint16_t	checkMaxValue( uint16_t val , uint16_t max )	
{
	if( val > max )
	{
		return 0;
	}
	return val;
}
	
char		*readRingBuff( char *stream )					
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
	if ( c == '\r' || c == '\n' )
	{
		*( stream + index ) = '\0';
		index = 0; 
			
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

	uart_puts( "**RelaisSwitchBoard**\r\n");
	uart_puts( "Support by J.H - Elec.\r\n");
	uart_puts( "www.jh-elec.de\r\n" );
	uart_puts( "Ver.: " );
	uart_puts( buildVer() );
	uart_puts( "\r\n\n" );

	cmdInit( cmdTab , &raw , CMD_TAB_SIZE( cmdTab ) );

	char *streamPtr		= NULL;
	const char *cmdPtr	= NULL;
	
	while (1) 
    {	
		streamPtr = readRingBuff( streamIn );
		
		if ( streamPtr != NULL )
		{
			cmdPtr = cmdGetName( streamPtr );

			if ( cmdPtr != NULL  )
			{
				uart_puts( "****************************\r\n" );
		
				void *(*funcPtr)(void*,void*) = cmdGetFunc( streamIn );
				if ( funcPtr != NULL )
				{
					uart_puts( cmdPtr ); // Namen der Funktion ausgeben
					
					if ( funcPtr( streamIn , NULL ) != NULL )
					{
						uart_puts( "CMD_OK\r\n" );
					}
					else
					{
						uart_puts( "CMD_BAD\r\n" );
					}
					
				}
			
// 				char crc[5] = "";
// 				char *crcPtr = cmdGetCRC( crc , streamIn );
// 				if ( crcPtr != NULL )
// 				{
// 					uart_puts( "CRC.: " );
// 					uart_puts( crcPtr );
// 					uart_puts( "\r\n" );
// 				}
						
				uart_puts( "****************************\r\n" );				
			}
			else
			{
				uart_puts( "no command\r\n" );
				uart_puts( "Loopback: " );
				uart_puts( streamIn );
				uart_puts( "\r\n" );
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