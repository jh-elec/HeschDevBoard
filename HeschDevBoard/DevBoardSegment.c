


typedef struct
{
	unsigned char	dig[4];
	uint8_t			dp;
}seg_t;
seg_t disp;

void segmentShowTime(uint8_t hh, uint8_t mm)
{
	disp.dig[0] = (hh&0xF0)>>4;
	disp.dig[1] = (hh&0x0F);
	disp.dig[2] = (mm&0xF0)>>4;
	disp.dig[3] = (mm&0x0F);
}

void segmentShowTemp(int16_t tempIn, uint8_t sense)
{
	
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

void segmentShowNumber(const unsigned char numb,enum mcp23017_gpb dig)
{
	if (numb>sizeof(ascii)){
		return;
	}
	mcp23017_set_gpa(~((ascii[numb])));
	mcp23017_set_gpb(~dig & 0x7F); // mask out REFERENCE_ENABLE
}

void muxSegment(void)
{
	static uint8_t mux=0;
	switch(mux)
	{
		case 0:
		{
			segmentShowNumber(disp.dig[0],DRIVE_dig_1);
		}break;
		case 1:
		{
			segmentShowNumber(11,~(DRIVE_dig_1 | DRIVE_dig_2 | DRIVE_dig_3 | DRIVE_dig_4));
		}break;
		case 2:
		{
			segmentShowNumber(disp.dig[1],DRIVE_dig_2);
		}break;
		case 3:
		{
			segmentShowNumber(11,~(DRIVE_dig_1 | DRIVE_dig_2 | DRIVE_dig_3 | DRIVE_dig_4));
		}break;
		case 4:
		{
			segmentShowNumber(disp.dig[2],DRIVE_dig_3);
		}break;
		case 5:
		{
			segmentShowNumber(11,~(DRIVE_dig_1 | DRIVE_dig_2 | DRIVE_dig_3 | DRIVE_dig_4));
		}break;
		case 6:
		{
			segmentShowNumber(disp.dig[3],DRIVE_dig_4);
		}break;
		case 7:
		{
			segmentShowNumber(11,~(DRIVE_dig_1 | DRIVE_dig_2 | DRIVE_dig_3 | DRIVE_dig_4));
		}break;
		case 8:
		{
			rx8564_get_data(&rtc);
		}break;
		case 9:
		{
			static uint16_t readOut_delay=0;
			if (readOut_delay++<100)
			{
				
			}else
			{
				readOut_delay=0;
				
				temp.sts3x = sts3x_calc(sts3x_read());
				if (temp.sts3x > temp.max_old_sts3x)
				{
					temp.max_old_sts3x = temp.sts3x;
				}
				if (temp.sts3x < temp.min_old_sts3x)
				{
					temp.min_old_sts3x = temp.sts3x;
				}
			}
		}break;
		case 10:
		{
			temp.tmp102 = tmp102_calc(tmp102_read());
			if (temp.tmp102 > temp.max_old_tmp102 )
			{
				temp.max_old_tmp102  = temp.tmp102 ;
			}
			if (temp.tmp102  < temp.min_old_tmp102 )
			{
				temp.min_old_tmp102  = temp.tmp102 ;
			}
		}break;
		default: mux = 0;
	}
	mux++;
}