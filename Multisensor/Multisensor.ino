/**
 * Arduino MEGA 2560
 * https://www.theengineeringprojects.com/wp-content/uploads/2018/06/introduction-to-arduino-mega-5.png
 * 54 Digital Input / Output Pins: D0 – D53
 * 16 Analog Input / Output Pins: A0 – A15
 * 12 Pulse Width Modulation (PWM) Pins: D2 – D13
 * 4 Serial Communication Ports (8 Pins):
 *   Pin #0 (RX) , Pin #1 (TX)
 *   Pin #19 (RX1) , Pin #18 (TX1)
 *   Pin #17 (RX2) , Pin #16 (TX2)
 *   Pin #15 (RX3) , Pin #14 (TX3)
 * SPI Communication Pins:
 *   Pin #50 (MISO)
 *   Pin #51 (MOSI)
 *   Pin #52 (SCK)
 *   Pin #53 (SS)
 * I2C Communication Pins: Pin #20 (SDA), Pin #21 (SCL)
 * Built-In LED for Testing: Pin #13
 * 
 * void rfid.begin(
 *    csnPin = 2,
 *    sckPin = 4,
 *    mosiPin = 5,
 *    misoPin = i + 7,
 *    chipSelectPin = 3,
 *    NRSTPD = 6
 * );
 *
 * RFID-RC522 - Currently connected via SOFTSPI (rfid1 library)
 * https://lastminuteengineers.com/wp-content/uploads/2018/07/RC522-RFID-Reader-Writer-Module-Pinout.jpg
 *   RX SDA SS 8 - Connect to D3
 *         SCK 7 - Connect to D4
 *        MOSI 6 - Connect to D5
 * TX SCL MISO 5 - Connect to D7-D53
 *         IRQ 4 - <disconnected>
 *         GND 3 - Connect to ground
 *         RST 2 - Connect to D6 (NRSTPD - Not Reset and Power-down)
 *         VCC 1 - Connect to 3.3V
 *
 * VCC supplies power for the module. This can be anywhere from 2.5 to 3.3 volts. You can connect it
 * to 3.3V output from your Arduino. Remember connecting it to 5V pin will likely destroy your module!
 *
 * RST is an input for Reset and power-down. When this pin goes low, hard power-down is enabled.
 * This turns off all internal current sinks including the oscillator and the input pins are
 * disconnected from the outside world. On the rising edge, the module is reset.
 *
 * GND is the Ground Pin and needs to be connected to GND pin on the Arduino.
 *
 * IRQ is an interrupt pin that can alert the microcontroller when RFID tag comes into its vicinity.
 *
 * MISO / SCL / Tx pin acts as Master-In-Slave-Out when SPI interface is enabled, acts as serial
 * clock when I2C interface is enabled and acts as serial data output when UART interface is enabled.
 *
 * MOSI (Master Out Slave In) is SPI input to the RC522 module.
 *
 * SCK (Serial Clock) accepts clock pulses provided by the SPI bus Master i.e. Arduino.
 *
 * SS / SDA / Rx pin acts as Signal input when SPI interface is enabled, acts as serial data when
 * I2C interface is enabled and acts as serial data input when UART interface is enabled. This pin
 * is usually marked by encasing the pin in a square so it can be used as a reference for
 * identifying the other pins.
 */

#include <SPI.h>
#include <rfid1.h>
RFID1 rfid;

static constexpr auto NUM_RFID = 6;

void setup()
{
  // initialize serial for debugging
  Serial.begin(9600);
  Serial.println("Hello World!");
}

void checkRFID(uint16_t i)
{
  // Zet scanner aan
  rfid.begin(2, 4, 5, i + 7, 3, 6); // 7, 8, 9, 10
  rfid.init();

  uchar status;
  uchar str[MAX_LEN];
  // Doe een scan
  // Als een van de sensoren niet goed is aangesloten, krijgen we een timeout, dus doe een tijdmeting
  unsigned long begin = millis();
  status = rfid.request(PICC_REQIDL, str);
  if (millis() - begin > 100)
  {
    Serial.print("Sensor ");
    Serial.print(i);
    Serial.println(" is niet goed aangesloten.");
  }
  if (status == MI_OK) // Scan was goed
  {
    Serial.print(i);
    Serial.print(" goal1!");

    // TODO: ID ophalen, maar dit lukt vaak niet.

    // Zet scanner uit
    rfid.halt();
  }
}

void loop()
{
  for (uint16_t i = 0; i < NUM_RFID; i++)
  {
    // Deze iteraties duren ongeveer ~25 ms elk, soms met spikes van 62-63 ms.
    checkRFID(i);
  }
}
