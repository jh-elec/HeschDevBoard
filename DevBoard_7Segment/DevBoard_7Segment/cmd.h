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

#ifndef __CMD_H__
#define __CMD_H__

#include <stdint.h>
#include <stdlib.h>


/*	Frame Offset
*	Specifies where the command package begins
*/
#define CMD_START_FRAME_OFFSET	0


enum Communication_Header_Enum
{
	CMD_HEADER_LENGHT, 		// L�nge des ganzen Streams
	CMD_HEADER_DATA_TYP, 	// (u)char , (u)int8 , (u)int16 , (u)int32
	CMD_HEADER_ID, 			// Stream ID
	CMD_HEADER_Exitcode,	// Exitkode aus Funktionen
	CMD_HEADER_CRC, 		// Checksumme von der Message

	__CMD_HEADER_ENTRYS__
};

enum Data_Type_Enum
{
    DATA_TYP_UINT8,
    DATA_TYP_UINT16,
    DATA_TYP_UINT32,
    DATA_TYP_FLOAT,
    DATA_TYP_STRING,

    DATA_TYP_INT8,
    DATA_TYP_INT16,
    DATA_TYP_INT32,

    __DATA_TYP_MAX_INDEX__
};

enum Cmd_Id_Enum
{
	ID_PING = 0, // Darauf sollte die Firmware ein Lebenszeichen zur�ckliefern
	ID_VERSION,
	/*...*/
	
	ID_APPLICATION = 255 // F�r irgendwelche System spezifschen Meldungen
};



typedef struct
{
	uint8_t DataLength;
	uint8_t DataType;
	uint8_t MessageID;
	uint8_t Exitcode;	

	uint8_t *DataPtr;

}cmd_t;

/*	Hinweis
*	Das erste Element ist IMMER eine "Ping" Funktion.
*	Die einfach den String "ping" zur�ckliefert.
*	Dies sollte in jedem Projekt beachtet werden.
*/
typedef struct 
{
	uint8_t (*fnc)( cmd_t *);	
}cmdFuncTab_t;


typedef struct  
{
	uint8_t	*FramePtr;
	uint8_t	Exitcode;
}Header_t;
	

void		cmdInit				( cmd_t *c );					

int8_t		cmdGetStartIndex	( uint8_t *rx );					

uint8_t		cmdGetEndIndex		( uint8_t *rx );					

uint8_t		cmdParse			( uint8_t *rx , cmd_t *c );		

uint8_t		cmdCrc8StrCCITT		( uint8_t *str , uint8_t leng );

Header_t	cmdBuildHeader		( cmd_t *a );					

void		cmdBuildAnswer		( cmd_t *a , uint8_t id , enum Data_Type_Enum DataTypee , uint8_t Exitcode , uint8_t DataLength , uint8_t *DataPtr );

void		cmdSendAnswer		( cmd_t *a );					



#endif
