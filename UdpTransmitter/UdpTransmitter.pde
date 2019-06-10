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


GLabel lblPath, lblSketch;
int panelHeight;
GButton btnSelSketch;

public static final int NUM_CARS = 4;
private ControlDevice[] devices = new ControlDevice[NUM_CARS];

// Ethernet configuratie van de auto (destination)
String ip = "192.168.1.102";
int port = 19538; // decimale waarde van "RL" (Rocket League) in ASCII

String message = new String("Hello");
UDP udpTX;


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
  udpTX = new UDP(this);
  udpTX.log(true);

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
  control = ControlIO.getInstance(this);
  String[] list = {"ps4", "xbox360"}; // Namen van configuratie bestanden
  int numControllers = 0;
  for (String string : list) // Alle configuraties proberen
  {
    while (true)
    {
      // Find a device that matches the configuration file
      try
      {
        ControlDevice gpad = control.getMatchedDeviceSilent(string);
      }
      catch (NullPointerException e)
      {
        break;
      }

      if (gpad != null)
      {
        devices[numControllers++] = gpad;
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
    addControllers();
  }
}

public void draw()
{
  if (false)
  {
    udpTX.send(message, ip, port);

    background(255, 200, 255);

    gpad.getButton("PUPILSIZE1").pressed();
    gpad.getButton("PUPILSIZE2").pressed();
    gpad.getSlider("XPOS").getValue();
    gpad.getSlider("YPOS").getValue();
    gpad.getSlider("EYELID").getValue();

    delay(1000);
    loop();
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
