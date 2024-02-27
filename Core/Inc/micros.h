/*
 * micros.h
 *
 *  Created on: Feb. 27, 2024
 *      Author: Mathew Morrow
 */

#ifndef INC_MICROS_H_
#define INC_MICROS_H_

#include "stm32f4xx_hal.h"

void microsInit(TIM_HandleTypeDef *_microsTimer);

uint32_t getMicros();

void delayMicros(uint32_t micros);


#endif /* INC_MICROS_H_ */
