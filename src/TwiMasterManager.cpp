#include <compat/twi.h>
#include "TwiMasterManager.h"

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif

#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

#define TWI_FREQ 100000L

u8		i2cBusy, i2cState, i2cWrLen, i2cRdLen, *i2cBuffer, *i2cStatus;

//TWI low level function
void twi_Init(void) {
	// activate internal pullups for twi.
	digitalWrite(SDA, 1);
	digitalWrite(SCL, 1);

	// initialize twi prescaler and bit rate (100KHz)
	cbi(TWSR, TWPS0);
	cbi(TWSR, TWPS1);
	TWBR = ((F_CPU / TWI_FREQ) - 16) / 2;

	/* twi bit rate formula from atmega128 manual pg 204
	SCL Frequency = CPU Clock Frequency / (16 + (2 * TWBR))
	note: TWBR should be 10 or higher for master mode
	It is 72 for a 16mhz Wiring board with 100kHz TWI */

	// enable twi module, acks, and twi interrupt
	TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWEA);

	i2cBusy = 0;
}

void twi_EnableInt() {
	// enable INTs, but not START
	TWCR = _BV(TWINT) | _BV(TWEA) | _BV(TWEN) | _BV(TWIE);
}

void twi_Start() {
	//send start condition
	TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWEA) | _BV(TWINT) | _BV(TWSTA);
}

void twi_Stop(void) {
	// send stop condition
	TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWEA) | _BV(TWINT) | _BV(TWSTO);
	// wait for stop condition to be exectued on bus
	// TWINT is not set after a stop condition!
	//while (TWCR & _BV(TWSTO)) {
	//	continue;
	//}
	i2cBusy = 0;
}

void twi_Transfer(uint8_t* buffer)
{
	if (i2cBusy == 0) {
		i2cStatus = buffer;
		buffer++;
		i2cWrLen = *buffer;
		buffer++;
		i2cRdLen = *buffer;
		i2cBuffer = buffer;
		i2cBusy = 1;
		twi_Start();
	}
}

void twi_Reply(uint8_t ack) {
	// transmit master read ready signal, with or without ack
	if (ack) {
		TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWINT) | _BV(TWEA);
	}
	else {
		TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWINT);
	}
}

void twi_ReleaseBus(void) {
	// release bus
	TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWEA) | _BV(TWINT);
}

ISR(TWI_vect)
{
	switch (TW_STATUS) {
	case TW_START:			// sent start condition
	case TW_REP_START:		// sent repeated start condition
		i2cBuffer++;
		TWDR = *i2cBuffer;
		twi_Reply(1);
		break;


	case TW_MT_SLA_NACK:
		twi_Stop();
		break;

	case TW_MT_SLA_ACK:
		i2cWrLen--;
		i2cBuffer++;
		TWDR = *i2cBuffer;
		twi_Reply(1);
		break;

	case TW_MT_DATA_ACK:
		if (i2cWrLen > 0) {
			//Still have data to write
			i2cWrLen--;
			i2cBuffer++;
			TWDR = *i2cBuffer;
			twi_Reply(1);
		}
		else {
			if (i2cRdLen == 0) {
				//We have no data to read, so terminate the transfer
				twi_Stop();
			}
			else {
				//OK, we have data to read
				twi_Start();
			}
		}
		break;

	case TW_MR_DATA_ACK:
		*i2cBuffer = TWDR;
	case TW_MR_SLA_ACK:		// address for read send, ack received
		i2cRdLen--;
		i2cBuffer++;
		if (i2cRdLen > 0) {
			twi_Reply(1);
		}
		else {
			twi_Reply(0);
		}
		break;

	case TW_MR_DATA_NACK:
		*i2cBuffer = TWDR;
		twi_Stop();
		break;

	case TW_SR_STOP:
		twi_ReleaseBus();
		break;

	default:
		TWCR = (1 << TWINT) | (1 << TWEN);
		Serial.println(TW_STATUS, HEX);
		break;
	}
}

//TwiMasterManager Class
TwiMasterManager::TwiMasterManager() {

}

void TwiMasterManager::begin() {
	//Init queue buffer
	u8 i;
	for (i = 0; i < QueueSize; i++) {
		clrQueueList(i);
	}
	state = 0;
	twi_Init();
}

int8_t  TwiMasterManager::loadQueueData(u8* pBuffer) {
	u8 i;
	int8_t  result = -1;
	for (i = 0; i < QueueSize; i++) {
		if (queue[i].Bptr == NULL) {
			queue[i].Bptr = pBuffer;
			result = (int8_t)i;
			break;
		}
	}
	return result;
}

void TwiMasterManager::clrQueueList(u8 index) {
	queue[index].Bptr = NULL;
}

u8	TwiMasterManager::stateMachine(u8 state) {
	switch (state) {
	case 0:
		if (queue[queueIndex].Bptr != NULL) {
			twi_Transfer((u8*)queue[queueIndex].Bptr);
			state = 1;
		}
		else {
			queueIndex++;
			if (queueIndex == QueueSize) queueIndex = 0;
		}
		break;
	case 1:
		if (!i2cBusy) {
			queue[queueIndex].Bptr = NULL;
			queueIndex++;
			*i2cStatus = 0;
			if (queueIndex == QueueSize) queueIndex = 0;
			state = 0;
		}
		break;
	}
	return state;
}

void TwiMasterManager::process() {
	state = stateMachine(state);
}

TwiMasterManager twiMasterManager;