

#ifndef __UART_H__
#define __UART_H__

#include "RingBuffer.h"


#define UART_BAUD_SELECT(baudRate,xtalCpu)  (((xtalCpu) + 8UL * (baudRate)) / (16UL * (baudRate)) -1UL)

#define UART_BAUD_SELECT_DOUBLE_SPEED(baudRate,xtalCpu) ( ((((xtalCpu) + 4UL * (baudRate)) / (8UL * (baudRate)) -1UL)) | 0x8000)


#ifndef UART_RX_BUFFER_SIZE
	#define UART_RX_BUFFER_SIZE 32
#endif

#ifndef UART_TX_BUFFER_SIZE
<<<<<<< .mine
	#define UART_TX_BUFFER_SIZE 32
||||||| .r47
#define UART_TX_BUFFER_SIZE 32
=======
#define UART_TX_BUFFER_SIZE 256
>>>>>>> .r50
#endif


#if ( (UART_RX_BUFFER_SIZE+UART_TX_BUFFER_SIZE) >= (RAMEND-0x60 ) )
#error "size of UART_RX_BUFFER_SIZE + UART_TX_BUFFER_SIZE larger than size of SRAM"
#endif


void		uartInit( unsigned int baudrate );

void		uartPutByte( uint8_t byte );

void		uartPutByteStr( uint8_t *str , uint8_t len );

uint8_t		*uartReadRingBuff( uint8_t *stream , uint8_t Reset );

#endif // #ifndef __UART_H_

