#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Servo.h>

Servo myServo;  // create a servo object

const char* ssid = "Ziggo25706";
const char* password = "tE7fVVp}7cLL";

// Processing IP lijst
//String[] ips = {"192.168.178.102", "192.168.178.103", "192.168.178.104", "192.168.178.105"};
IPAddress ip(192, 168, 178, 103);
IPAddress gateway(192, 168, 178, 1);
IPAddress subnet(255, 255, 255, 0);

const int SERVO_PIN = 12;
const int MOTOR_PWM = 5;
const int MOTOR_D1 = 0;
const int MOTOR_D2 = 1;
//int ledPin = 12;
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
  myServo.attach(SERVO_PIN); // attaches the servo on pin 9 to the servo object
  Serial.begin(115200);
  delay(10);

  //pinMode(ledPin, OUTPUT);
  //digitalWrite(ledPin, LOW);

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

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");

  // Motor
  digitalWrite(0, HIGH);
  digitalWrite(2, HIGH);
  digitalWrite(4, HIGH);
  digitalWrite(6, HIGH);
  digitalWrite(8, HIGH);
  digitalWrite(10, HIGH);
  digitalWrite(12, HIGH);
  digitalWrite(14, HIGH);

  // start UDP
  Udp.begin(localPort);

  Serial.println(sizeof(RLPacket));
}

static uint8_t lastHorizontal;
static int8_t lastForwardBackward;
//static int8_t lastBoost;

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

    if (packet.horizontal != lastHorizontal)
    {
      // set the servo position
      myServo.write(packet.horizontal);
      lastHorizontal = packet.horizontal;
    }

    if (packet.forwardBackward != lastForwardBackward)
    {
      // TODO: this is backwards
      analogWrite(MOTOR_PWM, packet.forwardBackward);
      lastForwardBackward = packet.forwardBackward;
    }
  }
}
