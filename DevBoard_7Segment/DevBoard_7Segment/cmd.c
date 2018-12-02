/* Generated by CodeDescriptor 1.5.0.0907 */
/*
* Project Name      -> Kommando Interpreter
* Version           -> 1.0.1.1124
* Author            -> Hm @ Workstadion.: QP-01-02
* Build Date        -> 20.09.2017 07:50:01
* Description       -> Description
*
*
*
*/

#include <avr/io.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cmd.h"
#include "Hardware Libs/uart.h"

/*	Speicher f�r den Antwort Header
*	Die Nutzdaten werden werden dem Zeiger
*	der cmd_t Struktur �bergeben!
*/
static uint8_t	Frame[__CMD_HEADER_ENTRYS__];

static uint8_t MasterFrameCRC = 0;
static uint8_t SlaveFrameCRC = 0;


static inline uint8_t cmdCrc8CCITTUpdate ( uint8_t inCrc , uint8_t *inData )
{
	uint8_t   i = 0;
	static uint8_t data = 0;
	
	data = ( inCrc ^ ( *inData ) );
	
	for ( i = 0; i < 8; i++ )
	{
		if ( ( data & 0x80 ) != 0 )
		{
			data <<= 1;
			data ^= 0x07;
		}
		else
		{
			data <<= 1;
		}
	}

	return data;
}

int8_t		cmdSearchFrame( uint8_t *frame )
{
	for ( uint8_t x = 0 ; x < 127 ; x++ )
	{
		if ( frame[x] == '-' )
		{
			if ( x < 127 )
			{
				if ( frame[x+1] == '+' )
				{
					return x + 2;
				}
			}
			else
			{
				return -1; // �berlauf
			}
		}
	}
	
	return -1;
}

void		cmdInit				( cmd_t *c )					
{
	c->DataLength	= 0;	// L�nge der Nutzdaten Bytes
	c->DataType		= 0;	// Datentyp der Nutzdaten
	c->MessageID	= 0;	// Message Erkennung
	c->Exitcode		= 0;	// Exitkode aus Funktionen
	MasterFrameCRC	= 0;	// Checksumme der gesamten Message ( Vom PC )
	SlaveFrameCRC	= 0;	// Checksumme der gesamten Message ( Vom �C )
	c->DataPtr		= NULL; // Zeiger auf Nutzdaten
}

uint8_t		cmdParse			( uint8_t *rx , cmd_t *c )		
{
// 	int8_t FrameStart = cmdSearchFrame( rx );
// 	
// 	if ( FrameStart == - 1 )
// 	{
// 		return 1;
// 	}
		
	c->DataLength 	= rx[ CMD_START_FRAME_OFFSET + CMD_HEADER_LENGHT		] -__CMD_HEADER_ENTRYS__;
	c->DataType		= rx[ CMD_START_FRAME_OFFSET + CMD_HEADER_DATA_TYP		];
	c->MessageID 	= rx[ CMD_START_FRAME_OFFSET + CMD_HEADER_ID			];
	c->Exitcode		= rx[ CMD_START_FRAME_OFFSET + CMD_HEADER_Exitcode		];	
	MasterFrameCRC	= rx[ CMD_START_FRAME_OFFSET + CMD_HEADER_CRC			];
	
	if ( c->DataLength )
	{
		c->DataPtr = rx + ( CMD_START_FRAME_OFFSET + __CMD_HEADER_ENTRYS__ );
	}
	else
	{
		c->DataPtr = NULL; // Keine Nutzdaten
	}
	
	uint8_t crc = 0;
	SlaveFrameCRC = 0;
	
	rx[ CMD_START_FRAME_OFFSET + CMD_HEADER_CRC ] = 0;
	
	for ( uint8_t x = 0 ; x < __CMD_HEADER_ENTRYS__ ; x++ )
	{
		crc = cmdCrc8CCITTUpdate( crc , &rx[ CMD_START_FRAME_OFFSET + ( CMD_HEADER_LENGHT ) + x ] );
	}
	
	for ( uint8_t x = 0 ; x < c->DataLength ; x++ )
	{
		crc = cmdCrc8CCITTUpdate( crc , &rx[  ( CMD_START_FRAME_OFFSET + __CMD_HEADER_ENTRYS__ ) + x ] );
	}
	SlaveFrameCRC = crc;
	
	if ( SlaveFrameCRC != MasterFrameCRC )
	{
		return 2;
	}
	
	return 0;
}

uint8_t		cmdCrc8StrCCITT		( uint8_t *str , uint8_t leng )	
{
	uint8_t crc = 0;
		
	for( uint8_t x = 0 ; x < leng ; x++ )
	{
		crc = cmdCrc8CCITTUpdate( crc , ( uint8_t * ) str );
		str++;
	}
	
	return crc;
}

Header_t	cmdBuildHeader		( cmd_t *a )					
{		
	static Header_t HeaderInfo;
		
	Frame[CMD_HEADER_CRC]	= 0;
	
	uint8_t *tmpPtr	= a->DataPtr;

	uint16_t FrameSize = __CMD_HEADER_ENTRYS__ + a->DataLength;
	
	if ( FrameSize >= 255 )
	{
		HeaderInfo.Exitcode = 1;	
	}
	
	Frame[CMD_HEADER_LENGHT]		= (uint8_t)FrameSize; // L�nge der ganzen Antwort
	Frame[CMD_HEADER_DATA_TYP]		= a->DataType;		  // (u)char , (u)int8 , (u)int16 , (u)int32 usw.	
	Frame[CMD_HEADER_ID]			= a->MessageID;		  // 0..255
	Frame[CMD_HEADER_Exitcode]		= a->Exitcode;		  // 0..255
	
	/*	Checksumme vom Header bilden
	*/
	uint8_t FrameCRC = 0;
	for ( uint8_t x = 0 ; x < __CMD_HEADER_ENTRYS__ ; x++)
	{
		FrameCRC = cmdCrc8CCITTUpdate( FrameCRC , &Frame[x] );
	}
	
	/*	Checksumme von Nutzdaten bilden
	*/	
	if ( a->DataLength )
	{
		for ( uint8_t x = 0 ; x < a->DataLength ; x++ )
		{
			FrameCRC = cmdCrc8CCITTUpdate( FrameCRC , tmpPtr++ );	
		}			
	}
	else
	{
		HeaderInfo.Exitcode = 1; // Keine Nutzdaten vorhanden
		a->DataPtr = NULL;
	}

	Frame[CMD_HEADER_CRC] = FrameCRC;
		
	a->MessageID	= 0;
	a->DataType		= 0;
	a->Exitcode		= 0;
				
	HeaderInfo.FramePtr = Frame;
				
	return HeaderInfo;
}

void		cmdBuildAnswer		( cmd_t *a , uint8_t id , enum Data_Type_Enum DataType , uint8_t Exitcode , uint8_t DataLength , uint8_t *DataPtr )
{
	a->MessageID	= id;			// Beschreibt den Nachrichten Type. Damit die gegenstelle die Nachrichten unterscheiden kann
	a->DataType		= DataType;		// Gibt an um welchen Daten Typ es sich handelt
	a->Exitcode		= Exitcode;		// R�ckgabewert einer Funktion 
	a->DataPtr		= DataPtr;		// Zeiger auf die Daten die gesendet werden sollen
	a->DataLength	= DataLength;	// Anzahl der Bytes
}

void		cmdSendAnswer		( cmd_t *a )					
{
	Header_t HeaderInfo  = cmdBuildHeader( a );
	
	uartPutByteStr( HeaderInfo.FramePtr , __CMD_HEADER_ENTRYS__ );
	uartPutByteStr( a->DataPtr , a->DataLength );
}