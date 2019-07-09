/* RTC module
 * SCL -> D22 
 * SDA -> D21
 * VCC -> 3V3
 * 
 * SR04 modules
 * echo1 D18
 * trig1 D5
 * echo2 D23
 * trig2 D19
 * 
 * SIM808 Module
 * Sim808 TX -> ESP32 RX2
 * Sim808 RX -> ESP32 TX2
 *
 * isrPinT1 13
 * isrPinR1 12
 * isrPinT2 27
 * isrPinR2 26 
 *  ======================================================
 *  Final Year Project on 
 *  "Vehicle Management System & e-Governance on Road"
 *  Date = 26-Jan-2019
 *  Version = 1.0.3
 *  Author = Shubhankar Kalele, 130907482 
 *  Guide = Prof. Suhas M. V.
 *  ======================================================
 *  Update Notes:
 *  Integrated Speed detection & 
 *  wrong direction detection (many other things deleted)
 *  
 */

#define BLYNK_PRINT Serial
// pins for ultrasonic modules
#define echo1 16
#define trig1 17
#define echo2 22
#define trig2 21
// pin for led
#define led 2
// tx and rx pins for self triggered conditional interrupts
#define isrPinT1 13
#define isrPinR1 12
#define isrPinT2 27
#define isrPinR2 26

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

char auth[] = "2a88d1c0091946cea25d4570d5539705";
const char apn[]  = "www";
const char user[] = "";
const char ssid[] = "AndroidAP5";
const char pass[] = "asdasdasd";

double d1, t1, t2, s, d2;
volatile bool trig1st, trig2nd, wrongDirection, trigged1st, trigged2nd;
volatile unsigned long millis1, millis2, diffmillis; // millis() to check time of when they got triggered

BlynkTimer speedEvent; // perform distance measurements
BlynkTimer resetEvent; // variable reset function

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

// ISR for trig1
void IRAM_ATTR ISR_trig1(){
  portENTER_CRITICAL_ISR(&mux);
  trig1st = true;
  trigged1st = true;
  millis1 = millis();
  portEXIT_CRITICAL_ISR(&mux);    
}

// ISR for trig2
void IRAM_ATTR ISR_trig2(){
  portENTER_CRITICAL_ISR(&mux);
  trig2nd = true;
  trigged2nd = true;
  millis2 = millis();
  portEXIT_CRITICAL_ISR(&mux);  
}

// turn on and off led
BLYNK_WRITE(V1){
  int p = param.asInt(); // assigning incoming value from pin V1 to a variable

  // Serial.println(p);
  // process received value
  if(p == 1){
    digitalWrite(led, HIGH);
  }
  if(p == 0){
    digitalWrite(led, LOW);
  }
}

void setup() {
  Serial.begin(9600);
  WiFi.disconnect();
  delay(100);
  
  // pin setup
  pinMode(echo1, INPUT);
  pinMode(trig1, OUTPUT);
  pinMode(echo2, INPUT);
  pinMode(trig2, OUTPUT);
  pinMode(led, OUTPUT);
  pinMode(isrPinT1, OUTPUT);
  pinMode(isrPinR1, INPUT);
  pinMode(isrPinT2, OUTPUT);
  pinMode(isrPinR2, INPUT);
  
  // trig and ISR tx pin low
  digitalWrite(trig1, LOW);
  digitalWrite(trig2, LOW);
  digitalWrite(isrPinT1, LOW);
  digitalWrite(isrPinT2, LOW);

  Blynk.begin(auth, ssid, pass);

  // write operation will only take place on variable reset
  speedEvent.setInterval(10L, speedCode); 
  resetEvent.setInterval(1000L, resetCode);
  
  Blynk.virtualWrite(V6, "Not Parked Yet");
  
  // initializing variables to 0
  trig1st = false;
  trig2nd = false;
  wrongDirection = false;
  s = 0;
  d1 = 0;
  d2 = 0;
  t1 = 0;
  t2 = 0;
  millis1 = 0;
  millis2 = 0;
  diffmillis = 0;
  trigged1st = false;
  trigged2nd = false;

  // declare interrupts for rising edge
  attachInterrupt(digitalPinToInterrupt(isrPinR1), ISR_trig1, RISING);
  attachInterrupt(digitalPinToInterrupt(isrPinR2), ISR_trig2, RISING);  
}

void loop() {
  Blynk.run();
  speedEvent.run();
  resetEvent.run();
}

void speedCode(){  
  
  trigFirst();

  // trigger conditional interrupt 1
  if(d1 > 5 && d1 <= 27 && trigged1st == false){
    portENTER_CRITICAL(&mux);
    digitalWrite(isrPinT1, HIGH);
    portEXIT_CRITICAL(&mux);
  }

  // check for interrupt 1 trigger
  if(trig1st){
    portENTER_CRITICAL(&mux);
    digitalWrite(isrPinT1, LOW);
    trig1st = false;
    portEXIT_CRITICAL(&mux);
  }
  
  trigSecond();

  // trigger conditional interrupt 2
  if(d2 > 5 && d2 <= 27 && trigged2nd == false){
    portENTER_CRITICAL(&mux);
    digitalWrite(isrPinT2, HIGH);
    portEXIT_CRITICAL(&mux);
  }

  // check for interrupt 2 trigger
  if(trig2nd){
    portENTER_CRITICAL(&mux);
    trig2nd = false;
    digitalWrite(isrPinT2, LOW);
    portEXIT_CRITICAL(&mux);
    /* checks whether ISR2 triggered after ISR1 and calculates
     * speed based time difference recorded and known module
     * seperation distance
     */
    if(millis1 < millis2 && millis1 !=0){
      diffmillis = millis2 - millis1;
      s = (0.1 / (diffmillis * 0.001))* 18 / 5;
      // Blynk.virtualWrite(V9, s);
      Serial.println(s);
    }
    /* if ISR2 triggered before ISR1, it will be known 
     * that vehicle is moving in wrong direction
     */
    if(millis1 == 0 && millis2 !=0){
      // Blynk.virtualWrite(V9, "wrong direction");
      Serial.println("wrong direction");
    }
  }
}

void trigFirst(){
  delayMicroseconds(10); // random delay
  digitalWrite(trig1, HIGH);
  delayMicroseconds(12); // minimum 10us for trigger pulse
  digitalWrite(trig1, LOW);
  
  t1 = pulseIn(echo1, HIGH, 2500UL); // 2500us for approx 30cm timeout (max road clearance of car)
  d1 = t1 / 58;  // distance output in cm
}

void trigSecond(){
  delayMicroseconds(10); // random delay
  digitalWrite(trig2, HIGH);
  delayMicroseconds(12); // minimum 10us for trigger pulse
  digitalWrite(trig2, LOW);
  
  t2 = pulseIn(echo2, HIGH, 2500UL); // 1000us for approx 30cm timeout (max road clearance of car) 
  d2 = t2 / 58;  // distance output in cm
}

void resetCode(){
  portENTER_CRITICAL(&mux);
  trigged1st = false;
  trigged2nd = false;
  portEXIT_CRITICAL(&mux);
  millis1 = 0;
  millis2 = 0;
  diffmillis = 0;  
}
