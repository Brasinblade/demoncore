/*
  Blink

  Turns an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, MEGA and ZERO
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN is set to
  the correct LED pin independent of which board is used.
  If you want to know what pin the on-board LED is connected to on your Arduino
  model, check the Technical Specs of your board at:
  https://www.arduino.cc/en/Main/Products

  modified 8 May 2014
  by Scott Fitzgerald
  modified 2 Sep 2016
  by Arturo Guadalupi
  modified 8 Sep 2016
  by Colby Newman

  This example code is in the public domain.

  https://www.arduino.cc/en/Tutorial/BuiltInExamples/Blink
*/
//#include <avr/sleep.h>
//#include <avr/wdt.h>

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif



//int led = 6; // LED connected to pin 6 of ATtiny85
//int input = 3;
#include <SoftwareSerial.h>
// Definitions
//#define rxPin 3
//#define txPin 6

#include <math.h>
#include <stdlib.h>
SoftwareSerial mySerial(2,1); 

int sensorPin = 0; //ACTUALLY PIN LABELED AS "2" on the HLT tutorial
int sensorVal = 1;

//boolean switchFans = 0;
int analogPin = 0;
int PWM_pin = 7; // LED on PB0 (pin 5) of ATtiny85
int Buzzer_pin = 8;
int val;
int outVal;
int timer = 0;
#define soundPin = 3;
float countVal = 0;
float delayVal = 1;

void setup() {
  mySerial.begin(9600);
   
  pinMode(PWM_pin, OUTPUT);
}


void loop() {

  val = analogRead(analogPin);
  int backupOutVal = outVal;
  outVal = max(min(abs(508-val)*sqrt(abs(508-val)),255),1);///1023;
  //if (outVal >15 && outVal <250) {
  timer = 0;
  analogWrite(PWM_pin, outVal);
  
  if ((countVal < delayVal) && (backupOutVal == outVal))
  {
    int divVal = max(100/outVal,1);
    delay(delayVal/divVal);
    countVal += delayVal/divVal;
  } 
  else
  {
    countVal = 0;
    tone(Buzzer_pin, 1000);
    delay(1);
    noTone(Buzzer_pin);
    delayVal = random(1,4000)/outVal;
    if(mySerial.available()){
      mySerial.print(delayVal);
      mySerial.print(",");
      mySerial.print(val);
      mySerial.print(",");
      mySerial.println(outVal);}
  }
  
  // } else{
  //   if (timer < 30000) {
  //     tone(Buzzer_pin, 1000);
  //     delay(1);
  //     noTone(Buzzer_pin);
  //     float delayVal = random(1,5000)/outVal;
  //     delay(delayVal);
  //     analogWrite(PWM_pin, outVal);
  //     timer = timer + delayVal;
  //   }
  //   else
  //   {
  //     analogWrite(PWM_pin, 0);}
  // }


  
}

