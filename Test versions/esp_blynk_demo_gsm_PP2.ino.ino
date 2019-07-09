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
 * 
 *  ======================================================
 *  Final Year Project on 
 *  "Vehicle Management System & e-Governance on Road"
 *  Date = 26-Jan-2019
 *  Version = 1.0.2
 *  Author = Shubhankar Kalele, 130907482 
 *  Guide = Prof. Suhas M. V.
 *  ======================================================
 *  
 *  Presentation for Parking
 */

#define BLYNK_PRINT Serial
#define echo1 18
#define trig1 5
#define echo2 23
#define trig2 19
#define led 2
#define TINY_GSM_MODEM_SIM808

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <TimeLib.h>
#include <Wire.h>
#include "RTClib.h"
#include <TinyGsmClient.h>

char auth[] = "2a88d1c0091946cea25d4570d5539705";
const char apn[]  = "www";
const char user[] = "";
const char ssid[] = "AndroidAP4";
const char pass[] = "asdasdasd";

unsigned long t = 0;
double d1, t1, t2, s1, s2, d2;
bool isTempParked = false;
bool parkedComplete = false;
bool isParked = false;
bool wasParked = false;
int count = 0;
unsigned long previousMicros, startTime, stopTime, diff;
unsigned long diffSeconds, diffMinutes, diffHours, diffDays;
float lat, lon;


BlynkTimer timeEvent; // read rtc
BlynkTimer ultraSR04; // Trigger the SR04 modules
BlynkTimer parkingEvent; // code to check if parked
BlynkTimer gpsEvent; // code to send GPS data to app

RTC_DS3231 rtc;

HardwareSerial SerialAT(2);

TinyGsm modem(SerialAT);

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


BLYNK_WRITE(V7){
  int q = param.asInt(); // assigning incoming value from pin V1 to a variable

  // process received value
  if(q != 0){
    gpsEvent.run();
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  SerialAT.begin(9600);
  WiFi.disconnect();
  
  delay(100);
  
  // pin setup
  pinMode(echo1, INPUT);
  pinMode(trig1, OUTPUT);
  pinMode(echo2, INPUT);
  pinMode(trig2, OUTPUT);
  pinMode(led, OUTPUT);
  
  // trig pin low
  digitalWrite(trig1, LOW);
  digitalWrite(trig2, LOW);

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
  }

  modem.enableGPS();

  Blynk.begin(auth, ssid, pass);
  
  timeEvent.setInterval(2000L, rtcWidget);
  ultraSR04.setInterval(300L, SR04);
  parkingEvent.setInterval(1000L, checkParked);
  gpsEvent.setInterval(500L, gpsCode);
  
  Blynk.virtualWrite(V6, "Not Parked Yet");
  
  // initializing variables to 0
  d1 = 0;
  d2 = 0;
  t1 = 0;
  t2 = 0;
  previousMicros = startTime =  stopTime = diff = 0;
  diffSeconds, diffMinutes, diffHours, diffDays = 0;
}

void loop() {
  // put your main code here, to run repeatedly:
  // previousMicros = millis();
  Blynk.run();
  // distanceEvent.run();
  timeEvent.run();
  ultraSR04.run();
  // previousMicros = millis() - previousMicros;
  // microsEvent.run();
}

void SR04(){
  DateTime now = rtc.now();
  
  trigFirst();
  trigSecond();

  Parking();

  if(parkedComplete){
  diff = now.unixtime() - startTime;
  stopTime = now.unixtime();
  timeConversion();
  String difference = (String(diffDays) + "d " + String(diffHours) + "h " + String(diffMinutes) + "m " + String(diffSeconds) + "s");
  Blynk.notify("You parked for total of " + difference);
  Blynk.virtualWrite(V6, difference);
  parkedComplete = false;
  }
}

void trigFirst(){
  delayMicroseconds(10); // random delay
  digitalWrite(trig1, HIGH);
  delayMicroseconds(12); // minimum 10us for trigger pulse.
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

  s2 = t2 * 0.000001; 
  d2 = s2 * 340 * 50;  // distance output in cm
}

void Parking(){
  if((d1 > 10 && d1 < 30) || (d2 > 10 && d2 < 30)){
    isTempParked = true;
    if((d2 > 10 && d2 < 30) || (d1 > 10 && d1 < 30)){      
      parkingEvent.run();
    }
  }
  else {
    isTempParked = false;
    isParked = false;
    t = 0;
  }
  if(wasParked == true && isTempParked == false){
    // Serial.println("Parking over, calculating fee");
    parkedComplete = true;
    reset();
  }
}

void checkParked(){
  DateTime now = rtc.now();
  
  if(isTempParked){
    t++;
  }
  if(t>=5){
    if(count == 0){
     // Serial.println("Parking detected and counting");
    Blynk.notify("You are now parked!");
    Blynk.virtualWrite(V6, "Now Parked");
    count++;
    startTime = now.unixtime();
    }
    isParked = true;
    wasParked = true;
    }
}

void reset(){
  wasParked = false;
  isTempParked = false;
  isParked = false;
  t = 0;
  count = 0;
}

void rtcWidget(){
  DateTime now = rtc.now();

  String currentDate =  String(now.day()) + "/" + now.month() + "/" + now.year();
  String currentTime = String(now.hour()) + ":" + now.minute() + ":" + now.second();
  int hours = now.hour();
  if(hours != 165){
    Blynk.virtualWrite(V4, currentTime);
    Blynk.virtualWrite(V3, currentDate);
  }
  else{
    Blynk.virtualWrite(V4, "RTC DC");
    Blynk.virtualWrite(V3, "RTC DC");
  }
    
}

void microsTime(){
  Blynk.virtualWrite(V5, previousMicros);
}

void timeConversion(){
  diffSeconds = diff  % 60;
  diffMinutes = diff / 60 % 60;
  diffHours = diff / (60 * 60) % 24;
  diffDays = diff / (24 * 60 * 60);
}

void gpsCode(){
  String fix = "False";

  String gps_raw = modem.getGPSraw();
  //parse data
  SerialAT.readStringUntil(','); // mode
  if(SerialAT.readStringUntil(',').toInt() == 1)
    fix = "True";
  SerialAT.readStringUntil(','); // UTC time 
  lat = SerialAT.readStringUntil(',').toFloat(); // lat
  lon = SerialAT.readStringUntil(',').toFloat(); // lon
  float lat1 = 13.355550;
  float lon1 = 74.795610;
  
  Blynk.virtualWrite(V0, 1, lat1, lon1, "Vehicle Position");  
  Blynk.virtualWrite(V8, fix);
}
