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

GButton buttonConfirm;

Serial myPort;
GToggleGroup toggleGroup;

GOption options[];

public void setup() {
  size(500, 300);
  G4P.setGlobalColorScheme(GCScheme.PURPLE_SCHEME);
  
  int y = 10;
  labelIpAddress = new GLabel(this, 10, y, 80, 20, "IP Address: ");
  fieldIpAddress = new GTextField(this, 100, y, 200,20);
  y += 30;
  labelSubnetMask = new GLabel(this, 10, y, 80, 20, "Subnet Mask: ");
  fieldSubnetMask = new GTextField(this, 100, y, 200,20);
  y += 30;
  labelGateway = new GLabel(this, 10, y, 80, 20, "Gateway: ");
  fieldGateway = new GTextField(this, 100, y, 200,20);
  y += 30;
  labelSsid = new GLabel(this, 10, y, 80, 20, "SSID: ");
  fieldSsid = new GTextField(this, 100, y, 200,20);
  y += 30;
  labelPassword = new GLabel(this, 10, y, 80, 20, "Password: ");
  fieldPassword = new GTextField(this, 100, y, 200,20);
  y += 30;
  
  buttonConfirm = new GButton(this, 10, y, 70, 20, "Configure");
  
  y = 10;
  toggleGroup = new GToggleGroup();
  String[] names = Serial.list();
  options = new GOption[names.length];
  for (int i = 0; i < names.length; i++)
  {
    String name = names[i];
    options[i] = new GOption(this, 310, y, 100, 20, name);
    toggleGroup.addControl(options[i]);
    y += 30;
  }
}

public void draw() {
  background(200, 128, 200);
}

int ParseIpStringToInt(String string)
{
  int[] ip = new int[4];
String[] parts = string.split("\\.");

  for (int i = 0; i < 4; i++) {
      ip[i] = Integer.parseInt(parts[i]);
  }
  
  int ipNumbers = 0;
  for (int i = 0; i < 4; i++) {
      ipNumbers += ip[i] << (24 - (8 * i));
  }
  
  return ipNumbers;
}

public void handleTextEvents(GEditableTextControl textcontrol, GEvent event) { /* code */ }
public void handleToggleControlEvents(GToggleControl option, GEvent event) { /* code */ }

public void handleButtonEvents(GButton button, GEvent event) {
  if (event == GEvent.CLICKED)
  {
    for (int i = 0; i < options.length; i++)
    {
      if (options[i] != null && options[i].isSelected())
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
      
      Serial serial = new Serial(this, options[i].getText(), 115200);
      serial.write(bb.array());
      delay(100);
      serial.stop();
    }
    }
    }
  }
}
