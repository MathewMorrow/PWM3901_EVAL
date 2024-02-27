/*
 * USARTRingBuffer.h
 *
 *  Created on: Jan. 14, 2021
 *      Author: Mathew
 */

#ifndef INC_USARTRINGBUFFER_H_
#define INC_USARTRINGBUFFER_H_

/**
  Section: Included Files
*/

#include "main.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_uart.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>



// Section: Macro Declarations
#define usart_DataReady  (usart_is_rx_ready())


// Section: Global variables
extern volatile uint8_t usartTxBufferRemaining;
extern volatile uint8_t usartRxCount;

bool UART_Receive_IT_Enable(UART_HandleTypeDef *huart);

bool usart_is_tx_ready(void);


bool usart_is_rx_ready(void);


bool usart_is_tx_done(void);


uint8_t usart_Read(UART_HandleTypeDef *huart);


uint8_t usart_peek(void);


void USART_ClearRxBuffer(UART_HandleTypeDef *huart);


void usart_Write(uint8_t txData);


void usart_Transmit_ISR(void);


void usart_Receive_ISR(UART_HandleTypeDef *huart);


void usart_RxDataHandler(UART_HandleTypeDef *huart);

// Safely return RxCount without race conditions
uint8_t getRxCount(UART_HandleTypeDef *huart);


#endif /* INC_USARTRINGBUFFER_H_ */
