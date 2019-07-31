// Baud rate: 74880

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Servo.h>
#include <EEPROM.h>

struct RLPacket
{
  uint8_t horizontal;
  int16_t forwardBackward;
  uint8_t boost;
} __attribute__((packed));

struct HeartbeatPacket
{
  uint16_t code;
} __attribute__((packed));

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

/**
 * NodeMCU(N) -> Deek-Robot(D) -> RC Car(C)
 * https://www.theengineeringprojects.com/wp-content/uploads/2018/10/Introduction-to-NodeMCU-V3.png
 * http://www.deek-robot.com/edit123/uploadfile/20131128182859312.jpg
 */
// VIN(N)->VCC(D)
// GND(N)->GND(D)
static const int IN2          = D1; // D1(N) -> IN2(D)
static const int BLOCK_SIGNAL = D2; // D2(N) -> Steer Spring(C)
static const int EN1          = D3; // D3(N) -> EN1(D)
static const int LEFT_BLOCK   = D4; // D4(N) + Right Steer Stop -> 10 kOhm -> GND(N)
static const int RIGHT_BLOCK  = D5; // D5(N) + Left Steer Stop -> 10 kOhm -> GND(N)
static const int SERVO_PIN    = D6; // D6(N) -> Servo (DM-S0090D), orange wire
static const int IN1          = D7; // D7(N) -> IN1(D)

static const uint16_t localPort = 0x524C; // ASCII value of "RL" (Rocket League)
static const int s_EepromAddress = 0;

static Servo myServo;
static WiFiUDP Udp;
static EepromData s_EepromData;
static bool s_Connecting;


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


static void WriteEeprom()
{
  EEPROM.put(s_EepromAddress, s_EepromData);
  EEPROM.commit(); // https://github.com/esp8266/Arduino/issues/3275
  PrintEepromData();
  Serial.println("Written EEPROM.");
}


static void ReadEeprom()
{
  s_EepromData = {};
  EEPROM.get(s_EepromAddress, s_EepromData);
  Serial.println("Read from EEPROM:");
  PrintEepromData();
}


static void ConnectDhcp()
{
  s_Connecting = true;
  // Connect to WiFi network
  Serial.print("Connecting to \"");
  Serial.print(s_EepromData.credentials.ssid);
  Serial.println("\" using DHCP...");

  WiFi.config(0, 0, 0);
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
      s_EepromData.ipAddress = 0;
      s_EepromData.subnetMask = 0;
      s_EepromData.gateway = 0;
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
  Serial.begin(74880);
  Serial.println("Hello World!");
  myServo.attach(SERVO_PIN);
  EEPROM.begin(512);
  delay(10);
  ReadEeprom();

  pinMode(IN2, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(EN1, OUTPUT);
  pinMode(BLOCK_SIGNAL, OUTPUT);
  pinMode(LEFT_BLOCK, INPUT);
  pinMode(RIGHT_BLOCK, INPUT);

  digitalWrite(BLOCK_SIGNAL, HIGH);

  pinMode(D0, OUTPUT);
  digitalWrite(D0, HIGH);

  // Reset motor in case we reboot due to a power dip
  digitalWrite(IN2, LOW);
  digitalWrite(IN1, LOW);
  analogWrite(EN1, 0);
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
    Serial.println("\" using static IP...");

    IPAddress ip(s_EepromData.ipAddress);
    IPAddress gateway(s_EepromData.gateway);
    IPAddress subnet(s_EepromData.subnetMask);

    WiFi.config(ip, gateway, subnet);
    WiFi.begin(s_EepromData.credentials.ssid, s_EepromData.credentials.password);
  }

  return WifiStatus;
}


static void ParseRlPacket()
{
  static RLPacket last;
  RLPacket packet;
  Udp.read((char*)&packet, sizeof(packet));

  // Steering left/right
  if (packet.horizontal != last.horizontal)
  {
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
      digitalWrite(IN2, HIGH);
      digitalWrite(IN1, LOW);
      analogWrite(EN1, -packet.forwardBackward);
    }
    else
    {
      digitalWrite(IN2, LOW);
      digitalWrite(IN1, HIGH);
      analogWrite(EN1, packet.forwardBackward);
    }
    last.forwardBackward = packet.forwardBackward;
  }

#if 0 // TODO: What does this code do?
  if (last.forwardBackward == 0)
  {
    digitalWrite(IN2, LOW);
    digitalWrite(IN1, LOW);
    analogWrite(EN1, packet.forwardBackward);
  }
#endif

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
      ParseRlPacket();
    }
    else if (packetSize == sizeof(HeartbeatPacket))
    {
      HeartbeatPacket packet;
      Udp.read((char*)&packet, sizeof(packet));
      if (packet.code == localPort) // OK to reply
      {
        Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
        Udp.write((uint8_t*)&localPort, sizeof(localPort));
        Udp.endPacket();
      }
    }
  }
}
