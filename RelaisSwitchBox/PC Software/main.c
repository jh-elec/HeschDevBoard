#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define CMD_RAW_DATA_BEGINN		":"
#define CMD_RAW_PARA_DELIMITER	","
#define CMD_DATA_END			";"
#define CMD_CRC_BEGINN			'#'

char	*cmd_ = NULL;

typedef struct						
{
	/*
	*	Name des Kommandos
	*/
	char	name[20];
	
	/*
	*	Befehl der empfangen werden muss
	*/
	char	instruction[15];
			
	/*
	*	Funktion die beim entsprechenden Kommando 
	*	ausgeführt werden soll
	*/	
	void*	(*fnc) (void* , void*);
	
}cmdTable_t;

typedef struct						
{
	uint8_t	 para[10][10];
	uint8_t  paraNumb;	
}cmdRaw_t;
cmdRaw_t raw;

typedef struct						
{
	const 			cmdTable_t 	*table;
	const			size_t		tabLen;
					cmdRaw_t	*raw;
}cmd_t;

const cmdTable_t cmdTab[] =
{
	{"Relais Handler:" 			, 	"-RELais"		, 	NULL	},
};

cmd_t cmd =
{
	.table	= cmdTab,
	.tabLen = sizeof( cmdTab ) / sizeof( *cmdTab ),
	.raw 	= &raw
};

static uint8_t 		cmdCntPara			( cmd_t *cmd , char *stream )								
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

char				*cmdSearch			( char *inBuff , char *srchCmd )			
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

int8_t				cmdGetIndex			( cmd_t *cmd , char *inBuff )				
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
	
	char *streamEndPtr = strchr( streamPtr , CMD_RAW_PARA_DELIMITER[0] );
	
	char *outPtr = out;
	while( *streamPtr != '\0' && *streamPtr != CMD_RAW_PARA_DELIMITER[0] && *streamPtr != CMD_DATA_END[0] )
	{
		if ( *( streamPtr )  == CMD_CRC_BEGINN )
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
	
	crcPtr = strchr( crcPtr , CMD_CRC_BEGINN );
	
	if ( crcPtr == NULL || *( crcPtr - 1 ) != CMD_RAW_PARA_DELIMITER[0] )
	{
		return NULL;
	}
	
	crcPtr += 1;
		
	while( *crcPtr != '\0' && *crcPtr != CMD_DATA_END[0] )
	{
		*outPtr++ = *crcPtr++;
	}	
	
	return out;	
}


char stream[] = "-RELais:123,345,312,431,#543;";

int main(int argc, char *argv[]) 
{
	
	char buff[10]="";
	
	printf( "Parameter 1.: %s\r\n" , cmdGetPara( &cmd , buff , stream , 0 ) );
	printf( "Parameter 2.: %s\r\n" , cmdGetPara( &cmd , buff , stream , 1 ) );
	printf( "Parameter 3.: %s\r\n" , cmdGetPara( &cmd , buff , stream , 2 ) );
	printf( "Parameter 4.: %s\r\n" , cmdGetPara( &cmd , buff , stream , 3 ) );
	printf( "Buff.: %s\r\n" , cmdGetCRC( buff , stream ) );

	return 0;
}
