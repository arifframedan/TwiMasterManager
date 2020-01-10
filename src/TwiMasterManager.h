// TwiMasterManager.h

#ifndef _TWIMASTERMANAGER_h
#define _TWIMASTERMANAGER_h

#include "arduino.h"

typedef struct {
	u8* Bptr;
}BPTR;

#define  QueueSize   10

class TwiMasterManager {
public:
	TwiMasterManager();
	void	begin();
	void	process();
	int8_t  loadQueueData(u8* pBuffer);
	
private:
	BPTR	queue[QueueSize];
	u8		queueIndex = 0;
	u8		state;
	void	clrQueueList(u8 index);
	u8		stateMachine(u8 state);
};

extern TwiMasterManager twiMasterManager;

#endif

