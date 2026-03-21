#include <math.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

// --- FINAL PRODUCTION SETTING ---
#define DEBUG 0

#if DEBUG
  #include <SoftwareSerial.h>
  SoftwareSerial mySerial(-1, 1); 
#endif

// Pins
const int analogPin = 0;   
const int LED_1W = 8;      
const int LED_3W = 7;      
const int Buzzer_pin = 5;  

// --- PERSISTENT MEMORY (Survives Flickers) ---
uint16_t magic_word __attribute__ ((section (".noinit")));
int persistent_center __attribute__ ((section (".noinit")));

// Dither Logic Variables
volatile int ditherStep = 0;
volatile int ditherThreshold = 1; 
volatile bool ditherEnabled = true; 

// Feature Variables
int val;
long internalScale; 
int center = 517;       
int sleepSnapshot = 517;
int lastVal = 517;         
unsigned long initial_timer = 0;
unsigned long lastTick = 0;
unsigned long lastPrint = 0;
long delayVal = 1000;

// --- STABILITY TRACKING GLOBALS ---
unsigned long stabilityTimer = 0;
int lastStabilityVal = 517;

// --- HIGH-SPEED GHOST ISR ---
ISR(TIMER0_COMPA_vect) {
  if (ditherEnabled) {
    ditherStep++;
    if (ditherStep >= 10) ditherStep = 0; 
    if (ditherStep < ditherThreshold) analogWrite(LED_1W, 1); 
    else analogWrite(LED_1W, 0); 
  }
}

ISR(WDT_vect) {} 

void setupWatchdog() {
  cli(); wdt_reset();
  WDTCSR |= (1<<WDCE) | (1<<WDE);
  WDTCSR = (1<<WDIE) | (1<<WDP2) | (1<<WDP1); 
  sei();
}

void enterDeepSleep() {
  #if DEBUG
    mySerial.println(F(">>> SLEEP BYPASS <<<"));
    initial_timer = millis(); return; 
  #endif
  
  ditherEnabled = false;
  analogWrite(LED_1W, 0); 
  analogWrite(LED_3W, 0);
  digitalWrite(Buzzer_pin, LOW);
  
  sleepSnapshot = analogRead(analogPin);
  while (true) {
    setupWatchdog();
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable(); sleep_cpu(); sleep_disable(); wdt_disable();
    delay(20); 
    if (abs(analogRead(analogPin) - sleepSnapshot) > 25) { 
      initial_timer = millis(); 
      return; 
    }
  }
}

void setup() {
  byte resetSource = MCUSR;
  MCUSR = 0; 

  OSCCAL = 0x72; 
  pinMode(LED_1W, OUTPUT); 
  pinMode(LED_3W, OUTPUT); 
  pinMode(Buzzer_pin, OUTPUT);
  
  #if DEBUG
    mySerial.begin(2400);
  #endif

  // --- LAZY STARTUP LOGIC ---
  if (magic_word == 0xACE2 && !(resetSource & (1 << PORF))) {
    center = persistent_center;
  } 
  else {
    center = 517; 
    magic_word = 0xACE2;
    persistent_center = center;
  }
  
  lastVal = center;
  sleepSnapshot = center;
  initial_timer = millis();

  cli(); 
  OCR0A = 128;
  TIMSK0 |= (1 << OCIE0A); 
  sei();
}

void loop() {
  unsigned long currentMillis = millis();
  val = analogRead(analogPin);

  int diff = abs(center - val);
  
  // --- 1. CORE INTENSITY ---
  if (diff <= 5) {
    internalScale = 0;
  } else {
    float adj = (float)(diff - 5);
    internalScale = (long)(pow(adj, 0.75) * 19.5); 
    internalScale = constrain(internalScale, 0, 510);
  }

  // --- 2. UNRESTRICTED AUTO-HEAL ---
  // Removed intensity lockout. Now only checks for Range (450-580) and Stillness.
  if (val > 450 && val < 580) { 
    if (abs(val - lastStabilityVal) < 2) { 
      if (currentMillis - stabilityTimer > 4500) {
        center = val;            
        persistent_center = center; 
        initial_timer = currentMillis; 
      }
    } else {
      lastStabilityVal = val;
      stabilityTimer = currentMillis;
    }
  } else {
    stabilityTimer = currentMillis;
  }

  // --- 3. MOVEMENT & SLEEP LOGIC ---
  if (abs(val - lastVal) > 12) {
    initial_timer = currentMillis;
    if (internalScale > 50 && delayVal > 100) delayVal = 50; 
    lastVal = val; 
  }

  unsigned long limit = (internalScale > 200) ? 5000 : 25000;
  if (currentMillis - initial_timer > limit) enterDeepSleep();

  // --- 4. LIGHT LOGIC (DITHERING ENGINE) ---
  if (diff < 12) {
    ditherEnabled = true; 
    ditherThreshold = map(diff, 0, 12, 1, 10); 
    analogWrite(LED_3W, 0);
  } 
  else {
    ditherEnabled = false; 
    long x = (long)(diff - 12);
    long curve = (x * x) / 80; 
    
    int finalRamp = constrain(curve + 1, 1, 510);
    
    if (finalRamp < 255) {
      analogWrite(LED_1W, (int)finalRamp);
      analogWrite(LED_3W, 0);
    } else {
      analogWrite(LED_1W, 255);
      analogWrite(LED_3W, (int)(finalRamp - 255));
    }
  }

  // --- 5. AUDIO LOGIC ---
  if (currentMillis >= lastTick + delayVal) {
    for(int i = 0; i < 3; i++) {
      digitalWrite(Buzzer_pin, HIGH); delayMicroseconds(110); 
      digitalWrite(Buzzer_pin, LOW);  delayMicroseconds(110);
    }
    long audioScale = internalScale / 2;
    long baseDelay = 4000 / (audioScale + 1); 
    
    delayVal = (baseDelay / 2) + random(5, (long)(baseDelay * 1.5) + 5);
    lastTick = currentMillis;
  }

  #if DEBUG
    if (currentMillis - lastPrint > 800) {
      mySerial.print(F("H:")); mySerial.print(val);
      mySerial.print(F(" | C:")); mySerial.print(center);
      mySerial.print(F(" | O:")); mySerial.print(internalScale);
      mySerial.println();
      lastPrint = currentMillis;
    }
  #endif
}