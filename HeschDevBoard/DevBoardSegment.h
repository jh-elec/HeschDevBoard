

#ifndef __DEVBOARDSEGMENT_H__
#define __DEVBOARDSEGMENT_H__

void segmentShowTime(uint8_t hh, uint8_t mm);

	disp.dig[0] = (hh&0xF0)>>4;
	disp.dig[1] = (hh&0x0F);
	disp.dig[2] = (mm&0xF0)>>4;
	disp.dig[3] = (mm&0x0F);
}
void segmentShowTemp(int16_t tempIn, uint8_t sense);

	
	int16_t result=0;
	uint8_t sign=0;
	
	result = tempIn;
	if (result<0){
		result = result*-1;
		sign = 0x5A;
	}
	
	switch(sense){
		case 0		:	{disp.dig[0] = 13;}break;
		case 1		:	{
			if (sign==0x5A){
				disp.dig[0]=16;
				}else{
				disp.dig[0]=14;
			}
		}break;
		
		default		:   {
			if (sign==0x5A&&sense==0xFF){
				disp.dig[0] = 15;
				}else{
				disp.dig[0] = 11;
			}
			
		}break;
	}

	disp.dig[1] = (result/10);
	disp.dig[2] = (result%10);
	disp.dig[3] = 12;
}
void segmentShowNumber(const unsigned char numb,enum mcp23017_gpb dig);

void muxSegment(void);

#endif