/*
 * USARTRingBuffer.c
 *
 *  Created on: Jan. 14, 2021
 *      Author: Mathew
 */

#include "USARTRingBuffer.h"
//#include "stm32f4xx_hal.h"

/**
  Section: Macro Declarations
*/

#define USART_TX_BUFFER_SIZE 64
#define USART_RX_BUFFER_SIZE 64

/**
  Section: Global Variables
*/
volatile uint8_t usartTxHead = 0;
volatile uint8_t usartTxTail = 0;
volatile uint8_t usartTxBuffer[USART_TX_BUFFER_SIZE];
volatile uint8_t usartTxBufferRemaining;

volatile uint8_t usartRxHead = 0;
volatile uint8_t usartRxTail = 0;
volatile uint8_t usartRxBuffer[USART_RX_BUFFER_SIZE];
volatile uint8_t usartRxCount = 0;

/**
  Section: usart APIs
*/
void USART_DefaultFramingErrorHandler(UART_HandleTypeDef *huart);
void USART_DefaultOverrunErrorHandler(UART_HandleTypeDef *huart);
void USART_DefaultErrorHandler(UART_HandleTypeDef *huart);


bool UART_Receive_IT_Enable(UART_HandleTypeDef *huart)
{
	/* Check that a Rx process is not already ongoing */
	  if (huart->RxState == HAL_UART_STATE_READY)
	  {
	    /* Process Unlocked */
	    __HAL_UNLOCK(huart);

	    /* Enable the UART Parity Error Interrupt */
	    __HAL_UART_ENABLE_IT(huart, UART_IT_PE);

	    /* Enable the UART Error Interrupt: (Frame error, noise error, overrun error) */
	    __HAL_UART_ENABLE_IT(huart, UART_IT_ERR);

	    /* Enable the UART Data Register not empty Interrupt */
	    __HAL_UART_ENABLE_IT(huart, UART_IT_RXNE);

	    return 1;
	  }
	  else
	  {
	    return 0;
	  }
}

bool usart_is_rx_ready(void)
{
    return (usartRxCount ? true : false);
}

uint8_t usart_peek(void)
{
	if(usartRxTail == usartRxHead) return -1;
	else return usartRxBuffer[usartRxTail];
}

uint8_t usart_Read(UART_HandleTypeDef *huart)
{
    uint8_t readValue = 0;

    // If there is no data to read return 0
    if(0 == usartRxCount)
    {
        return readValue;
    }

    // Get data from RxTail and increment RxTail
    readValue = usartRxBuffer[usartRxTail++];

    // Check if the RxTail needs to wrap around to zero
    if(sizeof(usartRxBuffer) <= usartRxTail)
    {
        usartRxTail = 0;
    }

    /* Disable the UART Data Register not empty Interrupt */
    __HAL_UART_DISABLE_IT(huart, UART_IT_RXNE);
    usartRxCount--;
    /* Disable the UART Data Register not empty Interrupt */
    __HAL_UART_ENABLE_IT(huart, UART_IT_RXNE);					// Check that this is correct way to disable/enable RX interrupt for this purpose

    return readValue;
}

void usart_Receive_ISR(UART_HandleTypeDef *huart)
{
    uint32_t isrflags   = READ_REG(huart->Instance->SR);
    uint32_t cr1its     = READ_REG(huart->Instance->CR1);
    uint32_t cr3its     = READ_REG(huart->Instance->CR3); // Currently not used
    uint32_t errorflags = 0x00U;

    /* If no error occurs */
    errorflags = (isrflags & (uint32_t)(USART_SR_PE | USART_SR_FE | USART_SR_ORE | USART_SR_NE));
    if (errorflags == RESET)
    {
	   usart_RxDataHandler(huart);
	   return;
    }

    /* If some errors occur */
    if ((errorflags != RESET) && (((cr3its & USART_CR3_EIE) != RESET) || ((cr1its & (USART_CR1_RXNEIE | USART_CR1_PEIE)) != RESET)))
    {
      /* UART parity error interrupt occurred ----------------------------------*/
      if (((isrflags & USART_SR_PE) != RESET) && ((cr1its & USART_CR1_PEIE) != RESET))
      {
        huart->ErrorCode |= HAL_UART_ERROR_PE;
      }

      /* UART noise error interrupt occurred -----------------------------------*/
      if (((isrflags & USART_SR_NE) != RESET) && ((cr3its & USART_CR3_EIE) != RESET))
      {
        huart->ErrorCode |= HAL_UART_ERROR_NE;
      }

      /* UART frame error interrupt occurred -----------------------------------*/
      if (((isrflags & USART_SR_FE) != RESET) && ((cr3its & USART_CR3_EIE) != RESET))
      {
        huart->ErrorCode |= HAL_UART_ERROR_FE;
      }

      /* UART Over-Run interrupt occurred --------------------------------------*/
      if (((isrflags & USART_SR_ORE) != RESET) && (((cr1its & USART_CR1_RXNEIE) != RESET) || ((cr3its & USART_CR3_EIE) != RESET)))
      {
        huart->ErrorCode |= HAL_UART_ERROR_ORE;
      }

      /* Call UART Error Call back function if need be --------------------------*/
      if (huart->ErrorCode != HAL_UART_ERROR_NONE)
      {
        /* UART in mode Receiver -----------------------------------------------*/
        if (((isrflags & USART_SR_RXNE) != RESET) && ((cr1its & USART_CR1_RXNEIE) != RESET))
        {
           usart_RxDataHandler(huart);
        }

        /* If Overrun error occurs, or if any error occurs in DMA mode reception,
           consider error as blocking */
        if ( (huart->ErrorCode & HAL_UART_ERROR_ORE) != RESET )
        {
          /* Blocking error : transfer is aborted
             Set the UART state ready to be able to start again the process,
             Disable Rx Interrupts, and disable Rx DMA request, if ongoing */
          //UART_EndRxTransfer(huart);

        	USART_DefaultErrorHandler(huart);
        }
        else
        {
        	USART_DefaultErrorHandler(huart);
        }
      }
      huart->ErrorCode = HAL_UART_ERROR_NONE;
      return;
    } /* End if some error occurs */

 return;
}

void usart_RxDataHandler(UART_HandleTypeDef *huart)
{
	uint8_t data = 0;

	if (huart->Init.Parity == UART_PARITY_NONE) { data = (uint8_t)(huart->Instance->DR & (uint8_t)0x00FF);}
	else { data = (uint8_t)(huart->Instance->DR & (uint8_t)0x007F); }

	/* Rx process is completed, restore huart->RxState to Ready */
	huart->RxState = HAL_UART_STATE_READY;

	/* Check if the buffer is about to wrap around to tail */
	if(usartRxTail - 1 != usartRxHead)
	{
		// Store data in buffer and increment RxHead
		usartRxBuffer[usartRxHead++] = data;
		// Increment data count
		__HAL_UART_DISABLE_IT(huart, UART_IT_RXNE);
		usartRxCount++;
		__HAL_UART_ENABLE_IT(huart, UART_IT_RXNE);

		// Check if the RxHead needs to be wrapped back to zero
		if(sizeof(usartRxBuffer) <= usartRxHead)
		{
			usartRxHead = 0;
		}
	}

}

void USART_DefaultFramingErrorHandler(UART_HandleTypeDef *huart){}

void USART_DefaultOverrunErrorHandler(UART_HandleTypeDef *huart){} // usart error - restart

void USART_DefaultErrorHandler(UART_HandleTypeDef *huart){
    usart_RxDataHandler(huart);
}

// Safely return RxCount without race conditions
uint8_t getRxCount(UART_HandleTypeDef *huart)
{
  __HAL_UART_DISABLE_IT(huart, UART_IT_RXNE);
  uint8_t RxCount = usartRxCount;
  __HAL_UART_ENABLE_IT(huart, UART_IT_RXNE);
  return RxCount;
}

// Clear the receive buffer
void USART_ClearRxBuffer(UART_HandleTypeDef *huart)
{
	while(usart_is_rx_ready())
	{
		usart_Read(huart);
	}
}



bool USART_is_tx_ready(void)
{
    //return (usartTxBufferRemaining ? true : false);
	return 0;
}

bool USART_is_tx_done(void)
{
    //return TX1STAbits.TRMT;
	return 0;
}

void usart_Write(uint8_t txData)
{
//    while(0 == usartTxBufferRemaining)
//    {
//        CLRWDT();
//    }
//
//    if(0 == PIE3bits.TX1IE)
//    {
//        TX1REG = txData;
//    }
//    else
//    {
//        PIE3bits.TX1IE = 0;
//        usartTxBuffer[usartTxHead++] = txData;
//        if(sizeof(usartTxBuffer) <= usartTxHead)
//        {
//            usartTxHead = 0;
//        }
//        usartTxBufferRemaining--;
//    }
//    PIE3bits.TX1IE = 1;
}

void putch(char txData)
{
    //usart_Write(txData);
}

void usart_Transmit_ISR(void)
{

//    // add your usart interrupt custom code
//    if(sizeof(usartTxBuffer) > usartTxBufferRemaining)
//    {
//        TX1REG = usartTxBuffer[usartTxTail++];
//        if(sizeof(usartTxBuffer) <= usartTxTail)
//        {
//            usartTxTail = 0;
//        }
//        usartTxBufferRemaining++;
//    }
//    else
//    {
//        PIE3bits.TX1IE = 0;
//    }
}




/**
  End of File
*/
