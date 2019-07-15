import processing.serial.*;
import g4p_controls.*;
import java.nio.*;
import java.nio.charset.StandardCharsets;

GLabel labelIpAddress;
GLabel labelSubnetMask;
GLabel labelGateway;
GLabel labelSsid;
GLabel labelPassword;

GTextField fieldIpAddress;
GTextField fieldSubnetMask;
GTextField fieldGateway;
GTextField fieldSsid;
GTextField fieldPassword;

GTextArea textArea;

GButton buttonConfirm;

Serial myPort;
GDropList dropList;

Serial serial;

public void setup()
{
  size(720, 480);
  G4P.setGlobalColorScheme(GCScheme.PURPLE_SCHEME);

  int y = 10;
  labelIpAddress = new GLabel(this, 10, y, 80, 20, "IP Address: ");
  fieldIpAddress = new GTextField(this, 100, y, 200, 20);
  y += 30;
  labelSubnetMask = new GLabel(this, 10, y, 80, 20, "Subnet Mask: ");
  fieldSubnetMask = new GTextField(this, 100, y, 200, 20);
  y += 30;
  labelGateway = new GLabel(this, 10, y, 80, 20, "Gateway: ");
  fieldGateway = new GTextField(this, 100, y, 200, 20);
  y += 30;
  labelSsid = new GLabel(this, 10, y, 80, 20, "SSID: ");
  fieldSsid = new GTextField(this, 100, y, 200, 20);
  y += 30;
  labelPassword = new GLabel(this, 10, y, 80, 20, "Password: ");
  fieldPassword = new GTextField(this, 100, y, 200, 20);
  y += 30;

  buttonConfirm = new GButton(this, 10, y, 70, 20, "Configure");

  textArea = new GTextArea(this, 400, 10, 300, 200);
  textArea.setTextEditEnabled(false);

  y = 10;
  String[] names = Serial.list();
  if (names.length == 0)
  {
    names = new String[] {"N/A"};
  }
  dropList = new GDropList(this, 310, y, 70, 100);
  dropList.setItems(names, 0);
  handleDropListEvents(dropList, GEvent.CLICKED);
}

public void draw()
{
  background(200, 128, 200);

  if (serial != null)
  {
    while (serial.available() > 0)
    {
      String myString = serial.readStringUntil(10);
      if (myString != null)
      {
        textArea.appendText(myString);
      }
    }
  }
}

int ParseIpStringToInt(String string)
{
  int[] ip = new int[4];
  String[] parts = string.split("\\.");

  for (int i = 0; i < 4; i++)
  {
    ip[i] = Integer.parseInt(parts[i]);
  }

  int ipNumbers = 0;
  for (int i = 0; i < 4; i++)
  {
    ipNumbers += ip[i] << (24 - (8 * i));
  }

  return ipNumbers;
}

public void handleTextEvents(GEditableTextControl textcontrol, GEvent event) { /* code */ }

public void handleDropListEvents(GDropList list, GEvent event)
{
  if (serial != null)
  {
    serial.stop();
  }
  serial = new Serial(this, list.getSelectedText(), 115200);
}

public void handleButtonEvents(GButton button, GEvent event)
{
  if (event == GEvent.CLICKED)
  {
    if (dropList.getSelectedText().equals("N/A"))
    {
      textArea.appendText("No serial device available.");
    }
    else
    {
      int ip = ParseIpStringToInt(fieldIpAddress.getText());
      int subnet = ParseIpStringToInt(fieldSubnetMask.getText());
      int gateway = ParseIpStringToInt(fieldGateway.getText());
      byte[] ssid = fieldSsid.getText().getBytes(StandardCharsets.US_ASCII);
      byte[] pw = fieldPassword.getText().getBytes(StandardCharsets.US_ASCII);
      if (ssid.length <= 32 && pw.length <= 32)
      {
        byte[] pad0 = new byte[32 - ssid.length];
        byte[] pad1 = new byte[32 - pw.length];

        ByteBuffer bb = ByteBuffer
                        .allocate(76)
                        .order(ByteOrder.LITTLE_ENDIAN)
                        .putInt(ip)
                        .putInt(subnet)
                        .putInt(gateway)
                        .put(ssid)
                        .put(pad0)
                        .put(pw)
                        .put(pad1);

        if (serial != null)
        {
          serial.write(bb.array());
        }
      }
      else
      {
        textArea.appendText("SSID or password too long (32 bytes max).");
      }
    }
  }
}
