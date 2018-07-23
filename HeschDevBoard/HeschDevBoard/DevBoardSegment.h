

#ifndef __DEVBOARDSEGMENT_H__
#define __DEVBOARDSEGMENT_H__

typedef struct
{
	uint8_t Rx[3];

	int16_t tmp102;
	int16_t sts3x;
	
	uint8_t	avg_cnt[2];
	uint8_t avg_state;
	
	int16_t max_old_sts3x;
	int16_t max_old_tmp102;
	
	int16_t min_old_sts3x;
	int16_t min_old_tmp102;
	
	int8_t alrt;

}temp_t;

typedef struct
{
	unsigned char	dig[4];
	uint8_t			dp;
}seg_t;


void segmentBuildTime(seg_t *disp, uint8_t hh, uint8_t mm);

void segmentBuildTemperature(seg_t *disp, int16_t tempIn, uint8_t sense);

void segmentShowNumber(const unsigned char numb,enum mcp23017_gpb dig);

void muxSegment(seg_t *s);

#endif