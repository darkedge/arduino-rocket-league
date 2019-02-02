#include <RH_ASK.h>
#include <SPI.h>

#define INPUT_PIN (7)

// Garden side                                                         
//         +------------------------------------------------+          
//         |                                                |          
//         |                                                |          
//  +------+                                                +------+   
//  |      |                                                |      |   
//  | Left |                    Middle                      |Right |   
//  |      |                                                |      |   
//  |      |                                                |      |   
//  +------+                                                +------+   
//         |                                                |          
//         |                                                |          
//         +------------------------------------------------+          
// Street side


static RH_ASK s_Driver(2000, INPUT_PIN);

/**
 * Initialization.
 */
void setup()
{
  Serial.begin(9600);
  if (!s_Driver.init())
  {
    Serial.println("RadioHead lib: Driver init failed");
  }
}


/**
 * Main loop.
 */
void loop()
{
  uint8_t buf[RH_ASK_MAX_MESSAGE_LEN];
  uint8_t buflen = sizeof(buf);
 
  if (s_Driver.recv(buf, &buflen)) {
    s_Driver.printBuffer("Received: ", buf, buflen);
    //buf[buflen] = '\0';
    //Serial.println((const char*)buf);
  }

  // Print a heartbeat just to make sure we're connected.
  static uint32_t heartBeatTicks;
  if(heartBeatTicks++ == 100000) // Good enough for the Arduino Nano.
  {
    Serial.println("Heartbeat");
    heartBeatTicks = 0;
  }
}
