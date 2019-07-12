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


ControlIO control;
ControlDevice gpad;
GLabel lblPath, lblSketch;
int panelHeight;
GButton btnSelSketch;

// Ethernet configuratie van de auto (destination)
String ip = "192.168.1.102";
int port = 19538; // decimale waarde van "RL" (Rocket League) in ASCII

String message = new String("Hello");
UDP udpTX;


List<TSelectEntry> deviceEntries =  new ArrayList<TSelectEntry>();

public void settings()
{
  control = ControlIO.getInstance(this);
  panelHeight = 40;
  int appHeight = control.getNumberOfDevices() * 20 + 40 + panelHeight;
  size(800, appHeight);
}

void startGame()
{
  //size(400, 240);
  // Initialise the ControlIO
  control = ControlIO.getInstance(this);
  // Find a device that matches the configuration file
  try
  {
    gpad = control.getMatchedDevice("gamepad_eyes");
  }
  catch (NullPointerException e)
  {
    // Do nothing
  }
  if (gpad == null)
  {
    println("No suitable device configured");
    System.exit(-1); // End the program NOW!
  }
}

void setup()
{
  udpTX = new UDP(this);
  udpTX.log(true);

  G4P.messagesEnabled(false);
  G4P.setGlobalColorScheme(GCScheme.GREEN_SCHEME);
  if (frame != null)
    surface.setTitle("Game Input Device Configurator");
  registerMethod("dispose", this);
  createSelectionInterface();
  TConfigUI.pathToSketch = sketchPath("");
  List<ControlDevice> devices = control.getDevices();
  // Add entries for devices added
  for (ControlDevice d : devices)
  {
    if (d != null && !d.getTypeName().equalsIgnoreCase("keyboard"))
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
  //GLabel lblPathTag = new GLabel(this, 0, 0, 100, 20, "Sketch Path : ");
  //lblPathTag.setOpaque(true);
  //lblPathTag.setTextBold();
  //GLabel lblSketchTag = new GLabel(this, 0, 20, 100, 20, "Sketch Name : ");
  //lblSketchTag.setOpaque(true);
  //lblSketchTag.setTextBold();

  //lblPath = new GLabel(this, 100, 0, width - 100, 20);
  //lblPath.setLocalColorScheme(G4P.PURPLE_SCHEME);
  //lblPath.setOpaque(true);
  //lblPath.setText(sketchPath(""));
  //lblPath.setTextAlign(GAlign.LEFT, GAlign.MIDDLE);

  //lblSketch = new GLabel(this, 100, 20, width - 100, 20);
  //lblSketch.setLocalColorScheme(G4P.PURPLE_SCHEME);
  //lblSketch.setOpaque(true);
  //lblSketch.setText(getClass().getSimpleName() + ".pde");
  //lblSketch.setTextAlign(GAlign.LEFT, GAlign.MIDDLE);

  //btnSelSketch = new GButton(this, width - 60, 4, 56, 33, "Select Sketch");

  GLabel lblControls = new GLabel(this, 0, panelHeight, width, 20);
  lblControls.setText("Game Devices Available");
  lblControls.setOpaque(true);
  lblControls.setTextBold();
}

public void handleButtonEvents(GButton button, GEvent event)
{
  if (button == btnSelSketch && event == GEvent.CLICKED)
  {
    selectSketch();
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

public void selectSketch()
{
  String selected = G4P.selectInput("Select main sketch (pde) file", "pde", "Processing sketch");
  if (selected != null)
  {
    File file = new File(selected);
    // Get the name of the parent folder
    String sketchFolderName = file.getParent();
    // Get the filename without the extension
    String filename = file.getName();
    int index = filename.lastIndexOf('.');
    String sketchName = (index > 0) ? filename.substring(0, index) : filename;
    // See if we have selected the main sketch pde
    if (sketchFolderName.endsWith(sketchName))
    {
      TConfigUI.pathToSketch = sketchFolderName;
      lblPath.setText(sketchFolderName);
      lblSketch.setText(filename);
    }
  }
}
