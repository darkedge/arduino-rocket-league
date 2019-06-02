/*
 WiFiEsp example: WebClientRepeating

 This sketch connects to a web server and makes an HTTP request
 using an Arduino ESP8266 module.
 It repeats the HTTP call each 10 seconds.

 For more details see: http://yaab-arduino.blogspot.com/p/wifiesp.html
*/

/**
 * void rfid.begin(
 *    csnPin = 2, 
 *    sckPin = 4,
 *    mosiPin = 5,
 *    misoPin = i + 7,
 *    chipSelectPin = 3,
 *    NRSTPD = 6
 * );
 * 
 * RFID-522
 *   RX SDA SS 8 - Connect to ???
 *         SCK 7 - Connect to 4 (sckPin)
 *        MOSI 6 - Connect to 5 (mosiPin)
 * TX SCL MISO 5 - Connect to i+7 (misoPin)
 *         IRQ 4 - <disconnected>
 *         GND 3 - Connect to ground
 *         RST 2 - Connect to 6 (NRSTPD - Not Reset and Power-down)
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

//#include <WiFiEsp.h>
#include <SPI.h>
#include <rfid1.h>
RFID1 rfid;

// Emulate Serial1 on pins 6/7 if not present
#ifndef HAVE_HWSERIAL1
  #include "SoftwareSerial.h"
  SoftwareSerial Serial1(2, 13); // RX, TX
#endif

#if 0
  char ssid[] = "Ziggo25706";   // SSID
  char pass[] = "tE7fVVp}7cLL"; // Password
  uint16_t status = WL_IDLE_STATUS;     // the Wifi radio's status

  char server[] = "192.168.178.17";
#endif

unsigned long timerBefore = 0;
const uint16_t timer = 1000; //1 second

// Initialize the Ethernet client object
//WiFiEspClient client;

#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

static uint32_t s_DoelLinks[] =
{
  123123123,
  12546235,
  2356326,
  23623723,
  236235
};
static uint32_t s_DoelRechts[] =
{
  643145847,
  123123123,
  12546235,
  2356326,
  23623723,
  236235
};
static uint32_t s_MiddenStip[] =
{
  2200328713,
  12546235,
  2356326,
  23623723,
  236235
};

void setup()
{
  // initialize serial for debugging
  Serial.begin(9600);
  // initialize serial for ESP module
  Serial1.begin(9600);
  // initialize ESP module
  //WiFi.init(&Serial1);

  //SPI.begin();

#if 0
  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD)
  {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }

  // attempt to connect to WiFi network
  while (status != WL_CONNECTED)
  {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }
#endif

  //Serial.println("You're connected to the network");
  Serial.println("Hello World!");
  //  printWifiStatus();

#if 0
  if (client.connect(server, 80))
  {
    Serial.println("connected");
  }
  else
  {
    Serial.println("connection failed");
  }
#endif
}

bool CodeInList(uint32_t* codes, uint16_t aantal_codes, uint32_t code)
{
  for (uint16_t i = 0; i < aantal_codes; i++)
  {
    if (code == codes[i])
    {
      // Dit is een van de codes die we zochten
      return true;
    }
  }

  // Geen matchende code gevonden
  return false;
}


void post(bool links, bool midden, bool rechts)
{
  String detectGoalLinks;
  String detectMiddenStip;
  String detectGoalRechts;

  //en doe er iets mee
  if (links == true)
  {
    detectGoalLinks = "1";
  }
  else
  {
    detectGoalLinks = "0";
  }
  if (midden == true)
  {
    detectMiddenStip = "1";
  }
  else
  {
    detectMiddenStip = "0";
  }
  if (rechts == true)
  {
    detectGoalRechts = "1";
  }
  else
  {
    detectGoalRechts = "0";
  }

  Serial.println("connecting...");
  // client.println("POST /rlrl/radio_Data.php HTTP/1.1");

#if 0
  // bouw de post string
  String PostData = "L=";
  PostData = PostData + detectGoalLinks;
  PostData = PostData + "&M=";
  PostData = PostData + detectMiddenStip;
  PostData = PostData + "&R=";
  PostData = PostData + detectGoalRechts;
  Serial.println(PostData);

  client.println("POST /rlrl/radio_Data.php HTTP/1.1");
  client.println("Host: 192.168.178.17");
  client.println("User-Agent: Arduino/1.0");
  client.println("Connection: Keep-Alive");
  client.println("Content-Type: application/x-www-form-urlencoded;");
  client.println("Content-Length: " + String(PostData.length()));
  client.println();
  client.println(PostData);
#endif

  //  client.print("Content-Length: ");
  // client.println(PostData.length());
  //client.flush();
  // client.stop();



}

void checkRFID(uint16_t i)
{
  // Zet scanner aan
  rfid.begin(2, 4, 5, i + 7, 3, 6); // 7, 8, 9, 10
  rfid.init();

  uchar status;
  uchar str[MAX_LEN];
  // Doe een scan
  status = rfid.request(PICC_REQIDL, str);
  Serial.println(i);
  if (status == MI_OK) // Scan was goed
  {
    // Krijg de ID
    status = rfid.anticoll(str);
    if (status == MI_OK) // ID goed
    {
      uint32_t code;
      memcpy(&code, str, 4); // Kopieer naar iets dat we kunnen vergelijken

      if (CodeInList(s_DoelLinks, COUNT_OF(s_DoelLinks), code))
      {
        Serial.println("Bal zit in doel links");
      }
      else if (CodeInList(s_MiddenStip, COUNT_OF(s_MiddenStip), code))
      {
        Serial.println("Bal zit in doel middenstip");
      }
      else if (CodeInList(s_DoelRechts, COUNT_OF(s_DoelRechts), code))
      {
        Serial.println("Bal zit op Rechts");
      }


      unsigned long timerNow = millis();
      if ((unsigned long)(timerNow - timerBefore) >= timer)
      {
        //post(goalLinks, middenStip, goalRechts);
        timerBefore = millis();

      }


      // Even printen zodat we codes kunnen verzamelen
      Serial.print(i);
      Serial.print(" detects ");
      Serial.println(code);
    }

    // Effe wachten tussen de scans door
    // delay(100);

    // Zet scanner uit
    rfid.halt();
  }
}



void loop()
{
  // Ga door alle scanners heen, een voor een
  for (uint16_t i = 0; i < 4; i++)
  {
    checkRFID(i);
  }
}


void printWifiStatus()
{
  // print the SSID of the network you're attached to
  //Serial.print("SSID: ");
  //Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address
#if 0
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
#endif

  // print the received signal strength
  //long rssi = WiFi.RSSI();
  //Serial.print("Signal strength (RSSI):");
  //Serial.print(rssi);
  //Serial.println(" dBm");
}
