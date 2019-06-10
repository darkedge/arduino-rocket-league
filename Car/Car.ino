#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "Ziggo25706";
const char* password = "tE7fVVp}7cLL";

// Processing IP lijst
//String[] ips = {"192.168.178.102", "192.168.178.103", "192.168.178.104", "192.168.178.105"};
IPAddress ip(192, 168, 178, 103);
IPAddress gateway(192, 168, 178, 1);
IPAddress subnet(255, 255, 255, 0);

int ledPin = 13; // GPIO13
WiFiServer server(80);
unsigned int localPort = 19538; // decimale waarde van "RL" (Rocket League) in ASCII

// buffers for receiving and sending data
#define UDP_TX_PACKET_MAX_SIZE 24
char packetBuffer[UDP_TX_PACKET_MAX_SIZE];  // buffer to hold incoming packet,
char ReplyBuffer[] = "acknowledged";        // a string to send back

// An EthernetUDP instance to let us send and receive packets over UDP
WiFiUDP Udp;

void setup()
{
  Serial.begin(115200);
  delay(10);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

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


  // start UDP
  Udp.begin(localPort);
}

void loop()
{
  // if there's data available, read a packet
  int packetSize = Udp.parsePacket();
  if (packetSize == 3)
  {
    // read the packet into packetBufffer
    Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);

    Serial.print((int)packetBuffer[0]);
    Serial.print(" ");
    Serial.print((int)packetBuffer[1]);
    Serial.print(" ");
    Serial.println((int)packetBuffer[2]);
  }
}
