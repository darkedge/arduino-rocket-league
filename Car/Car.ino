// Auto wordt aangestuurd via Wifi.
// Stuur servo zit op 6 van het NodeMCU bordje  (oranje kabel binnen)
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Servo.h>

Servo myServo;  // create a servo object

const char* ssid = "Ziggo25706";
const char* password = "tE7fVVp}7cLL";

// Processing IP lijst
//String[] ips = {"192.168.178.102", "192.168.178.103", "192.168.178.104", "192.168.178.105"};
IPAddress ip(192, 168, 178, 102);
IPAddress gateway(192, 168, 178, 1);
IPAddress subnet(255, 255, 255, 0);

const int SERVO_PIN = 12; // Pin 12 == GPIO 6

// https://hackaday.io/project/8856-incubator-controller/log/29291-node-mcu-motor-shield
const int DIRA = 0;
const int PWMA = 5;
WiFiServer server(80);
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

void setup()
{
  myServo.attach(SERVO_PIN);
  Serial.begin(115200);
  delay(10);

  pinMode(DIRA, OUTPUT);
  pinMode(PWMA, OUTPUT);

  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.config(ip, gateway, subnet);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // start UDP
  Udp.begin(localPort);

  Serial.println(sizeof(RLPacket));
}

static RLPacket last;

void loop()
{
  // if there's data available, read a packet
  int packetSize = Udp.parsePacket();
  if (packetSize == sizeof(RLPacket))
  {
    // read the packet into packetBufffer
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
      // set the servo position
      myServo.write(packet.horizontal);
      last.horizontal = packet.horizontal;
    }

    // Forward/backward
    if (packet.forwardBackward != last.forwardBackward)
    {
      if (packet.forwardBackward < 0)
      {
        digitalWrite(DIRA, HIGH);
        analogWrite(PWMA, -packet.forwardBackward);
      }
      else
      {
        digitalWrite(DIRA, LOW);
        analogWrite(PWMA, packet.forwardBackward);
      }
      last.forwardBackward = packet.forwardBackward;
    }

    if (last.forwardBackward == 0)
    {
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
