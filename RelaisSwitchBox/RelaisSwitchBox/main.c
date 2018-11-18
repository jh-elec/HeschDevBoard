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

uint8_t *streamPtr		= NULL;
cmd_t cmd;
uint8_t streamIn[64] = "";



uint8_t		cmdRelais	( cmd_t *c )			
{ 
 	uint16_t		kx;
 	uint8_t			state;
	 	 
	kx		= c->dataPtr[0];
	state	= c->dataPtr[1];

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

	cmd.id = 0;
	cmd.exitcode = 0;
	cmd.dataPtr = buff;
	cmd.dataLen = sizeof(buff);
	cmdSendAnswer( &cmd );

	return 0;
}

uint8_t		cmdVersion ( cmd_t *c )
{
	cmdBuildAnswer( &cmd , 1 , DATA_TYP_STRING , 0 , 16 , (uint8_t*)buildVer() );
	cmdSendAnswer( &cmd );
	
	return 0;
}	

const cmdFuncTab_t cmdFuncTab[] =
{
	{ cmdRelais },
	{ cmdVersion },
};

void		timerInit		( void )							
{	
	TCCR1B |= ((1<<WGM12) | (1<<CS10)); //CTC Mode 1

	TIMSK  |= (1<<OCIE1A); // streamInputCompareEnable
	
	OCR1A   = ( (uint16_t)( F_CPU / 1 / 8000 ) );	
}

uint16_t	checkMaxValue	( uint16_t val , uint16_t max )		
{
	if( val > max )
	{
		return 0;
	}
	return val;
}
	

int main(void)
{
	hardware_init();
	
	uart_init( UART_BAUD_SELECT( 9600 , F_CPU ) );
	
	cmdInit( &cmd );
	
	timerInit();

	sei();

	while (1) 
    {			
		streamPtr	= uartReadRingBuff( streamIn );
		
		if ( streamPtr != NULL )
		{
 			if ( !cmdParse( streamPtr , &cmd ) )
 			{								 
				if( cmd.inCrc == cmd.outCrc && (cmd.id < sizeof(cmdFuncTab) / sizeof(*cmdFuncTab)) )
				{
					cmdFuncTab[cmd.id].fnc( &cmd );
				}else
				{
					cmdBuildAnswer( &cmd , 255 , DATA_TYP_STRING , 255 , 7 , (uint8_t*)"cmd_bad" );
					cmdSendAnswer( &cmd );
				}
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