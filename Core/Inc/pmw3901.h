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
#define PWM_REG_MOTION 0x02
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
#define PMW_REG_MOTION_BURST 0x016
#define PMW_REG_POWER_UP_RESET 0x3A
#define PMW_REG_SHUTDOWN 0x3B
#define PMW_REG_RAW_DATA_GRAB 0x58
#define PMW_REG_RAW_DATA_GRAB_STATUS 0x59
#define PMW_REG_INVERSE_PRODUCT_ID 0x5F

#define PMW_CHIP_ID 0x49

extern uint8_t pmw3901Rev;

/* Structure to hold register data */
typedef struct PMW3901_s{

	uint16_t motion;
	int16_t deltaX;
	int16_t deltaY;
	uint8_t squal;
	uint16_t shuttderData;
	uint8_t observation;
	uint8_t rawAverage;
	uint8_t rawMax;
	uint8_t rawMin;
	uint16_t shutter;

	uint8_t isValid;
	float xDisplacement;
	float yDisplacement;
	float xVelocity;
	float yVelocity;

} PMW3901_t;

extern PMW3901_t pmw3901;


typedef struct motionBurst_s {
  union {
    uint8_t motion;
    struct {
      uint8_t frameFrom0    : 1;
      uint8_t runMode       : 2;
      uint8_t reserved1     : 1;
      uint8_t rawFrom0      : 1;
      uint8_t reserved2     : 2;
      uint8_t motionOccurred: 1;
    };
  };

  uint8_t observation;
  int16_t deltaX;
  int16_t deltaY;

  uint8_t squal;

  uint8_t rawDataSum;
  uint8_t maxRawData;
  uint8_t minRawData;

  uint16_t shutter;
} __attribute__((packed)) motionBurst_t;

uint8_t PMW3901_readRegs(uint8_t reg, uint32_t *data, uint16_t len);

uint8_t PMW3901_writeReg(uint8_t reg, uint8_t value);

uint8_t PMW3901_writeMultiple(uint8_t reg, uint8_t *data, uint16_t len);


uint8_t PMW3901_init(SPI_HandleTypeDef *spi_handle, GPIO_TypeDef *CS_GPIO_Port, uint16_t CS_Pin, GPIO_TypeDef *INT_GPIO_Port, uint16_t INT_Pin);

uint8_t PMW3901_PowerOnReset();

uint8_t PMW3901_WriteConfiguration();

uint8_t  PMW3901_SetInterrupt();

uint8_t PMW3901_IsDataReady();


uint8_t PMW3901_ReadMotion();


uint8_t  PMW3901_ReadMotionBulk();


#endif /* INC_PMW3901_H_ */
