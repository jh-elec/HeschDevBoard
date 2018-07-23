
#include <avr/io.h>
#include <stdlib.h>
#include <string.h>

#include "DevBoardSegment.h"
#include "Hardware Libs/hard_def.h"
#include "Hardware Libs/mcp23017.h"

seg_t disp =
{
	.dig = { ' ' , ' ' , ' ' , ' '},
};

static char str[11] = "";

void selectFont( char **fnt )
{
	disp.fnt = (char*)fnt;
}

uint8_t getCharMapIndex( char ch )
{
	uint8_t i = 0;	
	while( ch != *(disp.fnt + ( ( i * 2 ) ) ) && *(disp.fnt + ( ( i * 2 ) ) ) != '\0' )
	{
		i++;
	}
	return i;
}

char *timeBcdToStr( uint8_t hh , uint8_t mm , uint8_t ss )
{
	if ( hh == DISP_SPACE_CHAR )
	{
		str[0] = ' ';
		str[1] = ' ';
	}
	else
	{
		str[0] = ( ( hh & 0xF0 ) >> 4 ) + '0';
		str[1] = ( hh & 0x0F ) + '0';
	}
	
	str[2] = '.';
	str[3] = ' ';
	
	if ( mm == DISP_SPACE_CHAR )
	{
		str[4] = ' ';
		str[5] = ' ';		
	}
	else
	{
		str[4] = ( ( mm & 0xF0 ) >> 4 ) + '0';
		str[5] = ( mm & 0x0F ) + '0';		
	}
	
	str[6] = '.';
	str[7] = ' ';
	
	if ( ss == DISP_SPACE_CHAR )
	{
		str[8] = ' ';
		str[9] = ' ';
	}
	else
	{
		str[8] = ( ( ss & 0xF0 ) >> 4 ) + '0';
		str[9] = ( ss & 0x0F ) + '0';
	}	
	
	str[10] = '\0';
	
	return str;
}

void timeDecToRam( uint8_t hh , uint8_t mm )
{
	if ( hh == DISP_SPACE_CHAR )
	{
		disp.dig[0] = ' ';
		disp.dig[1] = ' ';
	}
	else
	{
		disp.dig[0] = ( hh / 10 ) + '0';
		disp.dig[1] = ( hh % 10 ) + '0';
	}

	if ( mm == DISP_SPACE_CHAR )
	{
		disp.dig[2] = ' ';
		disp.dig[3] = ' ';
	}
	else
	{
		disp.dig[2] = ( mm / 10 ) + '0';
		disp.dig[3] = ( mm % 10 ) + '0';
	}
}

char *tempToStr( int16_t tempIn , uint8_t sense )
{
	int16_t result		=0;
	uint8_t sign		=0;
		
	result = tempIn;
	if (result<0)
	{
		result = result*-1;
		sign = 0x5A;
	}
	
	str[0] = ' ';
	
	switch( sense )
	{
		case 0:	
		{
			str[0] = '`';
		}break;
		
		case 1:	
		{
			if ( sign == 0x5A )
			{
				str[0]	= 16;
			}
			else
			{
				str[0]	= '´';
			}
		}break;
		
		default:   
		{
			if ( sign == 0x5A && sense == 0xFF )
			{
				str[0] = '-';
			}
			else
			{
				str[0] = ' ';
			}
			
		}break;
	}

	str[1] = (result/10)  + '0';
	str[2] = (result%10)  + '0';
	str[3] = '°';
	str[4] = 'C';
	str[5] = '\0';
	
	return str;
}

void chToRam( char ch , enum mcp23017_gpb dig )
{	
	char ret = ~( * ( ( disp.fnt ) + ( ( 2 * getCharMapIndex( ch ) ) + 1 ) ) );
	
	mcp23017_set_gpa( ret );
	mcp23017_set_gpb( (~(dig)) & ~(REFERENCE_ENABLE) ); // REFERENCE_ENABLE nicht mit setzen
}

void refEnable( uint8_t state )
{
	switch( state )
	{
		case 0:
		{
			mcp23017_set_gpb( 0xFF & ~REFERENCE_ENABLE );
		}break;
		
		case 1:
		{
			mcp23017_set_gpb( REFERENCE_ENABLE );
		}break;
	}
}

void clearDigits( void )
{
	
	for ( uint8_t x = 0 ; x < 4 ; x++ )
	{
		disp.dig[x] = ' ';
	}
}

void muxDigits( void )
{
	static uint8_t mux = 0;
	
	if ( mux >= 35 )
	{
		mux = 0;
	}
	else
	{
		mux++;
	}
	
	switch( mux )
	{
		case 0:
		{
			chToRam( disp.dig[0] , DRIVE_dig_1 );
		}break;
		case 5:
		{
			chToRam( ' ' , ~(DRIVE_dig_1 | DRIVE_dig_2 | DRIVE_dig_3 | DRIVE_dig_4 ) );
		}break;
		case 10:
		{
			chToRam( disp.dig[1] , DRIVE_dig_2 );
		}break;
		case 15:
		{
			chToRam( ' ', ~( DRIVE_dig_1 | DRIVE_dig_2 | DRIVE_dig_3 | DRIVE_dig_4 ) );
		}break;
		case 20:
		{
			chToRam( disp.dig[2] , DRIVE_dig_3 );
		}break;
		case 25:
		{
			chToRam( ' ' , ~( DRIVE_dig_1 | DRIVE_dig_2 | DRIVE_dig_3 | DRIVE_dig_4 ) );
		}break;
		case 30:
		{
			chToRam(disp.dig[3],DRIVE_dig_4);
		}break;
		case 35:
		{
			chToRam( ' ' , ~( DRIVE_dig_1 | DRIVE_dig_2 | DRIVE_dig_3 | DRIVE_dig_4 ) );
		}break;
	}
}