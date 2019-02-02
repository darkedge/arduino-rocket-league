#include <RH_ASK.h>
#include <SPI.h>

#define INPUT_LEFT   (5)
#define INPUT_MIDDLE (6)
#define INPUT_RIGHT  (7)

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


static RH_ASK s_Left(2000, INPUT_LEFT, 12);
static RH_ASK s_Middle(2000, INPUT_MIDDLE, 12);
static RH_ASK s_Right(2000, INPUT_RIGHT, 12);

/**
 * Initialization.
 */
void setup()
{
  Serial.begin(9600);
  if (!s_Left.init())
  {
    Serial.println("RadioHead lib: Left driver init failed");
  }
  if (!s_Middle.init())
  {
    Serial.println("RadioHead lib: Middle driver init failed");
  }
  if (!s_Right.init())
  {
    Serial.println("RadioHead lib: Right driver init failed");
  }
}


/**
 * Main loop.
 */
void loop()
{
  uint8_t buf[RH_ASK_MAX_MESSAGE_LEN];
  uint8_t buflen = sizeof(buf);
 
  if (s_Left.recv(buf, &buflen)) {
    s_Left.printBuffer("Left sensor received:", buf, buflen);
    //buf[buflen] = '\0';
    //Serial.println((const char*)buf);
  }
  if (s_Middle.recv(buf, &buflen)) {
    s_Middle.printBuffer("Middle sensor received:", buf, buflen);
    //buf[buflen] = '\0';
    //Serial.println((const char*)buf);
  }
  if (s_Right.recv(buf, &buflen)) {
    s_Right.printBuffer("Right sensor received:", buf, buflen);
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
