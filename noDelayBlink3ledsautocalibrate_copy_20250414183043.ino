
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
int backgroundLight = 3;

//boolean switchFans = 0;
int analogPin = 0;
int PWM_pin = 7; // LED on PB0 (pin 5) of ATtiny85
int PMW_pin2 =8;
int Buzzer_pin = 5;
int val;
int outVal;
int delayVal = 1000;
long int lastTick = 0;
unsigned long currentMillis;
int lastBkgTick = 0;
int backupOutVal = 255;
int center = 520;
int temp_val = 520;
int cnt_val = 0;

void setup() {
  // put your setup code here, to run once:
  mySerial.begin(9600);
   
  pinMode(PWM_pin, OUTPUT);

}

void loop() {
  currentMillis = round(millis());
  long int soundMillis = round(millis());
  // put your main code here, to run repeatedly:
  val = analogRead(analogPin);

  if (480 < val && val < 520 && soundMillis % 30000 < 100 )
  {
    if (temp_val == val){
      cnt_val +=1;
      if (cnt_val == 5){
        center = temp_val;
        cnt_val = 0;
      }
    }
    else {
      cnt_val = 0;
      temp_val = val;
    }
  }
  outVal = max(min(abs(center-val)*sqrt(sqrt(sqrt(abs(center-val)))),255),1);
  long lowVal;
  if (outVal < 10) {

    lowVal=min(((outVal)*1.75),255);
  }
  else {
    lowVal=outVal*3;
  }
    //  lowVal = min(20+outVal*255/100, 255);
     
  analogWrite(PMW_pin2, lowVal);
  long high_val;
  if (outVal >= 150) {
   high_val = max(min(abs(150-outVal)*2.5,255),1);
   
       //high_val = min(max(outVal-30,1)*ratio,255);
    analogWrite(PWM_pin, abs(high_val));
  } else {
    analogWrite(PWM_pin, 0);
    
    //analogWrite(PMW_pin2, outVal);
  }
  if (outVal != backupOutVal)
  {
    backupOutVal = outVal;
    delayVal = round(random(1,4000)/outVal);
  }
  if (soundMillis >= lastTick + delayVal ) {
    tone(Buzzer_pin, 1000);
    delay(1);
    noTone(Buzzer_pin);
    delayVal = round(random(1,4000)/outVal);
    lastTick = soundMillis;
  } 
  if (abs(currentMillis % 15000) >=0  && abs(currentMillis % 15000) <=1500){
    analogWrite(backgroundLight, 255);
  }
  else {
    analogWrite(backgroundLight, 0);
  }
  

  //if (outVal >15 && outVal <250) {
  //analogWrite(PWM_pin, outVal);
    if(mySerial.available()){
      //mySerial.println(soundMillis);
     // mySerial.println(",");
      mySerial.print(val);
      mySerial.print(",");
      mySerial.print(center);
      mySerial.print(",");
      mySerial.print(outVal);
      mySerial.print(",");
      mySerial.print(high_val);
        mySerial.print(",");
      mySerial.println(lowVal);
      }

}
