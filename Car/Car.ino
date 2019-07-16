// Auto wordt aangestuurd via Wifi.
// Stuur servo zit op 6 van het NodeMCU bordje  (oranje kabel binnen)
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Servo.h>
#include <EEPROM.h>

Servo myServo;  // create a servo object

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

struct WifiCredentials
{
  char ssid[32];
  char password[32];
} __attribute__((packed));

struct EepromData
{
  uint32_t ipAddress;
  uint32_t subnetMask;
  uint32_t gateway;
  WifiCredentials credentials;
} __attribute__((packed));

static EepromData s_EepromData;

static void PrintCredentials()
{
  Serial.print("\tSSID: ");
  Serial.println(s_EepromData.credentials.ssid);
  Serial.print("\tPassword: ");
  Serial.println(s_EepromData.credentials.password);
}

static void PrintEepromData()
{
  Serial.print("\tIP Address: ");
  Serial.println(IPAddress(s_EepromData.ipAddress).toString());
  Serial.print("\tSubnet Mask: ");
  Serial.println(IPAddress(s_EepromData.subnetMask).toString());
  Serial.print("\tGateway: ");
  Serial.println(IPAddress(s_EepromData.gateway).toString());
  PrintCredentials();
}

// https://github.com/esp8266/Arduino/issues/3275
static void WriteEeprom()
{
  EEPROM.put(s_EepromAddress, s_EepromData);
  EEPROM.commit();
  PrintEepromData();
  Serial.println("Written EEPROM.");
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
static bool s_Connecting;


static void ConnectDhcp()
{
  s_Connecting = true;
  // Connect to WiFi network
  Serial.print("Connecting to \"");
  Serial.print(s_EepromData.credentials.ssid);
  Serial.println("\" using DHCP...");

  WiFi.begin(s_EepromData.credentials.ssid, s_EepromData.credentials.password);
}


/**
 * Check the serial communication for new configuration data
 */
void ReadSerial()
{
  static byte buf[sizeof(WifiCredentials)];
  static int numRead;

  if (Serial.available() > 0)
  {
    char c = Serial.read();

    buf[numRead++] = c;
    if (numRead == sizeof(buf)) // Done reading
    {
      numRead = 0;
      memcpy(&s_EepromData.credentials, buf, sizeof(buf));
      Serial.println("Received config:");
      PrintCredentials();
      memset(buf, 0, sizeof(buf));
      WriteEeprom();
      s_Connecting = false;
      WiFi.disconnect();

      ConnectDhcp();
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
}

static bool CheckWifiStatus()
{
  static bool lastWifiStatus;
  bool WifiStatus = (WiFi.status() == WL_CONNECTED);
  if (WifiStatus && !lastWifiStatus)
  {
    Serial.println("WiFi connected");
    s_EepromData.ipAddress = WiFi.localIP();
    s_EepromData.subnetMask = WiFi.subnetMask();
    s_EepromData.gateway = WiFi.gatewayIP();
    WriteEeprom();
    s_Connecting = false;
    Udp.begin(localPort);
  }
  else if (lastWifiStatus && !WifiStatus)
  {
    Serial.println("WiFi disconnected");
    Udp.stop();
  }
  lastWifiStatus = WifiStatus;

  digitalWrite(D0, WifiStatus ? LOW : HIGH);

  if (!WifiStatus && !s_Connecting & &s_EepromData.ipAddress != 0 &&
      s_EepromData.gateway != 0 &&
      s_EepromData.subnetMask != 0 &&
      strlen(s_EepromData.credentials.ssid) != 0 &&
      strlen(s_EepromData.credentials.password) != 0)
  {
    s_Connecting = true;
    // Connect to WiFi network
    Serial.print("Connecting to \"");
    Serial.print(s_EepromData.credentials.ssid);
    Serial.print("\" using static IP...");

    IPAddress ip(s_EepromData.ipAddress);// = CreateIpAddressFromUint32(s_EepromData.ipAddress);
    IPAddress gateway(s_EepromData.gateway);// = CreateIpAddressFromUint32(s_EepromData.gateway);
    IPAddress subnet(s_EepromData.subnetMask);// = CreateIpAddressFromUint32(s_EepromData.subnetMask);

    WiFi.config(ip, gateway, subnet);
    WiFi.begin(s_EepromData.credentials.ssid, s_EepromData.credentials.password);
  }

  return WifiStatus;
}

void loop()
{
  // Check Serial for new configuration data
  ReadSerial();

  if (CheckWifiStatus())
  {
    // if there's data available, read a packet
    int packetSize = Udp.parsePacket();
    if (packetSize == sizeof(RLPacket))
    {
      // read the packet into packetBuffer
      RLPacket packet;
      Udp.read((char*)&packet, sizeof(packet));

      //Serial.print(packet.horizontal);
      //Serial.print(" ");
      //Serial.print(packet.forwardBackward);
      //Serial.print(" ");
      //Serial.println(packet.boost);

      // Steering left/right
      if (packet.horizontal != last.horizontal)
      {
        //Serial.print(digitalRead(LEFT_BLOCK));
        //Serial.print(" ");
        //Serial.println(digitalRead(RIGHT_BLOCK));
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
