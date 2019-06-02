/*
 WiFiEsp example: WebClientRepeating

 This sketch connects to a web server and makes an HTTP request
 using an Arduino ESP8266 module.
 It repeats the HTTP call each 10 seconds.

 For more details see: http://yaab-arduino.blogspot.com/p/wifiesp.html
*/

/**
 * RFID-522
 *   RX SDA SS 8-
 *         SCK 7
 *        MOSI 6
 * TX SCL MISO 5
 *         IRQ 4
 *         GND 3
 *         RST 2
 *         VCC 1
 */

#include <WiFiEsp.h>
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
  int status = WL_IDLE_STATUS;     // the Wifi radio's status

  char server[] = "192.168.178.17";
#endif

unsigned long timerBefore = 0;
const int timer = 1000; //1 second

// Initialize the Ethernet client object
WiFiEspClient client;

void setup()
{
  // initialize serial for debugging
  Serial.begin(9600);
  // initialize serial for ESP module
  Serial1.begin(9600);
  // initialize ESP module
  WiFi.init(&Serial1);

  //SPI.begin();

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

  Serial.println("You're connected to the network");

  //  printWifiStatus();

  if (client.connect(server, 80))
  {
    Serial.println("connected");
  }
  else
  {
    Serial.println("connection failed");
  }

}

bool IsDoelLinks(uint32_t code)
{
  // Hier kun je je codes invullen die we uitprinten tijdens het scannen
  static uint32_t codes[] =
  {
    123123123,
    12546235,
    2356326,
    23623723,
    236235
  };

  // Bereken het aantal codes in de array
  int aantal_codes = sizeof(codes) / sizeof(uint32_t);
  for (int i = 0; i < aantal_codes; i++)
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

bool IsDoelRechts(uint32_t code)
{
  // Hier kun je je codes invullen die we uitprinten tijdens het scannen
  static uint32_t codes[] =
  {
    643145847,
    123123123,
    12546235,
    2356326,
    23623723,
    236235
  };

  // Bereken het aantal codes in de array
  int aantal_codes = sizeof(codes) / sizeof(uint32_t);
  for (int i = 0; i < aantal_codes; i++)
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

bool IsMiddenStip(uint32_t code)
{
  // Hier kun je je codes invullen die we uitprinten tijdens het scannen
  static uint32_t codes[] =
  {
    2200328713,
    12546235,
    2356326,
    23623723,
    236235
  };

  // Bereken het aantal codes in de array
  int aantal_codes = sizeof(codes) / sizeof(uint32_t);
  for (int i = 0; i < aantal_codes; i++)
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

  //  client.print("Content-Length: ");
  // client.println(PostData.length());
  //client.flush();
  // client.stop();



}

void checkRFID(int i)
{
  // Zet scanner aan
  rfid.begin(2, 4, 5, i + 7, 3, 6);
  rfid.init();

  uchar status;
  uchar str[MAX_LEN];
  // Doe een scan
  status = rfid.request(PICC_REQIDL, str);
  if (status == MI_OK) // Scan was goed
  {
    // Krijg de ID
    status = rfid.anticoll(str);
    if (status == MI_OK) // ID goed
    {
      uint32_t code;
      memcpy(&code, str, 4); // Kopieer naar iets dat we kunnen vergelijken

      bool goalLinks = IsDoelLinks(code);
      bool middenStip = IsMiddenStip(code);
      bool goalRechts = IsDoelRechts(code);

      if (goalLinks)
      {
        Serial.println("Bal zit in doel links");
      }
      if (middenStip)
      {
        Serial.println("Bal zit in doel middenstip");
      }
      if (goalRechts)
      {
        Serial.println("Bal zit op Rechts");
      }


      unsigned long timerNow = millis();
      if ((unsigned long)(timerNow - timerBefore) >= timer)
      {
        post(goalLinks, middenStip, goalRechts);
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
  for (int i = 0; i < 4; i++)
  {
    checkRFID(i);
  }



}


void printWifiStatus()
{
  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
