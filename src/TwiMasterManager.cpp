
extern "C" {
#include "twiUtil.h"
}

#include "TwiMasterManager.h"
#include "arduino.h"

TwiMasterManager::TwiMasterManager() {

}

void TwiMasterManager::begin() {
	//Init queue buffer
	uint8_t i;
	for (i = 0; i < QueueSize; i++) {
		clrQueueList(i);
	}
	state = 0;
	twi_Init();
}

void TwiMasterManager::end() {
	twi_ReleaseBus();
}

int8_t  TwiMasterManager::loadQueueData(uint8_t* pBuffer) {
	uint8_t i;
	int8_t  result = -1;
	//Clear the status register (first byte in the buffer)
	*pBuffer = 0;
	//Looking for empty queue slot 
	for (i = 0; i < QueueSize; i++) {
		if (queue[i].Bptr == NULL) {
			//empty slot found & save the buffer pointer in the queue slot
			queue[i].Bptr = pBuffer;
			result = (int8_t)i;
			break;
		}
	}
	//return queue index if success or -1 if failed
	return result;
}

void TwiMasterManager::clrQueueList(uint8_t index) {
	queue[index].Bptr = NULL;
}

uint8_t	TwiMasterManager::stateMachine(uint8_t state) {
	switch (state) {
	case 0:
		if (queue[queueIndex].Bptr != NULL) {
			twi_Transfer((uint8_t*)queue[queueIndex].Bptr);
			state = 1;
		}
		else {
			queueIndex++;
			if (queueIndex == QueueSize) queueIndex = 0;
		}
		break;
	case 1:
		if (!twi_isBusy()) {
			queue[queueIndex].Bptr = NULL;
			queueIndex++;
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