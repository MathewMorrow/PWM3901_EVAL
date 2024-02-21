/*
 * pmw3901.h
 *
 *  Created on: Feb 20, 2024
 *      Author: mat-m
 */

#ifndef INC_PMW3901_H_
#define INC_PMW3901_H_

#include "stm32f4xx_hal.h"
#include "stdint.h"


#define PMW_REG_PRODUCTID 0x00
#define PMW_REG_REVISIONID 0x01
#define PWM_REG_MOON 0x02
#define PMW_REG_DELTA_X_L 0x03
#define PMW_REG_DELTA_X_H 0x04
#define PMW_REG_DELTA_Y_L 0x05
#define PMW_REG_DELTA_Y_H 0x06
#define PMW_REG_SQUAL 0x07
#define PMW_REG_RAW_DATA_SUM 0x08
#define PMW_REG_MAX_RAW_DATA 0x09
#define PMW_REG_MIN_RAW_DATA 0x0A
#define PMW_REG_SHUER_LOWER 0x0B
#define PMW_REG_SHUER_UPPER 0x0C
#define PMW_REG_OBSERVATION 0x15
#define PMW_REG_MOON_BURST 0x016
#define PMW_REG_POWER_UP_RESET 0x3A
#define PMW_REG_SHUTDOWN 0x3B
#define PMW_REG_RAW_DATA_GRAB 0x58
#define PMW_REG_RAW_DATA_GRAB_STATUS 0x59
#define PMW_REG_INVERSE_PRODUCT_ID 0x5F

#define PMW_CHIP_ID 0x49


#endif /* INC_PMW3901_H_ */
