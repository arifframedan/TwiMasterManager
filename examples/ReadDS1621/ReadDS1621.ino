/* 
   Demo sketch for testing the library using I2C temperarture sensor DS1621.
   In this sketch, we also add "nonExistDev" as dummy to demonstare the I2C non-blocking behaviour.
   There are 2 function running at difference interval (based on blink without delay),
   and each function will initiate a I2C transfer without blocking each other.
*/

#include <TwiMasterManager.h>

#define ds1621addr    0x4F            //Pin A2, A1 & A0 connected to VCC
#define slvAdd (ds1621addr<<1)
#define ds1621_AccessTemp  0xAA

uint8_t ds1621ReadTemp[] = { 0, 1, 2, slvAdd, ds1621_AccessTemp, slvAdd + 1, 0, 0 };
uint8_t ds1621Start[]    = { 0, 1, 0, slvAdd, 0xEE };
unsigned long pMillis1   = 0;
const long interval1     = 1000;      //Interval to read temperature on DS1621 is 1 second

uint8_t  nonExistDev[]   = { 0, 1, 0, 0x3A, 0x55 };  //Just put 0x3A as the slave address
unsigned long pMillis2   = 0;
const long interval2     = 700;      //Interval to try access "nonExistDev" is 0.7 second


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("TWI init");
  twiMasterManager.begin();
  Serial.println("Start command for DS1621");
  //initiate new transfer by loading buffer to twiMasterManager
  //ds1621Start[0] == 0;
  twiMasterManager.loadQueueData((uint8_t*)ds1621Start);
}

void loop() {
  // put your main code here, to run repeatedly:
  twiMasterManager.process();       //This part need to call in the main loop.
  readTemperature();
  tryAccessNonExistDev();
}

void readTemperature (){
  unsigned long cMillis = millis();
  if (cMillis - pMillis1 >= interval1){
    pMillis1 = cMillis;
    if (ds1621ReadTemp[0] == TRANSFER_DONE) {    //check previous transfer had finnish
      Serial.print("Read temperature complete. Temp : ");
      //we can read back the data if any.
      //uint16_t temp = (ds1621ReadTemp[6] << 8) | ds1621ReadTemp[7];
      uint8_t temp = ds1621ReadTemp[6];
      //For this example, we just take the MSB byte
      Serial.println(temp);
      //initiate new transfer by loading buffer to twiMasterManager
      twiMasterManager.loadQueueData((uint8_t*)ds1621ReadTemp);
    }
    else if (ds1621ReadTemp[0] == DEVICE_NOT_REPLY) {
      Serial.println("DS1621 not response!");
      ds1621ReadTemp[0] = 0;      //Clear the status
    }
  }
}

void tryAccessNonExistDev (){
  unsigned long cMillis = millis();
  if (cMillis - pMillis2 >= interval2){
    pMillis2 = cMillis;
    if (nonExistDev[0] == TRANSFER_DONE) {
      Serial.println("Initiate access to NonExistDev...");
	  //initiate new transfer by loading buffer to twiMasterManager
      twiMasterManager.loadQueueData((uint8_t*)nonExistDev);
    }
    else if (nonExistDev[0] == DEVICE_NOT_REPLY) {
      Serial.println("NonExistDev not reply...");
	  //For the sake of this demo, we manually set the status.
      nonExistDev[0] = TRANSFER_DONE;
    }
  }
}
