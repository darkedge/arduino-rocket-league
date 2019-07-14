// Control schemes moeten de naam "ps4" of "xbox360" hebben.

import hypermedia.net.*;

import org.gamecontrolplus.gui.*;
import org.gamecontrolplus.*;
import net.java.games.input.*;


import g4p_controls.G4P;
import g4p_controls.GAlign;
import g4p_controls.GButton;
import g4p_controls.GCScheme;
import g4p_controls.GEvent;
import g4p_controls.GLabel;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import javax.swing.JFileChooser;
import javax.swing.filechooser.FileFilter;

import org.gamecontrolplus.Configuration;
import org.gamecontrolplus.ControlDevice;
import org.gamecontrolplus.ControlIO;
import java.nio.*;
import java.util.Arrays;


GLabel lblPath, lblSketch;
int panelHeight;
GButton btnSelSketch;

private enum EDeviceType
{
  PS4,
  Xbox360,
};
private class RLController
{
  EDeviceType deviceType;
  ControlDevice device;
}
private static final int NUM_CARS = 4;
private RLController[] devices = new RLController[NUM_CARS];
private boolean started = false;

// Ethernet configuratie van de auto (destination)
String[] ips = {"192.168.178.102", "192.168.178.103", "192.168.178.104", "192.168.178.105"};
int port = 19538; // decimale waarde van "RL" (Rocket League) in ASCII

String message = new String("Hello");
UDP udpTX;

byte[] lastCommand;


List<TSelectEntry> deviceEntries =  new ArrayList<TSelectEntry>();

public void settings()
{
  ControlIO control = ControlIO.getInstance(this);
  panelHeight = 40;
  int appHeight = control.getNumberOfDevices() * 20 + 40 + panelHeight;
  size(800, appHeight);
}

void setup()
{
  ControlIO control = ControlIO.getInstance(this);
  udpTX = new UDP(this);

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
      deviceEntries.add(new TSelectEntry(this, control, d));
  }
  // Reposition entries on screen
  for (int i = 0; i < deviceEntries.size(); i++)
    deviceEntries.get(i).setIndex(panelHeight + 20, i);
  //    sel
  System.getProperty("file.separator");
}

private void createSelectionInterface()
{
  btnSelSketch = new GButton(this, width - 60, 4, 56, 33, "Start game");

  GLabel lblControls = new GLabel(this, 0, panelHeight, width, 20);
  lblControls.setText("Game Devices Available");
  lblControls.setOpaque(true);
  lblControls.setTextBold();
}

private void addControllers()
{
  ControlIO control = ControlIO.getInstance(this);
  String[] list = {"ps4", "xbox360"}; // Namen van configuratie bestanden
  int numControllers = 0;
  for (String string : list) // Alle configuraties proberen
  {
    while (true)
    {
      // Find a device that matches the configuration file
      ControlDevice gpad = null;
      try
      {
        gpad = control.getMatchedDeviceSilent(string);
      }
      catch (NullPointerException e)
      {
        break;
      }

      if (gpad != null)
      {
        RLController controller = devices[numControllers++] = new RLController();
        controller.device = gpad;
        if (string.equals("ps4"))
        {
          controller.deviceType = EDeviceType.PS4;
        }
        else if (string.equals("xbox360"))
        {
          controller.deviceType = EDeviceType.Xbox360;
        }
        else
        {
          println("geen case toegevoegd voor deze control scheme");
        }
        println(string + " controller gevonden: " + gpad.getName());
        if (numControllers == NUM_CARS)
        {
          return;
        }
      }
      else
      {
        break;
      }
    }
  }
}

public void handleButtonEvents(GButton button, GEvent event)
{
  if (button == btnSelSketch && event == GEvent.CLICKED)
  {
    if (!started)
    {
      addControllers();
      started = true;
    }
  }
}

public void draw()
{
  if (started)
  {
    for (int i = 0; i < NUM_CARS; i++)
    {
      RLController controller = devices[i];
      if (controller != null)
      {
        byte horizontal = (byte) PApplet.map(controller.device.getSlider("Horizontal").getValue(), -1.0f, 1.0f, 0.0f, 179.0f);
        short forwardBackward = 0;
        byte boost = controller.device.getButton("Boost").pressed() ? (byte)1 : (byte)0;

        switch (controller.deviceType)
        {
        case PS4: // PS4 controller
          float forward = PApplet.map(controller.device.getSlider("Forward").getValue(), -1.0f, 1.0f, 0.0f, 1.0f);
          float backward = PApplet.map(controller.device.getSlider("Backward").getValue(), -1.0f, 1.0f, 0.0f, -1.0f);

          //forwardBackward = (short)forward;
          forwardBackward = (short)((forward + backward) * 1023.0f);
          break;
        case Xbox360: // Xbox 360 controller
          // TODO
          break;
        default:
          println("missing case bij controller switch in draw()");
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
          udpTX.send(newCommand, ips[i], port);
          lastCommand = Arrays.copyOf(newCommand, newCommand.length);
        }
      }
    }
  }


  background(255, 255, 220);

  stroke(230, 230, 200);
  fill(240, 240, 210);
  int y = panelHeight;
  while (y < height)
  {
    rect(0, y, width, 20);
    y += 40;
  }
}

public void dispose()
{
  System.out.println("Disposing");
  for (TSelectEntry entry : deviceEntries)
  {
    if (entry.winCofig != null)
    {
      entry.winCofig.close();
      entry.winCofig = null;
    }
  }
}
