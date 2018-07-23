
#include "Hardware Libs/hard_def.h"

#ifndef __DEVBOARDSEGMENT_H__
#define __DEVBOARDSEGMENT_H__


#define DISP_SPACE_CHAR		( 123 )
#define DIGITS				4

typedef struct
{
	unsigned char			dig[DIGITS];
	uint8_t					dp;
	char					*fnt;
}seg_t;
seg_t disp;

void selectFont( char **fnt );

char *timeBcdToStr( uint8_t hh , uint8_t mm , uint8_t ss);

void timeDecToRam( uint8_t hh , uint8_t mm );

char *tempToStr( int16_t tempIn , uint8_t sense );

void chToRam( char ch , enum mcp23017_gpb dig );

void refEnable( uint8_t state );

void clearDigits( void );

void muxDigits( void );

#endif