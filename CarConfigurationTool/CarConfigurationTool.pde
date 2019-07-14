import controlP5.*;
import processing.serial.*;

import java.io.IOException;
import java.net.*;
import java.util.Enumeration;

 
Serial myPort;
ControlP5 cp5;
 
String url1, url2;

RadioButton r;
 
void setup() {
  try {
  Enumeration<NetworkInterface> interfaces = NetworkInterface.getNetworkInterfaces();
    while (interfaces.hasMoreElements()) {
        NetworkInterface networkInterface = interfaces.nextElement();
      if (networkInterface.isUp()) {
        System.out.println("Network Interface Name : [" + networkInterface.getDisplayName() + "]");
        System.out.println("Is It connected? : [" + networkInterface.isUp() + "]");
        for (InterfaceAddress i : networkInterface.getInterfaceAddresses()){
          System.out.println("Host Name : "+i.getAddress().getCanonicalHostName());
          System.out.println("Host Address : "+i.getAddress().getHostAddress());
        }
        System.out.println("----------------------");
      }
    }
  } catch (Exception e){}
  
  
        
        
  size(700, 400);
  cp5 = new ControlP5(this);
  r = cp5.addRadioButton("radioButton")
  .setPosition(300,20)
         .setSize(40,20)
         .setColorForeground(color(120))
         .setColorActive(color(255))
         .setColorLabel(color(255))
         .setItemsPerRow(5)
         .setSpacingColumn(50);
  
  String[] names = Serial.list();
  
  for (int i = 0; i < names.length; i++)
  {
    String name = names[i];
    r.addItem(name, i);
  }
  
  final int height = 20;
  final int spacer = 20;
  int y = 20;
  cp5.addTextfield("IpAddress").setPosition(20, y).setSize(200, height).setAutoClear(false); y += height + spacer;
  cp5.addTextfield("Subnet Mask").setPosition(20, y).setSize(200, height).setAutoClear(false); y += height + spacer;
  cp5.addTextfield("Gateway").setPosition(20, y).setSize(200, height).setAutoClear(false); y += height + spacer;
  cp5.addTextfield("SSID").setPosition(20, y).setSize(200, height).setAutoClear(false); y += height + spacer;
  cp5.addTextfield("Password").setPosition(20, y).setSize(200, height).setAutoClear(false); y += height + spacer;
  
  cp5.addBang("Submit").setPosition(20, y).setSize(80, 40);    
}
 
 
void draw () {
  background(0);
}
 
void Submit() {
  print("the following text was submitted :");
  url1 = cp5.get(Textfield.class,"textInput_1").getText();
  url2 = cp5.get(Textfield.class,"textInput_2").getText();
  print(" textInput 1 = " + url1);
  print(" textInput 2 = " + url2);
  println();
}
