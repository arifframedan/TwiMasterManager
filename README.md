# TwiMasterManager

This library featuring a Two Wire Interface (TWI/I2C) master manager with non-blocking capablities.
ONLY support single master (Arduino UNO/NANO - Atmega328p as a Master) and multiple slave.

## Usage

Just include the header as below:

`#include <TwiMasterManager.h>`

This library provides you functions as below:

- ***begin()*** , initialize the library.

	`twiMasterManager.begin();`

- ***end()*** , to stop the library and release the I/O pins

	`twiMasterManager.end();`

- ***process()*** , to run the library core. This function MUST be call rapidly in the loop.
	```
	void loop (){
		...
		twiMasterManager.process();
		...
	}
	```

- ***loadQueueData(uint8_t* pBuffer)*** , to put the buffer into the queue. The buffer need to follow structure below.

	1) Write & Read Transfer
	
		**{Status, NumOfWriteByte, NumOfReadByte, Add+W, WriteByte0, ..., Add+R, ReadByte0, ...}**

		Example (base on DS1621 read temperature command)
		```
		uint8_t ds1621ReadTemp[] = { 0, 1, 2, Add+W, ds1621_AccessTemp, Add+R, 0, 0 };
		twiMasterManager.loadQueueData((uint8_t*)ds1621ReadTemp);
		```


	2) Write ONLY Transfer
	
		**{Status, NumOfWriteByte, 0, Add+W, WriteByte0, ...}**

		Example (base on DS1621 start command)
		```
		uint8_t ds1621Start[] = { 0, 1, 0, Add+W, 0xEE };
		twiMasterManager.loadQueueData((uint8_t*)ds1621Start);
		```

The default queue size is 10, but you can set your own.

	`#define QueueSize	yourqueuesize`

## Example

- ReadDS1621
