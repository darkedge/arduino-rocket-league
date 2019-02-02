#include <RH_ASK.h>
#include <SPI.h> 

#define OUTPUT_PIN (12)
#define MAGIC_VALUE (0x504c414e) // "PLAN" (PlatteLAN)

RH_ASK driver(2000, 11, OUTPUT_PIN);

/**
 * 
 */
void setup()
{ 
  Serial.begin(9600);   
  if (!driver.init()) Serial.println("init failed");
}

/**
 * 
 */
void loop()
{
  uint32_t plan = MAGIC_VALUE;
  driver.send((uint8_t *)&plan, sizeof(plan));
  driver.waitPacketSent(); // TODO: Does this wait for longer than a millisecond?

  // Print a heartbeat just to make sure we're connected.
  static uint32_t heartBeatTicks;
  if(heartBeatTicks++ == 55)
  {
    Serial.println("Heartbeat");
    heartBeatTicks = 0;
  }
}
