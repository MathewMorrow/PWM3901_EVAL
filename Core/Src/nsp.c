/*
 * mlink.c
 *
 *  Created on: Mar 23, 2024
 *      Author: mat-m
 */

#include "nsp.h"
#include "crc.h"
#include "stm32f4xx_hal.h"


UART_HandleTypeDef *uart;
uint32_t nspFrameErrorCnt;

static uint32_t nspFrameStartUs = 0;

static nspFrame_t nspRxFrame;
static nspFrame_t nspTxFrame;

#define MIN(a,b) \
  __extension__ ({ __typeof__ (a) _a = (a); \
  __typeof__ (b) _b = (b); \
  _a < _b ? _a : _b; })

/*
 * NSP "Navigation Serial Protocol"
 *
 * NSP protocol uses a single wire half duplex uart connection.
 *
 * 420000 baud
 * not inverted
 * 8 Bit
 * 1 Stop bit
 * Big endian
 *
 * 420000 bit/s = 46667 byte/s (including stop bit) = 21.43us per byte
 * Max frame size is 64 bytes - this includes the frame header and CRC
 * A 64 byte frame can be transmitted in 1372 microseconds.
 *
 * <Device address><Type><Frame length><Payload><CRC>
 *
 * Device address: (uint8_t)
 * Type:           (uint8_t)
 * Frame length:   Payload bytes - Does not include CRC
 * CRC:            (uint8_t)
 *
 */


uint8_t nspInit(UART_HandleTypeDef *huart, uint8_t deviceAddress)
{
	nspFrameErrorCnt = 0;
	uart = huart;
	nspTxFrame.deviceAddress = deviceAddress;
	UART_Receive_IT_Enable(huart); /* Move into crsfRxInit */
	UART_Transmit_IT_Enable(huart); /* Move into crsfRxInit */

	return 0;
}


void nspDataReceive(uint8_t newByte)
{
    static uint8_t nspFrameRxPosition = 0;
    const uint32_t currentTimeUs = getMicros();

    if (currentTimeUs - nspFrameStartUs > NSP_MAX_FRAME_US)
    {
        nspFrameRxPosition = 0;
        nspFrameStartUs = currentTimeUs;
    }
//    if (mlinkFrameRxPosition == 0) {
//    	mlinkFrameStartUs = currentTimeUs;
//    }

    // const int fullFrameLength = nspFrameRxPosition < 4 ? 5 : MIN(nspFrame.rxFrameLength + NSP_HEADER_CRC_SIZE, NSP_MAX_MESSAGE_SIZE);

    // <Device address><Type><Frame length><Payload><CRC>
    uint8_t rxFullFrameLength = nspFrameRxPosition < 2 ? 5 : MIN(nspRxFrame.frame.frameLength + NSP_HEADER_CRC_SIZE, NSP_MAX_MESSAGE_SIZE);

    if (nspFrameRxPosition < rxFullFrameLength)
    {
    	nspRxFrame.bytes[nspFrameRxPosition++] = newByte;
        if (nspFrameRxPosition >= fullFrameLength)
        {
            nspFrameRxPosition = 0;
            const uint8_t crc = nspRxFrameCRC();
            if (crc == crsfFrame.bytes[fullFrameLength - 1])
            {
                switch (crsfFrame.frame.type)
                {
                case CRSF_FRAMETYPE_RC_CHANNELS_PACKED:
                case CRSF_FRAMETYPE_SUBSET_RC_CHANNELS_PACKED:
                    if (crsfFrame.frame.deviceAddress == CRSF_ADDRESS_FLIGHT_CONTROLLER)
                    {
//                        rxRuntimeState->lastRcFrameTimeUs = currentTimeUs;
                        crsfFrameDone = true;
                        memcpy(&crsfChannelDataFrame, &crsfFrame, sizeof(crsfFrame));
                    }
                    break;

                case CRSF_FRAMETYPE_LINK_STATISTICS:
                    // if to FC and 10 bytes + CRSF_FRAME_ORIGIN_DEST_SIZE
                    if ((crsfFrame.frame.deviceAddress == CRSF_ADDRESS_FLIGHT_CONTROLLER) &&
                        (crsfFrame.frame.frameLength == CRSF_FRAME_ORIGIN_DEST_SIZE + CRSF_FRAME_LINK_STATISTICS_PAYLOAD_SIZE)) {
                        const crsfLinkStatistics_t* statsFrame = (const crsfLinkStatistics_t*)&crsfFrame.frame.payload;
                        handleCrsfLinkStatisticsFrame(statsFrame, currentTimeUs);
                    }
                    break;

                default:
                    break;
                }
            }
            else
            {
            	crsfFrameErrorCnt++;
            }
//			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
        }
    }
}

uint8_t nspRxFrameCRC(void)
{
	uint8_t crc = 0;
	/* CRC includes address, frame type and payload */
    for (int ii = 0; ii < NSP_HEADER_SIZE + nspRxFrame.frame.frameLength; ii++) {
        crc = crc8_dvb_s2(crc, crsfFrame.bytes[ii]);
    }
    return crc;
}



uint8_t  nspTransmit(uint8_t messageType, uint8_t numDataBytes, uint8_t *data)
{
	/* <Device address><Type><Frame length><Payload><CRC> */

	nspRxFrame.txFrame[0] = nspRxFrame.deviceAddress;
	nspRxFrame.txBuffer[1] = messageType;
	nspRxFrame.txBuffer[2] = numDataBytes;

	for(uint8_t i = 0; i < numDataBytes; i++)
	{
		nspRxFrame.txBuffer[NSP_HEADER_SIZE + i] = data[i];
	}

	nspRxFrame.txBuffer[NSP_HEADER_SIZE + numDataBytes] = crc8_dvb_s2(message[0],  NSP_HEADER_SIZE + numDataBytes);

	HAL_UART_Transmit_IT(uart, nspRxFrame.txBuffer, NSP_HEADER_CRC_SIZE + numDataBytes);

	return 0;
}

uint8_t nspTxCRC(uint8_t *data, uint8_t len)
{
	uint8_t crc = 0;
    for (int k = 0; k < len; k++)
    {
        crc = crc8_dvb_s2(crc, data[k]);
    }
    return crc;
}


























