#ifndef _TWIUTIL_h
#define _TWIUTIL_h

#include "stdint.h"

#ifndef TWI_FREQ
#define TWI_FREQ 100000L
#endif

#define  TRANSFER_DONE		0x01
#define  DEVICE_NOT_REPLY	0x80
#define  DEVICE_NACK_DATA	0x40

void twi_Init(void);
void twi_EnableInt(void);
void twi_Start(void);
void twi_Stop(void);
void twi_Reply(uint8_t ack);
void twi_ReleaseBus(void);
void twi_Transfer(uint8_t* buffer);
uint8_t twi_isBusy(void);

#endif

