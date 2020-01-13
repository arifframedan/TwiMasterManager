#include <compat/twi.h>
#include "arduino.h"
#include "pins_arduino.h"
#include "twiUtil.h"

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif

#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

uint8_t		i2cBusy, i2cState, i2cWrLen, i2cRdLen, * i2cBuffer, * i2cStatus;

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

uint8_t twi_isBusy(void) {
	return i2cBusy;
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


	case TW_MT_SLA_NACK:		//NO slave device ACK
		twi_Stop();
		*i2cStatus = DEVICE_NOT_REPLY;
		break;

	case TW_MT_DATA_NACK:	// data sent, nack received
		twi_Stop();
		*i2cStatus = DEVICE_NACK_DATA;
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
				*i2cStatus = TRANSFER_DONE;
			}
			else {
				//OK, we have data to read
				twi_Start();
			}
		}
		break;

	case TW_MR_DATA_ACK:	//Master read data with ACK
		*i2cBuffer = TWDR;
	case TW_MR_SLA_ACK:		//Slave ACK with "slvadd+R"
		i2cRdLen--;
		i2cBuffer++;
		if (i2cRdLen > 0) {
			twi_Reply(1);
		}
		else {
			twi_Reply(0);
		}
		break;

	case TW_MR_DATA_NACK:	//last data, Master read data with NACK
		*i2cBuffer = TWDR;
		twi_Stop();
		*i2cStatus = TRANSFER_DONE;
		break;

	case TW_SR_STOP:
		twi_ReleaseBus();
		break;

	default:
		TWCR = (1 << TWINT) | (1 << TWEN);
		break;
	}
}