import hypermedia.net.*;

// Ethernet configuratie van de auto
String ip = "192.168.1.102";
int port = 19538; // decimale waarde van "RL" (Rocket League) in ASCII

String message = new String("Hello");
UDP udpTX;

void setup()
{
  udpTX = new UDP(this);
  udpTX.log(true);
  noLoop();
}

void draw()
{
  udpTX.send(message,ip,port);
  delay(1000);
  loop();
}
