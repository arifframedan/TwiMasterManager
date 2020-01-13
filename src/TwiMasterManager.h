#ifndef _TWIMASTERMANAGER_h
#define _TWIMASTERMANAGER_h

typedef struct {
	uint8_t* Bptr;
}BPTR;

#ifndef  QueueSize
#define  QueueSize   10
#endif

#define  TRANSFER_DONE		0x01
#define  DEVICE_NOT_REPLY	0x80
#define  DEVICE_NACK_DATA	0x40

class TwiMasterManager {
public:
	TwiMasterManager();
	void	begin();
	void	end();
	void	process();
	int8_t  loadQueueData(uint8_t* pBuffer);
	
private:
	BPTR	queue[QueueSize];
	uint8_t	queueIndex = 0;
	uint8_t	state;
	void	clrQueueList(uint8_t index);
	uint8_t	stateMachine(uint8_t state);
};

extern TwiMasterManager twiMasterManager;

#endif

