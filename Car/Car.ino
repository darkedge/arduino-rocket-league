// RCRxESP8266
//
// Sample RCRx RCOIP receiver
// Receives RCOIP commmands on an ESP8266 device and uses them to set servo
// and digital outputs.
//
// This simple example handles 5 RCOIP receiver channels. Its configured like this:
// 4 Servos (receiver channels 0, 1, 2, 3)
// 1 Digital output (horn) (receiver channel 4)
// which is consistent with the default setup of the RCTx iPhone app.
//
// However, almost any combination of up to a large number of channels can be used as you see fit.
// Output devices supported are:
// analog output pins
// digital output pins
// Servo
// AccelStepper speed
// AccelStepper position
// HBridge to drive 2 other outputs
//
// Also you can string varies objects together top modify channel values as
// they make their way from the receiver to an output:
// Limiter
// Inverter
// This can be used off the shelf with the RCTx transmitter app for iPhone
// dont forget to set the destination IP address in the RCTx transmitter app profile
//
// Copyright (C) 2018 Mike McCauley


#include <ESP8266Transceiver.h>
#include <RCRx.h>
#include <Servo.h>
#include <ServoSetter.h>
#include <AnalogSetter.h>
#include <DigitalSetter.h>
#include <AccelStepper.h>

// Declare the receiver object. It will handle messages received from the
// transceiver and turn them into channel outputs.
// The receiver and transceiver obects are connected together during setup()
RCRx rcrx;

// Declare the transceiver object, in this case the built-in ESP8266 WiFi
// transceiver.
// Note: other type of transceiver are supported by RCRx
// It will create a WiFi Access Point with SSID of "RCArduino", that you can connect to with the
// password "xyzzyxyzzy"
// on channel 1
// The default IP address of this transceiver is 192.168.4.1/24
// These defaults can be changed by editing ESP8266Transceiver.cpp
// The reported RSSI is in dBm above -60dBm
// If you are using the RCTx transmitter app for iPhone
// dont forget to set the destination IP address in the RCTx transmitter app profile
ESP8266Transceiver transceiver;


// Definitions for the ESP8266 output pins we want to use to control our devices
// The battery voltage is measured on the ESP8266 ADC pin
#define HORN_PIN    12
#define NUM_OUTPUTS 2
#define SERVO_0_PIN 13

// There are 5 outputs for 5 channels:
// 4 Servos (receiver channels 0, 1, 2, 3)
// 1 Digital output (horn) (receiver channel 4)

// These is the low level Servo driver
Servo servo;

// These Setter set the output value onto a Servo
ServoSetter servoSetter0(&servo);

// This setter sets a digital output
DigitalSetter horn(HORN_PIN);

// This array of all the outputs is in channel-number-order so RCRx knows which
// Setter to call for each channel received. We define an array of 5 Setters for receiver channels 0 through 4
Setter*  outputs[NUM_OUTPUTS] = {&servoSetter0, &horn};

void setup()
{
  Serial.begin(9600);
  while (!Serial)
  {

  }

  // Ensure we can output on the horn digital pin
  pinMode(HORN_PIN, OUTPUT);

  // Attach the Servo drivers to the servo output pins
  servo.attach(SERVO_0_PIN);

  // Tell the receiver where to send the 5 channels
  rcrx.setOutputs((Setter**)&outputs, NUM_OUTPUTS);

  // Join the transceiver and the RCRx receiver object together
  rcrx.setTransceiver(&transceiver);

  // Initialise the receiver and transceiver
  rcrx.init();
}

void loop()
{
  // And do it
  rcrx.run();
}
