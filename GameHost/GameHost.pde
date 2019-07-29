import processing.serial.*;
import hypermedia.net.*;
import org.gamecontrolplus.gui.*;
import org.gamecontrolplus.*;
import g4p_controls.*;
import java.nio.*;
import java.io.*;
import java.net.*;
import java.util.*;

private static int panelHeight;
private static GButton buttonStart;

private enum EDeviceType
{
  None,
  PS4,
  Xbox360,
  XboxOne
}

private class RLController
{
  EDeviceType deviceType;
  ControlDevice device;
  TSelectEntry entry;
}

private static final int NUM_CARS = 4;
private boolean started = false;

private static final short port = 0x524C; // ASCII value of "RL" (Rocket League)
private static UDP udpTX;
private static String broadcastAddress;

private static byte[] lastCommand;

// Timing
private static int accumulator = 0;
private static int lastTime = 0;

private Map<String, Integer> carMapping;

private List<RLController> controllers =  new ArrayList<RLController>();

private GLabel lblGoal0;
private GLabel lblGoal1;
private GDropList dropGoal0;
private GDropList dropGoal1;
private GTextArea textArea;

private Serial srGoal0;
private Serial srGoal1;

private boolean goalTriggered;
private GButton btnReset;

public void settings()
{
  size(800, 600);
  pixelDensity(displayDensity());
}

static void GetLocalAddress()
{
  try
  {
    Enumeration<NetworkInterface> nets = NetworkInterface.getNetworkInterfaces();
    for (NetworkInterface netint : Collections.list(nets))
    {
      if (netint.isUp())
      {
        Enumeration<InetAddress> inetAddresses = netint.getInetAddresses();
        for (InetAddress inetAddress : Collections.list(inetAddresses))
        {
          if (inetAddress instanceof Inet4Address && !inetAddress.isLoopbackAddress())
          {
            byte[] address = inetAddress.getAddress();
            byte[] copy = Arrays.copyOf(address, address.length);
            copy[3] = (byte)255; // Change to broadcast
            String str = InetAddress.getByAddress(copy).toString();
            broadcastAddress = str.substring(str.indexOf("/") + 1);
            return;
          }
        }
      }
    }
  }
  catch (Exception e) {}
}

void setup()
{
  GetLocalAddress();
  carMapping = new HashMap<String, Integer>();

  ControlIO control = ControlIO.getInstance(this);
  udpTX = new UDP(this);
  udpTX.broadcast(true);
  udpTX.setReceiveHandler("myCustomReceiveHandler");
  udpTX.listen(true);

  G4P.messagesEnabled(false);
  G4P.setGlobalColorScheme(GCScheme.GREEN_SCHEME);
  if (frame != null)
    surface.setTitle("Rocket League");
  registerMethod("dispose", this);
  createSelectionInterface();
  TConfigUI.pathToSketch = sketchPath("");
  List<ControlDevice> devices = control.getDevices();
  // Add entries for devices added
  for (ControlDevice d : devices)
  {
    if (d != null &&
        !d.getTypeName().equalsIgnoreCase("keyboard") &&
        !d.getTypeName().equalsIgnoreCase("unknown") &&
        !d.getTypeName().equalsIgnoreCase("mouse")
       )
    {

      println(d.toText("    "));

      String s = d.getName();
      String t = d.getTypeName();

      RLController controller = new RLController();
      controller.device = d;

      if ((s.equals("Controller (XBOX 360 For Windows)") ||
           s.equals("XBOX 360 For Windows (Controller)")) &&
          t.equals("Gamepad") &&
          d.getNumberOfButtons() == 11 &&
          d.getNumberOfSliders() == 5)
      {
        controller.deviceType = EDeviceType.Xbox360;
      }
      else if (s.equals("Wireless Controller") &&
               t.equals("Stick") &&
               d.getNumberOfButtons() == 15 &&
               d.getNumberOfSliders() == 6)
      {
        controller.deviceType = EDeviceType.PS4;
      }
      else
      {
        controller.deviceType = EDeviceType.None;
      }

      if (controller.deviceType != EDeviceType.None)
      {
        controller.entry = new TSelectEntry(this, control, d, controller.deviceType.name());
        controllers.add(controller);
      }
    }
  }
  // Reposition entries on screen
  for (int i = 0; i < controllers.size(); i++)
    controllers.get(i).entry.setIndex(panelHeight + 20, i);

  lblGoal0 = new GLabel(this, 400, panelHeight + 20 + 4, 120, 20, "Goal 0");
  lblGoal1 = new GLabel(this, 400, panelHeight + 40 + 4, 120, 20, "Goal 1");

  List<String> list = new ArrayList<String>();
  list.add("N/A");
  list.addAll(Arrays.asList(Serial.list()));
  String[] strings = list.toArray(new String[list.size()]);
  dropGoal0 = new GDropList(this, 500, panelHeight + 20, 120, 100);
  dropGoal0.setItems(strings, 0);
  dropGoal1 = new GDropList(this, 500, panelHeight + 40, 120, 100);
  dropGoal1.setItems(strings, 0);
  handleDropListEvents(dropGoal0, GEvent.CLICKED);
  handleDropListEvents(dropGoal1, GEvent.CLICKED);
  handleButtonEvents(btnReset, GEvent.CLICKED);
  textArea = new GTextArea(this, 4, 140, 800 - 8, 600 - 140 - 4, GTextArea.SCROLLBARS_VERTICAL_ONLY);

  lastTime = millis();
}

private void DoSerial(Serial serial)
{
  int attackers = (serial == srGoal0) ? 1 : 0;
  int defenders = (serial == srGoal0) ? 0 : 1;

  if (serial != null)
  {
    while (serial.available() > 0)
    {
      String myString = serial.readStringUntil(10);
      if (myString != null)
      {
        // Check for goal
        if (myString.startsWith("G"))
        {
          if (!goalTriggered)
          {
            // Print custom message once
            goalTriggered = true;
            textArea.appendText("Team " + attackers + " scored!");

            if (false) // Optional: Get goal
            {
              String numbers = myString.replaceAll("\\D+", "");
              try
              {
                int x = Integer.parseInt(numbers);
              }
              catch (NumberFormatException e) {}
            }
          }
        }
        else
        {
          myString = "Goal " + defenders + ": " + myString;
          textArea.appendText(myString);
        }
      }
    }
  }
}

public void handleDropListEvents(GDropList list, GEvent event)
{
  if (list == dropGoal0)
  {
    if (srGoal0 != null)
    {
      srGoal0.stop();
    }
    if (!list.getSelectedText().equals("N/A"))
    {
      srGoal0 = new Serial(this, list.getSelectedText(), 9600);
    }
  }
  else if (list == dropGoal1)
  {
    if (srGoal1 != null)
    {
      srGoal1.stop();
    }
    if (!list.getSelectedText().equals("N/A"))
    {
      srGoal1 = new Serial(this, list.getSelectedText(), 9600);
    }
  }
}

private void createSelectionInterface()
{
  buttonStart = new GButton(this, width - 60, 20 + 4, 56, 33, "Start game");
  btnReset = new GButton(this, width - 60 - 60, 20 + 4, 56, 33, "Reset goal");

  GLabel lblControls = new GLabel(this, 0, panelHeight, width, 20);
  lblControls.setText("Game Devices Available");
  lblControls.setOpaque(true);
  lblControls.setTextBold();
}

private void addControllers()
{
  ControlIO control = ControlIO.getInstance(this);
  String[] list = {"ps4", "xbox360"};
  for (RLController controller : controllers)
  {
    if (!controller.entry.ipList.getSelectedText().equals("N/A"))
    {
      controller.device.matches(Configuration.makeConfiguration(this, controller.deviceType.name()));
    }
  }
}


public void handleButtonEvents(GButton button, GEvent event)
{
  if (button == buttonStart && event == GEvent.CLICKED)
  {
    if (!started)
    {
      addControllers();
      started = true;
    }
  }
  else if (button == btnReset && event == GEvent.CLICKED)
  {
    goalTriggered = false;
  }
}


void myCustomReceiveHandler(byte[] message, String ip, int port)
{
  try
  {
    short code = ByteBuffer.wrap(message)
                 .order(ByteOrder.LITTLE_ENDIAN)
                 .getShort();
    if (code == port)
    {
      if (!carMapping.containsKey(ip))
      {
        print("Adding car IP to list: ");
        println(ip);
        carMapping.put(ip, -1);

        Set<String> keys = carMapping.keySet();
        List<String> list = new ArrayList<String>();
        list.add("N/A");
        list.addAll(carMapping.keySet());
        String[] strings = new String[list.size()];
        list.toArray(strings);
        for (RLController controller : controllers)
        {
          controller.entry.ipList.setItems(strings, 0);
        }
      }
    }
  }
  catch (Exception e) {}
}


public void draw()
{
  int now = millis();
  accumulator += now - lastTime;
  lastTime = now;

  if (accumulator > 1000) // Send heartbeat
  {
    accumulator %= 1000;

    byte[] packet = ByteBuffer
                    .allocate(2)
                    .order(ByteOrder.LITTLE_ENDIAN)
                    .putShort(port)
                    .array();

    udpTX.send(packet, broadcastAddress, port);
  }

  if (started)
  {
    for (RLController controller : controllers)
    {

      if (!controller.entry.ipList.getSelectedText().equals("N/A"))
      {
        // Steering left/right
        byte horizontal = (byte)0;
        ControlSlider sldHorizontal = controller.device.getSlider("Horizontal");
        if (sldHorizontal != null)
        {
          horizontal = (byte)PApplet.map(sldHorizontal.getValue(), -1.0f, 1.0f, 0.0f, 179.0f);
        }

        // Throttle/reverse
        short forwardBackward = 0;
        ControlSlider sldForward = controller.device.getSlider("Forward");
        ControlSlider sldBackward = controller.device.getSlider("Backward");
        ControlSlider sldForwardBackward = controller.device.getSlider("ForwardBackward");

        // Boost
        byte boost = (byte)0;
        ControlButton btnBoost = controller.device.getButton("Boost");
        if (btnBoost != null)
        {
          boost = btnBoost.pressed() ? (byte)1 : (byte)0;
        }

        switch (controller.deviceType)
        {
        case PS4:
          if (sldForward != null && sldBackward != null)
          {
            float forward = PApplet.map(sldForward.getValue(), -1.0f, 1.0f, 0.0f, 1.0f);
            float backward = PApplet.map(sldBackward.getValue(), -1.0f, 1.0f, 0.0f, -1.0f);
            forwardBackward = (short)((forward + backward) * 1023.0f);
          }
          break;
        case Xbox360:
          if (sldForwardBackward != null)
          {
            forwardBackward = (short)PApplet.map(sldForwardBackward.getValue(), -1.0f, 1.0f, 1023.0f, -1023.0f);
          }
          break;
        case XboxOne:
          if (sldForwardBackward != null)
          {
            forwardBackward = (short)PApplet.map(sldForwardBackward.getValue(), -1.0f, 1.0f, 1023.0f, -1023.0f);
          }
          break;
        default:
          break;
        }

        ByteBuffer bb = ByteBuffer
                        .allocate(4)
                        .order(ByteOrder.LITTLE_ENDIAN)
                        .put(horizontal)
                        .putShort(forwardBackward)
                        .put(boost);

        byte[] newCommand = bb.array();


        if (lastCommand == null || !Arrays.equals(newCommand, lastCommand))
        {
          println(horizontal + " " + forwardBackward + " " + boost);
          udpTX.send(newCommand, controller.entry.ipList.getSelectedText(), port);
          lastCommand = Arrays.copyOf(newCommand, newCommand.length);
        }
      }
    }
  }

  DoSerial(srGoal0);
  DoSerial(srGoal1);

  background(255, 255, 220);
}

public void dispose()
{
  for (RLController controller : controllers)
  {
    if (controller.entry.winCofig != null)
    {
      controller.entry.winCofig.close();
      controller.entry.winCofig = null;
    }
  }
}
