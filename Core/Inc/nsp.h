/*
 * mlink.h
 *
 *  Created on: Mar 23, 2024
 *      Author: mat-m
 */

#ifndef INC_NSP_H_
#define INC_NSP_H_

#define NSP_MAX_MESSAGE_SIZE  64
#define NSP_HEADER_CRC_SIZE		4
#define NSP_HEADER_SIZE 3
#define NSP_MAX_PAYLOAD_SIZE (NSP_MAX_MESSAGE_SIZE - NSP_HEADER_CRC_SIZE)
#define NSP_MAX_FRAME_US   	1500


/* <Device address><Type><Frame length><Payload><CRC> */
typedef struct nspFrameDef_s
{
    uint8_t deviceAddress;
    uint8_t rxFrametype;
    uint8_t rxFrameLength;
    uint8_t payload[NSP_MAX_PAYLOAD_SIZE];
    uint8_t crc;
} nspFrameDef_t;

typedef union nspFrame_u
{
    uint8_t bytes[CRSF_FRAME_SIZE_MAX];
    nspFrameDef_t frame;
} nspFrame_t;



#endif /* INC_NSP_H_ */
