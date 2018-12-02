/* Generated by Header_Generator V1.0 */

/*
* Project Name -> Portexpander - MCP23017
* Version      -> 1.0
* Author       -> Hm
* Build Date   -> 24.04.2017
* Description  -> Driver
*
*
*
*/

#include <avr/io.h>
#include "i2cmaster.h"
#include "mcp23017.h"
#include "hard_def.h"

static uint8_t cmd = 0;

void mcp23017_init(void)
{
	i2c_start_wait(MCP23017_ADDR + I2C_WRITE);
	
	cmd = GPIOS_COMP_AS_OUT; 
	/* set gpios @ outputs */
	i2c_write(IODIRA);
	i2c_write(cmd);
	i2c_write(IODIRB);
	i2c_write(cmd);
	i2c_stop();
	
	mcp23017_set_gpa(0);
	mcp23017_set_gpb(0);
	
}

void mcp23017_set_gpa(uint8_t cmd)
{	
	i2c_start_wait(MCP23017_ADDR + I2C_WRITE);
	i2c_write(GPIOA);
	i2c_write(cmd);
	i2c_stop();
};

void mcp23017_set_gpb(uint8_t cmd)
{	
	i2c_start_wait(MCP23017_ADDR + I2C_WRITE);
	i2c_write(GPIOB);
	i2c_write(cmd);
	i2c_stop();
};
