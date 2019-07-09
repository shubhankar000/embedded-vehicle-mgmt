/* THIS MODULE WILL BE GOVT DASHBOARD
 * RTC module
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
 * isrPinT1 13
 * isrPinR1 12
 * isrPinT2 27
 * isrPinR2 26 
 *  ======================================================
 *  Final Year Project on 
 *  "Vehicle Management System & e-Governance on Road"
 *  Date = 26-Jan-2019
 *  Version = 2.0a
 *  Author = Shubhankar Kalele, 130907482 
 *  Guide = Prof. Suhas M. V.
 *  ======================================================
 *  Update Notes:
 *  This version contains the code to control all 4 SR04s.
 *  Parking, speed and direction will be done on this device
 *  For parking, this code also contains RTC code
 *  All relevant data will be forwarded through blynk
 *  bridge to the CAR module. 
 *    
 */

/*
 * Pending: 
 * add bridge to send parking status
 * add bridge to send rtc values (for amt of time parked)
 * add bridge to send speed value
 * 
 */

/*
 * Bridges in use:
 * V100 - tx speed value
 * V102 - tx rtc values
 * V103 - tx parking status
 * V101 - NOT IN USE HERE
 * 
 */

// define blynk debug output on Serial
#define BLYNK_PRINT Serial
#define led 2

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <TimeLib.h>
#include <Wire.h>
#include <RTClib.h>

// pins for ultrasonic modules
int echo1 = 18;
int trig1 = 5;
int echo2 = 23;
int trig2 = 19;
int echo3 = 16;
int trig3 = 17;
int echo4 = 32; // black wire
int trig4 = 33;
// tx and rx pins for self triggered conditional interrupts
int isrPinT1 = 13;
int isrPinR1 = 12;
int isrPinT2 = 27;
int isrPinR2 = 26;
int n = 0;

char auth[] = "f8ebf0f6abb049f28c203f68b6e1d88a";
const char ssid[] = "AndroidAP5";
const char pass[] = "asdasdasd";

bool isTempParked = false;
bool isParked = false;
bool wasParked = false;
bool parkedComplete = false;
int count = 0;
int t = 0;
float d1, t1, t2, d2, s, lat1, lon1, lat2, lon2, d3, d4;
volatile bool trig1st, trig2nd, wrongDirection, trigged1st, trigged2nd;
volatile unsigned long millis1, millis2, diffmillis; // millis() to check time of when they got triggered
bool only1output = false;
unsigned long diffSeconds, diffMinutes, diffHours, diffDays;
unsigned long startTime, stopTime, diff;

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

BlynkTimer speedEvent; // perform distance measurements
BlynkTimer resetEvent; // variable reset function
BlynkTimer parkingEvent; // check if car parked
BlynkTimer checkEvent;
BlynkTimer rtcEvent; // output current date and time. to ensure rtc is connected and working

WidgetBridge bridge1(V100);

BLYNK_CONNECTED() {
  bridge1.setAuthToken("2a88d1c0091946cea25d4570d5539705");
}

// RECEIVE VEHICLE STOLEN STATUS
BLYNK_WRITE(V25){
  int x = param.asInt();
  bool check = false;
  
  if(x == 1 && check == false){
    Blynk.notify("New stolen vehicle report. Check map for location");
    Blynk.virtualWrite(V1, "New stolen vehicle reported!");
    Blynk.virtualWrite(V2, 0, lat1, lon1, "STOLEN");
    check = true;
  }
  if(x == 0)
    Blynk.virtualWrite(V1, "No reports");
    Blynk.virtualWrite(V2, 0, 0, 0, "");
    check = false;
}

BLYNK_WRITE(V26){
  int x = param.asInt();

  if(x == 1){
    Blynk.notify("New vehicle illegally parked! Check map");
    Blynk.virtualWrite(V1, "New vehicle parked illegally!");
    Blynk.virtualWrite(V2, 1, lat2, lon2, "ILLEGAL PARK");
  }
  if(x == 1){
    Blynk.virtualWrite(V1, "No vehicle parked illegally");
    Blynk.virtualWrite(V2, 1, 0, 0, "");
  }
}

RTC_DS3231 rtc;

// turn on and off led
BLYNK_WRITE(V3){
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
  pinMode(echo3, INPUT);
  pinMode(trig3, OUTPUT);
  pinMode(echo4, INPUT);
  pinMode(trig4, OUTPUT);
  pinMode(led, OUTPUT);
  pinMode(isrPinT1, OUTPUT);
  pinMode(isrPinR1, INPUT);
  pinMode(isrPinT2, OUTPUT);
  pinMode(isrPinR2, INPUT);
  
  // trig and ISR tx pin low
  digitalWrite(trig1, LOW);
  digitalWrite(trig2, LOW);
  digitalWrite(trig3, LOW);
  digitalWrite(trig4, LOW);
  digitalWrite(isrPinT1, LOW);
  digitalWrite(isrPinT2, LOW);

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
  }

  Blynk.begin(auth, ssid, pass);

  // write operation will only take place on variable reset
  speedEvent.setInterval(10L, speedCode); 
  resetEvent.setInterval(1000L, resetCode);
  parkingEvent.setInterval(100L, parkingCode);
  checkEvent.setInterval(1000L, checkParked);
  rtcEvent.setInterval(5000L, rtcWidget);
  
  Blynk.virtualWrite(V6, "Not Parked Yet");
  
  // initializing variables to 0
  startTime = 0;
  stopTime = 0;
  diff = 0;
  diffSeconds = 0;
  diffMinutes = 0;
  diffHours = 0;
  diffDays = 0;
  trig1st = false;
  trig2nd = false;
  wrongDirection = false;
  s = 0;
  d1 = 0;
  d2 = 0;
  d3 = 0;
  d4 = 0;
  t1 = 0;
  t2 = 0;
  millis1 = 0;
  millis2 = 0;
  diffmillis = 0;
  lat1 = 13.355549;
  lon1 = 74.795610;
  lat2 = 13.353724;
  lon2 = 74.792848;
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
  parkingEvent.run();
  rtcEvent.run();
}

void parkingCode(){
  d1 = trigger(trig1, echo1);
  d2 = trigger(trig2, echo2);

  Parking();

  
  if(parkedComplete){
    DateTime now = rtc.now(); 
    diff = now.unixtime() - startTime;
    stopTime = now.unixtime();
    timeConversion();
    String difference = (String(diffDays) + "d " + String(diffHours) + "h " + String(diffMinutes) + "m " + String(diffSeconds) + "s");
    bridge1.virtualWrite(V97, difference);
    bridge1.virtualWrite(V98, 0);
    parkedComplete = false;
    Serial.println("done");
  }
}

void speedCode(){
  d3 = trigger(trig3, echo3); 

  if(d3 > 5 && d3 <= 27 && trigged1st == false){
    portENTER_CRITICAL(&mux);
    digitalWrite(isrPinT1, HIGH);
    portEXIT_CRITICAL(&mux);
  }

  if(trig1st){
    portENTER_CRITICAL(&mux);
    digitalWrite(isrPinT1, LOW);
    trig1st = false;
    portEXIT_CRITICAL(&mux);
  }

  d4 = trigger(trig4, echo4); 

  if(d4 > 4 && d4 <= 27 && trigged2nd == false){
    portENTER_CRITICAL(&mux);
    digitalWrite(isrPinT2, HIGH);
    portEXIT_CRITICAL(&mux);
  }

  if(trig2nd){
    portENTER_CRITICAL(&mux);
    trig2nd = false;
    digitalWrite(isrPinT2, LOW);
    portEXIT_CRITICAL(&mux);
    if(millis1 < millis2 && millis1 !=0){
      diffmillis = millis2 - millis1;
      s = (0.1 / (diffmillis * 0.001) * 18 / 5);
      Blynk.virtualWrite(V4 , s);
      //Serial.println(s);
    }
    if(millis1 == 0 && millis2 !=0){
      Blynk.virtualWrite(V4, "Wrong Direction");
      //Serial.println("wrong dir");
    }
  }
}

double trigger(int trig, int echo){
  delayMicroseconds(10);
  digitalWrite(trig, HIGH);
  delayMicroseconds(12);
  digitalWrite(trig, LOW);

  t1 = pulseIn(echo, HIGH, 2500L);

  return t1 / 58;
}

void checkParked(){
  DateTime now = rtc.now();
  
  if(isTempParked){
    t++;
  }
  if(t>=2){
    if(count == 0){
      bridge1.virtualWrite(V98, 1);
      Serial.println("parked");
      count++;
      startTime = now.unixtime();
    }
    isParked = true;
    wasParked = true;
    }
}

void Parking(){
  if((d1 > 4 && d1 < 30) || d2 > 4 && d2 < 30){
    isTempParked = true;
    if(d2 > 4 && d2 < 30){      
      checkEvent.run();
    }
  }
  else {
    isTempParked = false;
    isParked = false;
    t = 0;
  }
  if(wasParked == true && isTempParked == false){
    reset();
  }
  
}

void reset(){
  wasParked = false;
  isTempParked = false;
  isParked = false;
  t = 0;
  count = 0;
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

void rtcWidget(){
  DateTime now = rtc.now();

  String currentDate =  String(now.day()) + "/" + now.month() + "/" + now.year();
  String currentTime = String(now.hour()) + ":" + now.minute() + ":" + now.second();
  int hours = now.hour();
  if(hours == 165 && n <1){
    Serial.println("RTC DC");
    n++;
  } 
}

void timeConversion(){
  diffSeconds = diff  % 60;
  diffMinutes = diff / 60 % 60;
  diffHours = diff / (60 * 60) % 24;
  diffDays = diff / (24 * 60 * 60);
}
