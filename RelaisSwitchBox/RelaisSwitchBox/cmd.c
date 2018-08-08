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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cmd.h"

char	*cmd_ = NULL;

// const cmdTable_t cmdTab[] =
// {
// 	{"Relais_Handler!" 			, 	"-REL"		, 	NULL 	},
// 
// };
// 
// cmd_t cmd =
// {
// 	.table	= cmdTab,
// 	.tabLen = sizeof( cmdTab ) / sizeof( *cmdTab ),
// 	.raw 	= &raw
// };



static char			*cmdSearch			( char *inBuff , char *srchCmd )					
{
	/*
	*	Zeiger deklarieren).
	*/
	char *inBuffPtr		= inBuff;
	char *srchCmdPtr	= srchCmd;
	char *cmdBeginnPtr 	= NULL;
	char *cmdEndPtr		= NULL;
	
	if ( inBuffPtr == NULL || srchCmdPtr == NULL )
	{
		return NULL;
	}

	cmdBeginnPtr = strstr( inBuffPtr , srchCmdPtr );
	cmdEndPtr	 = strchr( inBuffPtr , CMD_DATA_END[0] );	      

	if ( cmdEndPtr == NULL || cmdBeginnPtr == NULL )
	{
		return NULL;
	}
	
	cmd_ = cmdBeginnPtr;
		
	return cmdBeginnPtr;
}

static int8_t		cmdGetIndex			( cmd_t *cmd , char *inBuff )						
{
	uint8_t i;
	
	cmd_t 	*cmdPtr			= cmd;
	char  	*cmdSearchPtr 	= NULL;
	char	*inputPtr		= inBuff;

	for ( i = 0 ; i < cmdPtr->tabLen ; i++ )
	{
		cmdSearchPtr = cmdSearch( inputPtr , ( char* ) cmdPtr->table[i].instruction );
		
		if ( cmdSearchPtr != NULL )
		{
			return i;
		}
	}
	
	return -1;	
}


uint8_t 			cmdCntPara			( cmd_t *cmd , char *stream )						
{
	uint8_t x = 0;
	char *cmdPtr = stream;
	char *beginnPtr = stream;
	
	beginnPtr = strchr( beginnPtr , CMD_RAW_DATA_BEGINN[0] ); // Erster Parameter
	if ( beginnPtr != NULL )
	{
		if ( *(beginnPtr + 1 ) != CMD_DATA_END[0] )
		{
			x++;	
		}
	}
	
	for ( ; cmdPtr != NULL ;  )
	{
		cmdPtr = strchr( cmdPtr + 1 , CMD_RAW_PARA_DELIMITER[0] ); // weitere Parameter
		x++;
	}
	
	cmd->raw->paraNumb = ( x - 1 );
	
	return ( x - 1 );
}

const char			*cmdGetInstruction	( cmd_t *cmd , char *input )						
{
	int8_t i = cmdGetIndex( cmd , input );
	
	if ( i != (int8_t) -1 )
	{
		return cmd->table[i].instruction;
	}
	
	return NULL;
}

const char			*cmdGetName			( cmd_t *cmd , char *input )						
{
	int8_t ret = cmdGetIndex( cmd , input );
	
	if ( ret != (int8_t) -1 )
	{
		return cmd->table[ret].name;
	}
		
	return NULL;
}

void				*cmdGetFunc			( cmd_t *cmd , char *input )						
{
	int8_t ret = cmdGetIndex( cmd , input );
	if ( ret != (int8_t) -1 )
	{
		return cmd->table[ret].fnc;
	}
	return NULL;
}

char 				*cmdGetPara 		( cmd_t *cmd , char *out , char *in , uint8_t num )	
{
	const char 	*rawPtr		= NULL;

	uint8_t x;
	
	for ( x = 0 ; x < cmd->tabLen ; x++ )
	{
		rawPtr = cmdSearch( in , ( char * ) cmd->table[x].instruction );
		if ( rawPtr != NULL )
		{
			break;
		}
	}
	
	if ( rawPtr == NULL )
	{
		return NULL;
	}
		
	char *streamPtr = in;
	
	streamPtr = strchr( in , CMD_RAW_DATA_BEGINN[0] ) + 1;
	
	uint8_t i;
	for ( i = 0 ; i < num ; i++ )
	{
		streamPtr = strchr( streamPtr , CMD_RAW_PARA_DELIMITER[0] ) + 1;
	}
		
	char *outPtr = out;
	while( *streamPtr != '\0' && *streamPtr != CMD_RAW_PARA_DELIMITER[0] && *streamPtr != CMD_DATA_END[0] )
	{
		if ( *( streamPtr )  == CMD_CRC_BEGINN[0] )
		{
			return NULL;
		}
		*outPtr++ = *streamPtr++;
	}
	
	return out;
}

char 				*cmdGetCRC 			( char *out , char *stream )						
{
	char *crcPtr = stream;
	char *outPtr = out;
	
	crcPtr = strchr( crcPtr , CMD_CRC_BEGINN[0] ) + 1;
	
	if ( ( crcPtr - 1 ) == NULL )
	{
		return NULL;
	}
	
	while( *crcPtr != '\0' && *crcPtr != CMD_DATA_END[0] )
	{
		*outPtr++ = *crcPtr++;
	}	
	
	return out;	
}



