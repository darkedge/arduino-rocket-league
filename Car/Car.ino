// Auto wordt aangestuurd via Wifi.
// Stuur servo zit op 6 van het NodeMCU bordje  (oranje kabel binnen)
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Servo.h>
#include <EEPROM.h>

Servo myServo;  // create a servo object

const char* ssid = "Ziggo25706";
const char* password = "tE7fVVp}7cLL";

// Processing IP lijst
//String[] ips = {"192.168.178.102", "192.168.178.103", "192.168.178.104", "192.168.178.105"};
//IPAddress ip(192, 168, 178, 102); // 192.168.178.102
//IPAddress subnet(255, 255, 255, 0); // 255.255.255.0
//IPAddress gateway(192, 168, 178, 1); // 192.168.178.1

const int SERVO_PIN    = D6;
const int RIGHT_BLOCK  = D5; // Note: metalen plaatje zit links
const int LEFT_BLOCK   = D4; // Note: metalen plaatje zit rechts
const int BLOCK_SIGNAL = D2; // Sluit deze aan op de veer van het stuur

// D2->Veer->Plaatje links ->|->D5
//         ->Plaatje rechts->|->D4
//                           |->10 kOhm weerstand->Ground


// https://hackaday.io/project/8856-incubator-controller/log/29291-node-mcu-motor-shield
const int PWMA = D3;
const int DIRA = D1;
const int DIRB = D7;
unsigned int localPort = 19538; // decimale waarde van "RL" (Rocket League) in ASCII

struct RLPacket
{
  uint8_t horizontal;
  int16_t forwardBackward;
  uint8_t boost;
} __attribute__((packed));

// buffers for receiving and sending data
#define UDP_TX_PACKET_MAX_SIZE 24
char packetBuffer[UDP_TX_PACKET_MAX_SIZE];  // buffer to hold incoming packet,
char ReplyBuffer[] = "acknowledged";        // a string to send back

// An EthernetUDP instance to let us send and receive packets over UDP
WiFiUDP Udp;

static constexpr int s_EepromAddress = 0;

struct EepromData
{
  uint32_t ipAddress;
  uint32_t subnetMask;
  uint32_t gateway;
  char wifiSsid[32];
  char wifiPassword[32];
} __attribute__((packed));

static EepromData s_EepromData;

// https://github.com/esp8266/Arduino/issues/3275
static void WriteEeprom()
{
  EEPROM.put(s_EepromAddress, s_EepromData);
  EEPROM.commit();
  Serial.println("Written EEPROM.");
}

// https://stackoverflow.com/questions/1680365/integer-to-ip-address-c
static void print_ip(uint32_t ip)
{
  uint8_t bytes[4];
  bytes[0] = ip & 0xFF;
  bytes[1] = (ip >> 8) & 0xFF;
  bytes[2] = (ip >> 16) & 0xFF;
  bytes[3] = (ip >> 24) & 0xFF;
  char buf[16] = {};
  sprintf(buf, "%d.%d.%d.%d", bytes[3], bytes[2], bytes[1], bytes[0]);
  Serial.println(buf);
}

static void PrintEepromData()
{
  Serial.print("\tIP Address: ");
  print_ip(s_EepromData.ipAddress);
  Serial.print("\tSubnet Mask: ");
  print_ip(s_EepromData.subnetMask);
  Serial.print("\tGateway: ");
  print_ip(s_EepromData.gateway);
  Serial.print("\tSSID: ");
  Serial.println(s_EepromData.wifiSsid);
  Serial.print("\tPassword: ");
  Serial.println(s_EepromData.wifiPassword);
}

// https://github.com/esp8266/Arduino/issues/3275
static void ReadEeprom()
{
  s_EepromData = {};
  EEPROM.get(s_EepromAddress, s_EepromData);
  Serial.println("Read from EEPROM:");
  PrintEepromData();
}

static RLPacket last;

void ReadSerial()
{
  static byte buf[sizeof(EepromData)];
  static int numRead;

  if (Serial.available() > 0)
  {
    char c = Serial.read();

    buf[numRead++] = c;
    if (numRead == sizeof(buf)) // Done reading
    {
      numRead = 0;
      memcpy(&s_EepromData, buf, sizeof(buf));
      Serial.println("Received config:");
      PrintEepromData();
      memset(buf, 0, sizeof(buf));
      WriteEeprom();
      WiFi.disconnect();
    }
  }
}


void setup()
{
  Serial.begin(115200);
  Serial.println(); // Insert newline after initial garbage(?) output
  myServo.attach(SERVO_PIN);
  EEPROM.begin(512);
  delay(10);
  ReadEeprom();

  pinMode(DIRA, OUTPUT);
  pinMode(DIRB, OUTPUT);
  pinMode(PWMA, OUTPUT);
  pinMode(BLOCK_SIGNAL, OUTPUT);
  pinMode(LEFT_BLOCK, INPUT);
  pinMode(RIGHT_BLOCK, INPUT);

  digitalWrite(BLOCK_SIGNAL, HIGH);

  pinMode(D0, OUTPUT);
  digitalWrite(D0, HIGH);

  Serial.print("Config size: ");
  Serial.println(sizeof(EepromData));
}

void loop()
{
  static bool lastWifiStatus;
  static bool connecting;
  // Check Serial for new configuration data
  ReadSerial();

  bool WifiStatus = (WiFi.status() == WL_CONNECTED);
  if (WifiStatus && !lastWifiStatus)
  {
    Serial.println("WiFi connected");
    connecting = false;
    Udp.begin(localPort);
  }
  else if (lastWifiStatus && !WifiStatus)
  {
    Serial.println("WiFi disconnected");
    WiFi.disconnect();
    Udp.stop();
  }
  lastWifiStatus = WifiStatus;

  digitalWrite(D0, WifiStatus ? LOW : HIGH);

  if (!WifiStatus && !connecting & &s_EepromData.ipAddress != 0 &&
      s_EepromData.gateway != 0 &&
      s_EepromData.subnetMask != 0 &&
      strlen(s_EepromData.wifiSsid) != 0 &&
      strlen(s_EepromData.wifiPassword) != 0)
  {
    connecting = true;
    // Connect to WiFi network
    Serial.print("Connecting to ");
    Serial.println(s_EepromData.wifiSsid);

    IPAddress ip((s_EepromData.ipAddress & 0xff000000) >> 24
                 , (s_EepromData.ipAddress & 0x00ff0000) >> 16
                 , (s_EepromData.ipAddress & 0x0000ff00) >> 8
                 , (s_EepromData.ipAddress & 0x000000ff));
    IPAddress gateway((s_EepromData.gateway & 0xff000000) >> 24
                      , (s_EepromData.gateway & 0x00ff0000) >> 16
                      , (s_EepromData.gateway & 0x0000ff00) >> 8
                      , (s_EepromData.gateway & 0x000000ff));
    IPAddress subnet((s_EepromData.subnetMask & 0xff000000) >> 24
                     , (s_EepromData.subnetMask & 0x00ff0000) >> 16
                     , (s_EepromData.subnetMask & 0x0000ff00) >> 8
                     , (s_EepromData.subnetMask & 0x000000ff));

    WiFi.config(ip, gateway, subnet);
    WiFi.begin(s_EepromData.wifiSsid, s_EepromData.wifiPassword);
  }

  if (WifiStatus)
  {
    // if there's data available, read a packet
    int packetSize = Udp.parsePacket();
    if (packetSize == sizeof(RLPacket))
    {
      // read the packet into packetBuffer
      RLPacket packet;
      Udp.read((char*)&packet, sizeof(packet));

      Serial.print(packet.horizontal);
      Serial.print(" ");
      Serial.print(packet.forwardBackward);
      Serial.print(" ");
      Serial.println(packet.boost);

      // Steering left/right
      if (packet.horizontal != last.horizontal)
      {
        Serial.print(digitalRead(LEFT_BLOCK));
        Serial.print(" ");
        Serial.println(digitalRead(RIGHT_BLOCK));
        // set the servo position
        if (((packet.horizontal < 90) && (digitalRead(LEFT_BLOCK) == LOW)) ||
            ((packet.horizontal > 90) && (digitalRead(RIGHT_BLOCK) == LOW)))
        {
          myServo.write(packet.horizontal);
        }
        last.horizontal = packet.horizontal;
      }

      // Forward/backward
      if (packet.forwardBackward != last.forwardBackward)
      {
        if (packet.forwardBackward < 0)
        {
          digitalWrite(DIRA, HIGH);
          digitalWrite(DIRB, LOW);
          analogWrite(PWMA, -packet.forwardBackward);
        }
        else
        {
          digitalWrite(DIRA, LOW);
          digitalWrite(DIRB, HIGH);
          analogWrite(PWMA, packet.forwardBackward);
        }
        last.forwardBackward = packet.forwardBackward;
      }

      if (last.forwardBackward == 0)
      {
        digitalWrite(DIRA, LOW);
        digitalWrite(DIRB, LOW);
        analogWrite(PWMA, packet.forwardBackward);
      }

      // Reset neutral on boost
      if (packet.boost != last.boost)
      {
        if (packet.boost)
        {
          myServo.writeMicroseconds(1500); // Reset to neutral
        }
        last.boost = packet.boost;
      }
    }
  }
}
