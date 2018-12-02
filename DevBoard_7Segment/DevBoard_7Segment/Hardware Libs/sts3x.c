/* Generated by CodeDescriptor 1.3.0.0714 */
/*
* Project Name      -> STS3x
* Version           -> 1.0.0.0810
* Author            -> Hm @ Workstadion.: QP-01-02
* Build Date        -> 10.08.2017 06:51:39
* Description       -> Description
*
*
*
*/

#include <avr/io.h>
#include <avr/crc16.h>

#include "sts3x.h"
#include "i2cmaster.h"



sts3x_t sts3x =
{
	.lowest  =	127,
	.actual  =	25,
	.highest = -127,
};

static inline uint8_t cmdCrc8CCITTUpdate ( uint8_t inCrc , uint8_t *inData )
{
	uint8_t   i = 0;
	static uint8_t data = 0;
	
	data = ( inCrc ^ ( *inData ) );
	
	for ( i = 0; i < 8; i++ )
	{
		if ( ( data & 0x80 ) != 0 )
		{
			data <<= 1;
			data ^= 0x31;
		}
		else
		{
			data <<= 1;
		}
	}

	return data;
}


void		sts3xInit		(void)
{
	sts3xReset();
	
	i2c_start_wait( STS3x_ADDR + I2C_WRITE );
	
	i2c_write( periodic_measure_commands_hpm[4][0] );
	i2c_write( periodic_measure_commands_hpm[4][1] );

 	i2c_write( STS3x_FETCH_DATA_MSB );
 	i2c_write( STS3x_FETCH_DATA_LSB );	
	 
	i2c_stop();
}

int16_t		sts3xCalc		(uint16_t temp)
{	
	double stemp=temp;
	stemp *= 175;
	stemp /= 0xffff;
	stemp = -45 + stemp;
	
	return (int16_t)stemp;
}

uint16_t	sts3xRead		(void)
{
	static uint8_t read[STS3X_NUM_OF_BYTES] = "";
	
	/*
	*	Alle Status Bits reseten..
	*/
	sts3x.state = 0;
	
	i2c_start_wait( STS3x_ADDR + I2C_READ );
	read[STS3X_MSB] = i2c_readAck();
	read[STS3X_LSB] = i2c_readAck();
	read[STS3X_CRC] = i2c_readNak();

	i2c_stop();

	uint8_t crc = 0xff;
	crc = cmdCrc8CCITTUpdate(crc,&read[STS3X_MSB]);
	crc = cmdCrc8CCITTUpdate(crc,&read[STS3X_LSB]);

	/* 
	* Generierten CRC mit empfangenen �berpr�fen 
	*/
	if ( crc == read[STS3X_CRC] )
	{
		return (uint16_t)read[STS3X_MSB] << 8 | read[STS3X_LSB];
	}
	else
	{
		/*
		*	CRC Error
		*/
		sts3x.state |= STS3X_CRC_ERR;		
	}
	
	/*
	*	Schl�gt eine Messung fehl oder CRC passt nicht
	*	wird der Wert f�r "0 �C" zur�ck gegeben
	*/
	return 1700;
}

int8_t		sts3xGetTemp	(void)
{
	/*
	*	Sollte ein Checksummenfehler auftreten,
	*	so wird der zuletzt gemessene Wert zur�ck gegeben
	*/
	
	sts3x.actual = sts3xCalc( sts3xRead() );
	
	return (sts3x.actual);
}

void		sts3xReset		(void)
{
	i2c_start_wait( STS3x_ADDR + I2C_WRITE );
	i2c_write( 0xA2 );
	i2c_write( 0x30 );
	i2c_stop();
}