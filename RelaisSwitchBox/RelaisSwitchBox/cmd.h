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


#define CMD_RAW_DATA_BEGINN		":"
#define CMD_RAW_PARA_DELIMITER	","
#define CMD_DATA_END			'\0'
#define CMD_CRC_BEGINN			"#"


typedef struct						
{
	/*
	*	Name des Kommandos
	*/
	char		name[20];
	
	/*
	*	Befehl der empfangen werden muss
	*/
	char		instruction[15];
			
	/*
	*	Funktion die beim entsprechenden Kommando 
	*	ausgeführt werden soll
	*/	
	void*		(*fnc) (void* , void*);
	
	/*
	*	Befehlssyntax
	*/
	const char	*syntax;
	
}cmdTable_t;

typedef struct						
{
	char 	*cmdPtr;
	uint8_t  paraNumb;	
}cmdRaw_t;
cmdRaw_t raw;

typedef struct						
{
	const 			cmdTable_t 	*table;
	const			size_t		tabLen;
					cmdRaw_t	*raw;
}cmd_t;


uint8_t 			cmdCntPara			( cmd_t *cmd , char *stream );

const char			*cmdGetInstruction	( cmd_t *cmd , char *input );

const char			*cmdGetName			( cmd_t *cmd , char *input );

void				*cmdGetFunc			( cmd_t *cmd , char *input );

char 				*cmdGetPara 		( cmd_t *cmd , char *out , char *in , uint8_t num );

char 				*cmdGetCRC 			( char *out , char *stream );

char				*cmdHelp			( cmd_t *cmd , char *helpBuff );

#endif
