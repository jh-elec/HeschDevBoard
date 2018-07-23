/* Generated by CodeDescriptor 1.2.0.0713 */
/*
* Project Name      -> RX8564
* Version           -> 1.0.0.0714
* Author            -> Hm @ Workstadion.: QP-01-02
* Build Date        -> 14.07.2017 07:18:15
* Description       -> Device Driver
*
*
*
*/

#include <avr/interrupt.h>
#include "rx8564.h"
#include "i2cmaster.h"
 
 uint8_t tmp[2] = "";

inline uint8_t bcdToDec(uint8_t val)
{
	return ((( val & 0xF0 ) >> 4 ) * 10 ) + ( val & 0x0F );
}

inline uint8_t decToBcd(uint8_t val)
{	
	return ((( val / 10 ) << 4) | ( val % 10 ));
}

void rtcInit(void)
{
	i2c_start_wait(RX8564+I2C_WRITE);
	i2c_write(control_1_register);
	i2c_write(0x00);
	i2c_stop();
}

void rtcSetTime(uint8_t hour, uint8_t minutes, uint8_t seconds)
{
    i2c_start_wait(RX8564+I2C_WRITE);
    i2c_write(seconds_register);
    i2c_write(decToBcd(seconds));
    i2c_write(decToBcd(minutes)); 
    i2c_write(decToBcd(hour));
    i2c_stop();
} 
   
void rtcSetDate(uint8_t day, uint8_t week_day, uint8_t month, uint16_t year)
{
    i2c_start_wait(RX8564+I2C_WRITE); 
    i2c_write(day_register);
	
    i2c_write(decToBcd(day));
    i2c_write(decToBcd(week_day)); 
    i2c_write(decToBcd(month));
    i2c_write(decToBcd( year - 2000 ));                
    i2c_stop();     
}
  
void rtcSetAlrt(uint8_t day, uint8_t week_day, uint8_t hour, uint8_t minutes)
{
    /*
	*	Umrechnung von Dezimal in BCD
	*/     
    i2c_start_wait(RX8564+I2C_WRITE);
    i2c_write(minute_alert_register);
    i2c_write(decToBcd(minutes)); 
    i2c_write(decToBcd(hour)); 
    i2c_write(decToBcd(day));
    i2c_write(decToBcd(week_day));
    i2c_stop();  
 }
  
void rtcSetClkOut(uint8_t frequency)
{    
    i2c_start_wait(RX8564+I2C_WRITE);
    i2c_write(clkout_frequency_register);
    i2c_write(frequency);
    i2c_stop();
}
 
void rtcGetData(rx8564_t *buffer)
{   
    i2c_start_wait(RX8564+I2C_WRITE);
    i2c_write(0x02);
	
	/*
	*	Slave bereit?
	*/
	if ( i2c_rep_start(RX8564+I2C_READ) )
	{
		i2c_stop();
		return;
	}
	
    buffer->second			= i2c_readAck();
    buffer->minute			= i2c_readAck();   
    buffer->hour			= i2c_readAck();
    buffer->day				= i2c_readAck();
    buffer->dayName			= i2c_readAck();
    buffer->month			= i2c_readAck();
    buffer->year			= i2c_readAck();
    buffer->alrt_minute		= i2c_readAck();
    buffer->alrt_hour		= i2c_readAck();
    buffer->alrt_day		= i2c_readAck();
    buffer->alrt_dayName	= i2c_readNak();
    i2c_stop();
 
    buffer->second			&= 0x7F;
    buffer->minute			&= 0x7F;
    buffer->hour			&= 0x3F;
          
    buffer->day				&= 0x3F;
    buffer->month			&= 0x1F;
    buffer->dayName			&= 0x07;
     
    buffer->alrt_minute		&= 0x7F;
    buffer->alrt_hour		&= 0x7F;
    buffer->alrt_day		&= 0x7F;
    buffer->alrt_dayName	&= 0x7F;
}
  
void rtcSetCtrl2(uint8_t mask)
{
    i2c_start_wait(RX8564+I2C_WRITE);
    i2c_write(0x01);
    i2c_write(mask);
    i2c_stop(); 
}
 
void rtcSetTimer_control(uint8_t mask)
{
    i2c_start_wait(RX8564+I2C_WRITE);
    i2c_write(0x0E);
    i2c_write(mask);
    i2c_stop(); 
}

uint8_t rtcReadTim(void)
{	
	i2c_start_wait(RX8564+I2C_WRITE);
	i2c_write(0x0F);
	i2c_stop();
	
	i2c_rep_start(RX8564+I2C_READ);
	tmp[0] = i2c_readNak();
	i2c_stop();
	
	return tmp[0];
}