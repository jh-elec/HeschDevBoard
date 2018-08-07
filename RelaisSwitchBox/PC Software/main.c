#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define CMD_SIZE_OF_TAB			7
#define CMD_RAW_DATA_BEGINN		":"
#define CMD_RAW_PARA_DELIMITER	","
#define CMD_DATA_END			';'

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
	{"Relais Handler:" 			, 	"-REL"			, 	NULL	},
};

cmd_t cmd =
{
	.table	= cmdTab,
	.tabLen = sizeof( cmdTab ) / sizeof( *cmdTab ),
	.raw 	= &raw
};

static uint8_t 		cmdCntPara			( char *cmd )								
{
	uint8_t x = 0;
	char *cmdPtr = cmd;
	char *beginnPtr = cmd;
	
	beginnPtr = strchr( beginnPtr , ':' ); // Erster Parameter
	if ( beginnPtr != NULL )
	{
		if ( *(beginnPtr + 1 ) != ';' )
		{
			x++;	
		}
	}
	
	for ( ; cmdPtr != NULL ;  )
	{
		cmdPtr = strchr( cmdPtr + 1 , ',' ); // weitere Parameter
		x++;
	}
	
	return ( x - 1 );
}

char			*cmdSearch			( char *inBuff , char *srchCmd )			
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
	cmdEndPtr	 = strchr( inBuffPtr , CMD_DATA_END );	      

	if ( cmdEndPtr == NULL || cmdBeginnPtr == NULL )
	{
		return NULL;
	}
	
	cmd_ = cmdBeginnPtr;
		
	return cmdBeginnPtr;
}

int8_t		cmdGetIndex			( cmd_t *cmd , char *inBuff )				
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


char 				*cmdGetPara 		( cmd_t *cmd , char *input , uint8_t num )	
{
	char 		*delimiter 	= NULL;
	char 		*cmdEndPtr	= NULL;
	const char 	*rawPtr		= NULL;

	uint8_t x;
			
	for ( x = 0 ; x < cmd->tabLen ; x++ )
	{
		rawPtr = cmdSearch( input , ( char * ) cmd->table[x].instruction );
		if ( rawPtr != NULL )
		{
			break;
		}	
	}
			
	if ( rawPtr == NULL )
	{
		return NULL;
	}
	
	cmd->raw->paraNumb = cmdCntPara( ( char * ) rawPtr );
		
	cmdEndPtr = strchr( rawPtr , ';' );	
	if ( cmdEndPtr == NULL )
	{
		return  NULL;
	}

	delimiter = strchr( rawPtr , ':' ) + 1;
	if ( ( delimiter - 1 ) == NULL )
	{
		return  NULL;
	}

	uint8_t pos;
	pos = strcspn( delimiter , "," );
	
	
	return delimiter; 
}

int main(int argc, char *argv[]) 
{
	char stream[] = "-RELais:0,1,2,3,4;";
	
	printf( "%s\r\n" , cmdGetPara( &cmd , stream , 0 ) );

	return 0;
}
