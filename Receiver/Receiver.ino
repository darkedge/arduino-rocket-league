#include <RH_ASK.h>
#include <SPI.h>

#define INPUT_PIN (7)
#define OUTPUT_PIN (8)
#define MAGIC_VALUE (0x504c414e) // "PLAN" (PlatteLAN)
#define LINGER_MS (1000) // 1 second linger time when detecting the ball

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
static uint16_t s_Linger; // Current linger time

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
 * Executed every millisecond.
 */
static void DoMillisecond()
{
  if (s_Linger == 1)
  {
    Serial.println("Output pin low.");
    digitalWrite(LED_BUILTIN, LOW);
  }

  // Keep setting pin every millisecond
  if (s_Linger > 0)
  {
    digitalWrite(OUTPUT_PIN, HIGH);
    s_Linger--;
  }
  else if (s_Linger == 0)
  {
    digitalWrite(OUTPUT_PIN, LOW);
  }

  // Print a heartbeat just to make sure we're connected.
  static uint16_t heartBeatTicks;
  if (heartBeatTicks++ == 5000) // Good enough for the Arduino Nano.
  {
    Serial.println("Heartbeat");
    heartBeatTicks = 0;
  }
}

/**
 * Main loop.
 */
void loop()
{
  //uint8_t buf[RH_ASK_MAX_MESSAGE_LEN];
  uint32_t plan;
  uint8_t buflen = sizeof(plan);

  if (s_Driver.recv((uint8_t*)&plan, &buflen))
  {
    //s_Driver.printBuffer("Received: ", buf, buflen);
    //buf[buflen] = '\0';
    //Serial.println((const char*)buf);
    if (plan == MAGIC_VALUE)
    {
      if (s_Linger == 0)
      {
        Serial.println("Output pin high.");
      }
      s_Linger = LINGER_MS; // Reset linger period
      digitalWrite(LED_BUILTIN, HIGH);
    }
  }

  static unsigned long s_Milliseconds;
  unsigned long ms = millis();
  if (ms != s_Milliseconds)
  {
    DoMillisecond();
    s_Milliseconds = ms;
  }
}
