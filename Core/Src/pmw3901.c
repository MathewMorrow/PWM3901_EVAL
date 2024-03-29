/*
 * pmw3901.c
 *
 *  Created on: Feb 20, 2024
 *      Author: mat-m
 *
 *      https://github.com/siuisa/pmw3901mb/blob/master/src/driver_pmw3901mb.c#L406
 *
 *      https://github.com/PX4/PX4-Autopilot/blob/main/src/drivers/optical_flow/pmw3901/PMW3901.cpp#L356
 */

#include "pmw3901.h"
#include "stm32f4xx_hal.h"
#include "stdint.h"
#include "micros.h"
#include "IIR.h"

/* SPI Peripherals */
SPI_HandleTypeDef *spi_pmw;
GPIO_TypeDef *cs_port_pmw;
uint16_t cs_pin_pmw;


/* INT1 PIN Peripherals */
GPIO_TypeDef *int_port_pmw;
uint16_t int_pin_pmw;

/* SPI const and defs */
#define WRITE_MASK 0b10000000
#define SPI_TIMEOUT 1000
uint8_t NOP_MASK = 0x00;

PMW3901_t pmw3901 = {0};
uint8_t pmw3901Rev = 256;

pt2Filter_t xSpeedFilter;
pt2Filter_t ySpeedFilter;


uint8_t PMW3901_readRegs(uint8_t reg, uint32_t *data, uint16_t len)
{
	uint8_t HAL_Error = 0;
	uint8_t regReadMask = reg;
	uint8_t dummy[16] = {0x00};

	HAL_GPIO_WritePin(cs_port_pmw , cs_pin_pmw, GPIO_PIN_RESET);

	HAL_Error = HAL_SPI_Transmit(spi_pmw, &regReadMask, 1, SPI_TIMEOUT);
	if(HAL_Error)
	{
		HAL_GPIO_WritePin(cs_port_pmw , cs_pin_pmw, GPIO_PIN_SET);
		return HAL_Error;
	}

	HAL_Error = HAL_SPI_TransmitReceive(spi_pmw, dummy, data, len, SPI_TIMEOUT);
//	HAL_Error = HAL_SPI_Receive(spi_pmw, data, len, SPI_TIMEOUT);
	if(HAL_Error)
	{
		HAL_GPIO_WritePin(cs_port_pmw , cs_pin_pmw, GPIO_PIN_SET);
		return HAL_Error;
	}

//	uint8_t testData[3] = {0x00};
//	HAL_Error = HAL_SPI_TransmitReceive(spi_pmw, dummy, testData[0], len+1, SPI_TIMEOUT);
//	for(int i=0; i<len; i++)
//	{
//		data[i] = testData[i+1];
//	}

	HAL_GPIO_WritePin(cs_port_pmw , cs_pin_pmw, GPIO_PIN_SET);

//	if(HAL_Error){ BMI2_processSPIErrors(HAL_Error);}
	return HAL_Error;
}

uint8_t PMW3901_writeReg(uint8_t reg, uint8_t value)
{
	uint8_t HAL_Error = 0;
	uint8_t regWriteMask[2] = {(WRITE_MASK | reg) ,  value };

	HAL_GPIO_WritePin(cs_port_pmw , cs_pin_pmw, GPIO_PIN_RESET);
	HAL_Error += HAL_SPI_Transmit(spi_pmw, regWriteMask, 2, SPI_TIMEOUT);
	if(HAL_Error)
	{
		HAL_GPIO_WritePin(cs_port_pmw , cs_pin_pmw, GPIO_PIN_SET);
		return HAL_Error;
	}
	HAL_GPIO_WritePin(cs_port_pmw , cs_pin_pmw, GPIO_PIN_SET);

//	if(HAL_Error){ BMI2_processSPIErrors(HAL_Error);}
	return HAL_Error;
}

uint8_t PMW3901_writeMultiple(uint8_t reg, uint8_t *data, uint16_t len)
{
	uint8_t HAL_Error = 0;
	uint8_t temp = 0;
	uint8_t regWriteMask[1] = {(WRITE_MASK | reg)};

	HAL_GPIO_WritePin(cs_port_pmw , cs_pin_pmw, GPIO_PIN_RESET);
	HAL_Error += HAL_SPI_Transmit(spi_pmw, regWriteMask, 1, SPI_TIMEOUT);
	HAL_Error += HAL_SPI_Transmit(spi_pmw, data, len, SPI_TIMEOUT);
	HAL_GPIO_WritePin(cs_port_pmw , cs_pin_pmw, GPIO_PIN_SET);
	HAL_Delay(1);

//	if(HAL_Error){ BMI2_processSPIErrors(HAL_Error);}
	return HAL_Error;
}


uint8_t PMW3901_init(SPI_HandleTypeDef *spi_handle, GPIO_TypeDef *CS_GPIO_Port, uint16_t CS_Pin, GPIO_TypeDef *INT_GPIO_Port, uint16_t INT_Pin)
{
	/* Set SPI peripherals */
	spi_pmw = spi_handle;
	cs_port_pmw = CS_GPIO_Port;
	cs_pin_pmw = CS_Pin;
	/* INT1 PIN Peripherals */
	int_port_pmw = INT_GPIO_Port;
	int_pin_pmw = INT_Pin;
	uint8_t HAL_Error = 0;


	/* Toggle SPI pins to enable SPI bus on device */
	HAL_GPIO_WritePin(cs_port_pmw, cs_pin_pmw, GPIO_PIN_SET);
	delayMicros(50);
	HAL_GPIO_WritePin(cs_port_pmw, cs_pin_pmw, GPIO_PIN_RESET);
	delayMicros(50);
	HAL_GPIO_WritePin(cs_port_pmw, cs_pin_pmw, GPIO_PIN_SET);

	/* Wait at least 40ms from power up */
	do{}while(HAL_GetTick() < 40);

	/* Power on reset */
	HAL_Error = PMW3901_PowerOnReset();
	if(HAL_Error) return HAL_Error;
	HAL_Delay(5);

	/* Read chip ID */
	uint8_t chipID = 0;
	PMW3901_readRegs(PMW_REG_PRODUCTID, &chipID, 1);
	if(chipID != PMW_CHIP_ID) return 1;

	/* Get revision number */
	HAL_Error = PMW3901_readRegs(PMW_REG_REVISIONID, &pmw3901Rev, 1);
	if(HAL_Error) return HAL_Error;

	/* Dummy read the motion registers once */
	uint8_t dummyRead = 0;
	HAL_Error = PMW3901_readRegs(0x02, &dummyRead, 1);
	if(HAL_Error) return HAL_Error;
	HAL_Error = PMW3901_readRegs(0x03, &dummyRead, 1);
	if(HAL_Error) return HAL_Error;
	HAL_Error = PMW3901_readRegs(0x04, &dummyRead, 1);
	if(HAL_Error) return HAL_Error;
	HAL_Error = PMW3901_readRegs(0x05, &dummyRead, 1);
	if(HAL_Error) return HAL_Error;
	HAL_Error = PMW3901_readRegs(0x06, &dummyRead, 1);
	if(HAL_Error) return HAL_Error;
	HAL_Delay(1);

	HAL_Error = PMW3901_WriteConfiguration();
	if(HAL_Error) return HAL_Error;

	HAL_Error = PMW3901_SetInterrupt();
	if(HAL_Error) return HAL_Error;

	  /* Initialize Filters */
	  float filterGain = pt2FilterGain(5.0f, 0.01f);
	  pt2FilterInit(&xSpeedFilter,filterGain);
	  pt2FilterInit(&ySpeedFilter,filterGain);

	return 0;
}

uint8_t PMW3901_PowerOnReset()
{
	return PMW3901_writeReg(0x3A, 0x5A);
}

uint8_t PMW3901_Process()
{
	uint8_t rslt = 1;
	static uint32_t lastMicros = 0;

	uint32_t deltaMicros = getMicros() - lastMicros;

	if(deltaMicros >= 10000)
	{
		PMW3901_ReadMotionBulk();

		if(pmw3901.isValid)
		{
			pmw3901.isValid = 0;

			lastMicros = getMicros();

			pmw3901.deltaMicros = deltaMicros;

			pmw3901.xDisplacement = (float)pmw3901.deltaX * 0.025 * 1.0f;
			pmw3901.yDisplacement = (float)pmw3901.deltaY * 0.025 * 1.0f;;

			pmw3901.xDisplacementSum += pmw3901.xDisplacement;
			pmw3901.yDisplacementSum += pmw3901.yDisplacement;

			float xVeloticty = (float)pmw3901.xDisplacement / 0.01;
			float yVeloticty = (float)pmw3901.yDisplacement / 0.01;

			pmw3901.xVelocity = pt2FilterApply(&xSpeedFilter, xVeloticty);
			pmw3901.yVelocity = pt2FilterApply(&ySpeedFilter, yVeloticty);

			rslt = 0;
		}
	}
	return rslt;
}

uint8_t  PMW3901_SetInterrupt()
{
	/* Set the motion reg (0x02) to 0x01 to enable interrupt? */
//	return PMW3901_writeReg(0x3F, 0x10);

	/* Set MOTION_CONTROL reg (0x0F) to configure pin polarity? */
	/* ChatGPT says Bit-2 to 0 for active high, bit-1 to 0 for clear on read of MOTION reg */
	/* All other regs are reserved */
	PMW3901_writeReg(0x3F, 0b11);
}

uint8_t PMW3901_IsDataReady()
{
	return HAL_GPIO_ReadPin(int_port_pmw, int_pin_pmw);
}


uint8_t PMW3901_ReadMotion()
{
	uint8_t rslt = 0;
	uint8_t data[6] = {0};

	for(int i = 0; i<6; i++)
	{
		rslt += PMW3901_readRegs(PWM_REG_MOTION + i, &data[i], 1);
	}

	if(rslt == 0)
	{
		pmw3901.motion = data[0];
		pmw3901.deltaX = ((int16_t)data[2] << 8) | data[1];
		pmw3901.deltaY = ((int16_t)data[4] << 8) | data[3];
		pmw3901.squal = data[5];
	}

	return rslt;
}


uint8_t PMW3901_ReadMotionBulk()
{
	uint8_t rslt = 0;
	uint8_t data[12] = {0};

	rslt = PMW3901_readRegs(PMW_REG_MOTION_BURST, &data[0], 12);

	if(rslt == 0)
	{
		if(data[0] && (1 << 7))
		{
//			if( (data[6] < 0x19) || (data[10] == 0x1F) )
//			{
//				pmw3901.isValid = 0;
//			}
//			else
//			{
				pmw3901.isValid = 1;
				pmw3901.deltaX = (int16_t)(((uint16_t)data[3] << 8) | data[2]);                /* set delta_x */
				pmw3901.deltaY = (int16_t)(((uint16_t)data[5] << 8) | data[4]);                /* set delta_y */
				pmw3901.observation = data[1] & 0x3F;                                                  /* set observation */
				pmw3901.rawAverage = data[7];                                                         /* set raw average */
				pmw3901.rawMax = data[8];                                                             /* set raw max */
				pmw3901.rawMin = data[9];                                                             /* set raw min */
				pmw3901.shutter = (((((uint16_t)data[10] & 0x1F) << 8)) | data[11]);            /* set shutter */
//				pmw3901.squal = data[6] * 4;
				pmw3901.squal = data[6];
//			}
		}
	}
	else
	{
		pmw3901.isValid = 0;
	}

	return rslt;
}

//uint8_t PMW3901_WriteConfiguration()
//{
//	// set performance optimization registers
//	// from PixArt PMW3901MB Optical Motion Tracking chip demo kit V3.20 (21 Aug 2018)
//	unsigned char v = 0;
//	unsigned char c1 = 0;
//	unsigned char c2 = 0;
//
//	PMW3901_writeReg(0x7F, 0x00);
//	PMW3901_writeReg(0x55, 0x01);
//	PMW3901_writeReg(0x50, 0x07);
//	PMW3901_writeReg(0x7f, 0x0e);
//	PMW3901_writeReg(0x43, 0x10);
//
//	PMW3901_readRegs(0x67, &v, 1);
//
//	// if bit7 is set
//	if (v & (1 << 7)) {
//		PMW3901_writeReg(0x48, 0x04);
//
//	} else {
//		PMW3901_writeReg(0x48, 0x02);
//	}
//
//	PMW3901_writeReg(0x7F, 0x00);
//	PMW3901_writeReg(0x51, 0x7b);
//	PMW3901_writeReg(0x50, 0x00);
//	PMW3901_writeReg(0x55, 0x00);
//
//	PMW3901_writeReg(0x7F, 0x0e);
//	PMW3901_readRegs(0x73, &v, 1);
//
//	if (v == 0) {
//		PMW3901_readRegs(0x70, &c1, 1);
//
//		if (c1 <= 28) {
//			c1 = c1 + 14;
//
//		} else {
//			c1 = c1 + 11;
//		}
//
//		if (c1 > 0x3F) {
//			c1 = 0x3F;
//		}
//
//		PMW3901_readRegs(0x71, &c2, 1);
//		c2 = ((unsigned short)c2 * 45) / 100;
//
//		PMW3901_writeReg(0x7f, 0x00);
//		PMW3901_writeReg(0x61, 0xAD);
//		PMW3901_writeReg(0x51, 0x70);
//		PMW3901_writeReg(0x7f, 0x0e);
//		PMW3901_writeReg(0x70, c1);
//		PMW3901_writeReg(0x71, c2);
//	}
//
//	PMW3901_writeReg(0x7F, 0x00);
//	PMW3901_writeReg(0x61, 0xAD);
//	PMW3901_writeReg(0x7F, 0x03);
//	PMW3901_writeReg(0x40, 0x00);
//	PMW3901_writeReg(0x7F, 0x05);
//	PMW3901_writeReg(0x41, 0xB3);
//	PMW3901_writeReg(0x43, 0xF1);
//	PMW3901_writeReg(0x45, 0x14);
//	PMW3901_writeReg(0x5B, 0x32);
//	PMW3901_writeReg(0x5F, 0x34);
//	PMW3901_writeReg(0x7B, 0x08);
//	PMW3901_writeReg(0x7F, 0x06);
//	PMW3901_writeReg(0x44, 0x1B);
//	PMW3901_writeReg(0x40, 0xBF);
//	PMW3901_writeReg(0x4E, 0x3F);
//	PMW3901_writeReg(0x7F, 0x08);
//	PMW3901_writeReg(0x65, 0x20);
//	PMW3901_writeReg(0x6A, 0x18);
//	PMW3901_writeReg(0x7F, 0x09);
//	PMW3901_writeReg(0x4F, 0xAF);
//	PMW3901_writeReg(0x5F, 0x40);
//	PMW3901_writeReg(0x48, 0x80);
//	PMW3901_writeReg(0x49, 0x80);
//	PMW3901_writeReg(0x57, 0x77);
//	PMW3901_writeReg(0x60, 0x78);
//	PMW3901_writeReg(0x61, 0x78);
//	PMW3901_writeReg(0x62, 0x08);
//	PMW3901_writeReg(0x63, 0x50);
//	PMW3901_writeReg(0x7F, 0x0A);
//	PMW3901_writeReg(0x45, 0x60);
//	PMW3901_writeReg(0x7F, 0x00);
//	PMW3901_writeReg(0x4D, 0x11);
//	PMW3901_writeReg(0x55, 0x80);
//	PMW3901_writeReg(0x74, 0x21);
//	PMW3901_writeReg(0x75, 0x1F);
//	PMW3901_writeReg(0x4A, 0x78);
//	PMW3901_writeReg(0x4B, 0x78);
//	PMW3901_writeReg(0x44, 0x08);
//	PMW3901_writeReg(0x45, 0x50);
//	PMW3901_writeReg(0x64, 0xFF);
//	PMW3901_writeReg(0x65, 0x1F);
//	PMW3901_writeReg(0x7F, 0x14);
//	PMW3901_writeReg(0x65, 0x67);
//	PMW3901_writeReg(0x66, 0x08);
//	PMW3901_writeReg(0x63, 0x70);
//	PMW3901_writeReg(0x7F, 0x15);
//	PMW3901_writeReg(0x48, 0x48);
//	PMW3901_writeReg(0x7F, 0x07);
//	PMW3901_writeReg(0x41, 0x0D);
//	PMW3901_writeReg(0x43, 0x14);
//	PMW3901_writeReg(0x4B, 0x0E);
//	PMW3901_writeReg(0x45, 0x0F);
//	PMW3901_writeReg(0x44, 0x42);
//	PMW3901_writeReg(0x4C, 0x80);
//	PMW3901_writeReg(0x7F, 0x10);
//	PMW3901_writeReg(0x5B, 0x02);
//	PMW3901_writeReg(0x7F, 0x07);
//	PMW3901_writeReg(0x40, 0x41);
//	PMW3901_writeReg(0x70, 0x00);
//
//	HAL_Delay(10); // delay 10ms
//
//	PMW3901_writeReg(0x32, 0x44);
//	PMW3901_writeReg(0x7F, 0x07);
//	PMW3901_writeReg(0x40, 0x40);
//	PMW3901_writeReg(0x7F, 0x06);
//	PMW3901_writeReg(0x62, 0xF0);
//	PMW3901_writeReg(0x63, 0x00);
//	PMW3901_writeReg(0x7F, 0x0D);
//	PMW3901_writeReg(0x48, 0xC0);
//	PMW3901_writeReg(0x6F, 0xD5);
//	PMW3901_writeReg(0x7F, 0x00);
//	PMW3901_writeReg(0x5B, 0xA0);
//	PMW3901_writeReg(0x4E, 0xA8);
//	PMW3901_writeReg(0x5A, 0x50);
//	PMW3901_writeReg(0x40, 0x80);
//}

uint8_t PMW3901_WriteConfiguration()
{
	uint8_t HAL_Error = 0;
	HAL_Error = PMW3901_writeReg(0x7F, 0x00);
	if(HAL_Error) return HAL_Error;
	HAL_Error = PMW3901_writeReg(0x61, 0xAD);
	if(HAL_Error) return HAL_Error;
	HAL_Error = PMW3901_writeReg(0x7F, 0x03);
	if(HAL_Error) return HAL_Error;
	HAL_Error = PMW3901_writeReg(0x40, 0x00);
	if(HAL_Error) return HAL_Error;
	HAL_Error =   PMW3901_writeReg(0x7F, 0x05);
	if(HAL_Error) return HAL_Error;
	HAL_Error =   PMW3901_writeReg(0x41, 0xB3);
	if(HAL_Error) return HAL_Error;
	HAL_Error =   PMW3901_writeReg(0x43, 0xF1);
	if(HAL_Error) return HAL_Error;
	HAL_Error =   PMW3901_writeReg(0x45, 0x14);
	if(HAL_Error) return HAL_Error;
	HAL_Error =   PMW3901_writeReg(0x5B, 0x32);
	if(HAL_Error) return HAL_Error;
	HAL_Error =   PMW3901_writeReg(0x5F, 0x34);
	if(HAL_Error) return HAL_Error;
	HAL_Error =   PMW3901_writeReg(0x7B, 0x08);
	if(HAL_Error) return HAL_Error;
	HAL_Error =   PMW3901_writeReg(0x7F, 0x06);
	if(HAL_Error) return HAL_Error;
	HAL_Error =   PMW3901_writeReg(0x44, 0x1B);
	if(HAL_Error) return HAL_Error;
	HAL_Error =   PMW3901_writeReg(0x40, 0xBF);
	if(HAL_Error) return HAL_Error;
	HAL_Error =   PMW3901_writeReg(0x4E, 0x3F);
	if(HAL_Error) return HAL_Error;
	HAL_Error =   PMW3901_writeReg(0x7F, 0x08);
	if(HAL_Error) return HAL_Error;
	HAL_Error =   PMW3901_writeReg(0x65, 0x20);
	if(HAL_Error) return HAL_Error;
	HAL_Error =   PMW3901_writeReg(0x6A, 0x18);
	if(HAL_Error) return HAL_Error;
	HAL_Error =   PMW3901_writeReg(0x7F, 0x09);
	if(HAL_Error) return HAL_Error;
	HAL_Error =   PMW3901_writeReg(0x4F, 0xAF);
	if(HAL_Error) return HAL_Error;
	HAL_Error =   PMW3901_writeReg(0x5F, 0x40);
	if(HAL_Error) return HAL_Error;
	HAL_Error =   PMW3901_writeReg(0x48, 0x80);
	if(HAL_Error) return HAL_Error;
	HAL_Error =   PMW3901_writeReg(0x49, 0x80);
	if(HAL_Error) return HAL_Error;
	HAL_Error =   PMW3901_writeReg(0x57, 0x77);
	if(HAL_Error) return HAL_Error;
	HAL_Error =   PMW3901_writeReg(0x60, 0x78);
	if(HAL_Error) return HAL_Error;
	HAL_Error =   PMW3901_writeReg(0x61, 0x78);
	if(HAL_Error) return HAL_Error;
	HAL_Error =   PMW3901_writeReg(0x62, 0x08);
	if(HAL_Error) return HAL_Error;
	HAL_Error =   PMW3901_writeReg(0x63, 0x50);
	if(HAL_Error) return HAL_Error;
	HAL_Error =   PMW3901_writeReg(0x7F, 0x0A);
	if(HAL_Error) return HAL_Error;
	HAL_Error =   PMW3901_writeReg(0x45, 0x60);
	if(HAL_Error) return HAL_Error;
	HAL_Error =   PMW3901_writeReg(0x7F, 0x00);
	if(HAL_Error) return HAL_Error;
	HAL_Error =   PMW3901_writeReg(0x4D, 0x11);
	if(HAL_Error) return HAL_Error;
	HAL_Error =   PMW3901_writeReg(0x55, 0x80);
	if(HAL_Error) return HAL_Error;
	HAL_Error =   PMW3901_writeReg(0x74, 0x1F);
	if(HAL_Error) return HAL_Error;
	HAL_Error =   PMW3901_writeReg(0x75, 0x1F);
	if(HAL_Error) return HAL_Error;
	HAL_Error =   PMW3901_writeReg(0x4A, 0x78);
	if(HAL_Error) return HAL_Error;
	HAL_Error =   PMW3901_writeReg(0x4B, 0x78);
	if(HAL_Error) return HAL_Error;
	HAL_Error =   PMW3901_writeReg(0x44, 0x08);
	if(HAL_Error) return HAL_Error;
	HAL_Error =   PMW3901_writeReg(0x45, 0x50);
	if(HAL_Error) return HAL_Error;
	HAL_Error =   PMW3901_writeReg(0x64, 0xFF);
	if(HAL_Error) return HAL_Error;
	HAL_Error =   PMW3901_writeReg(0x65, 0x1F);
	if(HAL_Error) return HAL_Error;
	HAL_Error =   PMW3901_writeReg(0x7F, 0x14);
	if(HAL_Error) return HAL_Error;
	HAL_Error = PMW3901_writeReg(0x65, 0x60);
	if(HAL_Error) return HAL_Error;
	HAL_Error = PMW3901_writeReg(0x66, 0x08);
	if(HAL_Error) return HAL_Error;
	HAL_Error = PMW3901_writeReg(0x63, 0x78);
	if(HAL_Error) return HAL_Error;
	HAL_Error = PMW3901_writeReg(0x7F, 0x15);
	if(HAL_Error) return HAL_Error;
	HAL_Error = PMW3901_writeReg(0x48, 0x58);
	if(HAL_Error) return HAL_Error;
	HAL_Error = PMW3901_writeReg(0x7F, 0x07);
	if(HAL_Error) return HAL_Error;
	HAL_Error = PMW3901_writeReg(0x41, 0x0D);
	if(HAL_Error) return HAL_Error;
	HAL_Error = PMW3901_writeReg(0x43, 0x14);
	if(HAL_Error) return HAL_Error;
	HAL_Error = PMW3901_writeReg(0x4B, 0x0E);
	if(HAL_Error) return HAL_Error;
	HAL_Error = PMW3901_writeReg(0x45, 0x0F);
	if(HAL_Error) return HAL_Error;
	HAL_Error = PMW3901_writeReg(0x44, 0x42);
	if(HAL_Error) return HAL_Error;
	HAL_Error = PMW3901_writeReg(0x4C, 0x80);
	if(HAL_Error) return HAL_Error;
	HAL_Error = PMW3901_writeReg(0x7F, 0x10);
	if(HAL_Error) return HAL_Error;
	HAL_Error = PMW3901_writeReg(0x5B, 0x02);
	if(HAL_Error) return HAL_Error;
	HAL_Error = PMW3901_writeReg(0x7F, 0x07);
	if(HAL_Error) return HAL_Error;
	HAL_Error = PMW3901_writeReg(0x40, 0x41);
	if(HAL_Error) return HAL_Error;
	HAL_Error = PMW3901_writeReg(0x70, 0x00);
	if(HAL_Error) return HAL_Error;

	HAL_Delay(100);
	HAL_Error = PMW3901_writeReg(0x32, 0x44);
	if(HAL_Error) return HAL_Error;
	HAL_Error = PMW3901_writeReg(0x7F, 0x07);
	if(HAL_Error) return HAL_Error;
	HAL_Error = PMW3901_writeReg(0x40, 0x40);
	if(HAL_Error) return HAL_Error;
	HAL_Error = PMW3901_writeReg(0x7F, 0x06);
	if(HAL_Error) return HAL_Error;
	HAL_Error = PMW3901_writeReg(0x62, 0xf0);
	if(HAL_Error) return HAL_Error;
	HAL_Error = PMW3901_writeReg(0x63, 0x00);
	if(HAL_Error) return HAL_Error;
	HAL_Error = PMW3901_writeReg(0x7F, 0x0D);
	if(HAL_Error) return HAL_Error;
	HAL_Error = PMW3901_writeReg(0x48, 0xC0);
	if(HAL_Error) return HAL_Error;
	HAL_Error = PMW3901_writeReg(0x6F, 0xd5);
	if(HAL_Error) return HAL_Error;
	HAL_Error = PMW3901_writeReg(0x7F, 0x00);
	if(HAL_Error) return HAL_Error;
	HAL_Error = PMW3901_writeReg(0x5B, 0xa0);
	if(HAL_Error) return HAL_Error;
	HAL_Error = PMW3901_writeReg(0x4E, 0xA8);
	if(HAL_Error) return HAL_Error;
	HAL_Error = PMW3901_writeReg(0x5A, 0x50);
	if(HAL_Error) return HAL_Error;
	HAL_Error = PMW3901_writeReg(0x40, 0x80);
	if(HAL_Error) return HAL_Error;


	return 0;
}

















