
#include <stdlib.h>
#include <stdint.h>

#include "mcp23017.h"
#include "DevBoardSegment.h"

#ifndef __HARD_DEF_H__
	#include "hard_def.h"
#endif

#ifndef __LED_FONT_H__
	#include "led_font.h"
#endif


void segmentBuildTime(seg_t *disp, uint8_t hh, uint8_t mm)
{
	disp->dig[0] = (hh&0xF0)>>4;
	disp->dig[1] = (hh&0x0F);
	disp->dig[2] = (mm&0xF0)>>4;
	disp->dig[3] = (mm&0x0F);
}

void segmentBuildTemperature(seg_t *disp, int16_t tempIn, uint8_t sense)
{
	int16_t result=0;
	uint8_t sign=0;
	
	result = tempIn;
	if (result<0)
	{
		result = result*-1;
		sign = 0x5A;
	}
	
	switch(sense){
		case 0:	
		{
			disp->dig[0] = 13;
		}break;
		case 1:	
		{
			if (sign==0x5A)
			{
				disp->dig[0]=16;
			}
			else
			{
				disp->dig[0]=14;
			}
		}break;
		
		default	:   
		{
			if (sign==0x5A&&sense==0xFF)
			disp->dig[0] = 15;
			else
			disp->dig[0] = 11;	
		}break;
	}

	disp->dig[1] = (result/10);
	disp->dig[2] = (result%10);
	disp->dig[3] = 12;
}




void segmentShowNumber(const unsigned char numb,enum mcp23017_gpb dig)
{
	if (numb>sizeof(ascii))
	return;
	
	mcp23017_set_gpa(~((ascii[numb])));
	mcp23017_set_gpb(~dig & 0x7F); // mask out REFERENCE_ENABLE
}

/*
*	Sollte in einem Timer aufgerufen werden. 
*	Auf die Frequenz ist unbedingt zu achten!
*/
void muxSegment(seg_t *s)
{
	static uint8_t mux=0;
	switch(mux)
	{
		case 0:
		{
			segmentShowNumber(s->dig[0],DRIVE_dig_1);
		}break;
		
		case 1:
		{
			segmentShowNumber(11,~(DRIVE_dig_1 | DRIVE_dig_2 | DRIVE_dig_3 | DRIVE_dig_4));
		}break;
		
		case 2:
		{
			segmentShowNumber(s->dig[1],DRIVE_dig_2);
		}break;
		
		case 3:
		{
			segmentShowNumber(11,~(DRIVE_dig_1 | DRIVE_dig_2 | DRIVE_dig_3 | DRIVE_dig_4));
		}break;
		
		case 4:
		{
			segmentShowNumber(s->dig[2],DRIVE_dig_3);
		}break;
		
		case 5:
		{
			segmentShowNumber(11,~(DRIVE_dig_1 | DRIVE_dig_2 | DRIVE_dig_3 | DRIVE_dig_4));
		}break;
		
		case 6:
		{
			segmentShowNumber(s->dig[3],DRIVE_dig_4);
		}break;
		
		case 7:
		{
			segmentShowNumber(11,~(DRIVE_dig_1 | DRIVE_dig_2 | DRIVE_dig_3 | DRIVE_dig_4));
		}break;
		
		default: mux = 0;
	}
	mux++;
}