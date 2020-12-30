/*
  Analog Input
 Demonstrates analog input by reading an analog sensor on analog pin 0 and
 turning on and off a light emitting diode(LED)  connected to digital pin 13. 
 The amount of time the LED will be on and off depends on
 the value obtained by analogRead(). 
 
 The circuit:
 * Potentiometer attached to analog input 0
 * center pin of the potentiometer to the analog pin
 * one side pin (either one) to ground
 * the other side pin to +5V
 * LED anode (long leg) attached to digital output 13
 * LED cathode (short leg) attached to ground
 
 * Note: because most Arduinos have a built-in LED attached 
 to pin 13 on the board, the LED is optional.
 
 
 Created by David Cuartielles
 modified 30 Aug 2011
 By Tom Igoe
 
 This example code is in the public domain.
 
 http://arduino.cc/en/Tutorial/AnalogInput
 
 */

int sensorPin = A0;    // select the input pin for the potentiometer
int sensorPinfiltered = A2;
//int ledPin = 13;      // select the pin for the LED
  // Pin 13: Arduino has an LED connected on pin 13
  // Pin 11: Teensy 2.0 has the LED on pin 11
  // Pin  6: Teensy++ 2.0 has the LED on pin 6
  // Pin 13: Teensy 3.0 has the LED on pin 13
int sensorValue = 0;  // variable to store the value coming from the sensor
int sensorValuefiltered = 0;

void setup() {
  // declare the ledPin as an OUTPUT:
//  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);  
}

void loop() {
  
  
  // read the value from the sensor:
  sensorValue = analogRead(sensorPin);
  float voltage = sensorValue;
  delay(10);
  sensorValuefiltered = analogRead(sensorPinfiltered);
  float voltagefiltered = sensorValuefiltered;
  
  // stop the program for <sensorValue> milliseconds:
  //delay(sensorValue);          
  // turn the ledPin off:        
  // stop the program for for <sensorValue> milliseconds:
  //delay(sensorValue);
  Serial.print(voltage);
  Serial.print(" ");
  Serial.println(voltagefiltered);                 
}