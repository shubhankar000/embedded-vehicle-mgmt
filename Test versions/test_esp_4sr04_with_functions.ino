/*
 * to test if parking and speed will work on 1 device 
 * 
 * testing incrementally
 * 
 * blynk added in last step to see how whole thing will
 * together with blynk
 * 
 * GREAT SUCCESS!!
 * 
 * all that remains is to modify blynk demo PP4 s/d
 * code to integrate this, and then remove the parking
 * code from the park/toll, and add another bridge for 
 * these values. and blynk.notify also.
 * 
 */

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <TimeLib.h>
#include <Wire.h>
#include <RTClib.h>

int echo1 = 18;
int trig1 = 5;
int echo2 = 23;
int trig2 = 19;
int echo3 = 16;
int trig3 = 17;
int echo4 = 32;
int trig4 = 33;
int isrPinT1 = 13;
int isrPinR1 = 12;
int isrPinT2 = 27;
int isrPinR2 = 26;
int n =0;

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

char auth[] = "2a88d1c0091946cea25d4570d5539705";
const char apn[]  = "www";
const char user[] = "";
const char ssid[] = "AndroidAP5";
const char pass[] = "asdasdasd";

bool isTempParked = false;
bool isParked = false;
bool wasParked = false;
bool parkedComplete = false;
int count = 0;
int t = 0;
float d1, t1, t2, d2, s;
volatile bool trig1st, trig2nd, wrongDirection, trigged1st, trigged2nd;
volatile unsigned long millis1, millis2, diffmillis; // millis() to check time of when they got triggered
bool only1output = false;
unsigned long diffSeconds, diffMinutes, diffHours, diffDays;
unsigned long startTime, stopTime, diff;
BlynkTimer timer1;
BlynkTimer timer2;
BlynkTimer timer3;
BlynkTimer timer4;
RTC_DS3231 rtc;
void IRAM_ATTR ISR_trig1(){
  portENTER_CRITICAL_ISR(&mux);
  trig1st = true;
  trigged1st = true;
  millis1 = millis();
  portEXIT_CRITICAL_ISR(&mux);    
}

void IRAM_ATTR ISR_trig2(){
  portENTER_CRITICAL_ISR(&mux);
  trig2nd = true;
  trigged2nd = true;
  millis2 = millis();
  portEXIT_CRITICAL_ISR(&mux);  
}

void setup() {
  // put your setup code here, to run once:
  // begin serial port
  Serial.begin(9600);
  WiFi.disconnect();
  delay(100);
    if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
  }
  // pin setup
  pinMode(echo1, INPUT);
  pinMode(trig1, OUTPUT);
  pinMode(echo2, INPUT);
  pinMode(trig2, OUTPUT);
  pinMode(echo3, INPUT);
  pinMode(trig3, OUTPUT);
  pinMode(echo4, INPUT);
  pinMode(trig4, OUTPUT);
  pinMode(isrPinT1, OUTPUT);
  pinMode(isrPinR1, INPUT);
  pinMode(isrPinT2, OUTPUT);
  pinMode(isrPinR2, INPUT);
      
  // trig pin low
  digitalWrite(trig1, LOW);
  digitalWrite(trig2, LOW);
  digitalWrite(trig3, LOW);
  digitalWrite(trig4, LOW);
  digitalWrite(isrPinT1, LOW);
  digitalWrite(isrPinT2, LOW);
  
  Blynk.begin(auth, ssid, pass);

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
  t1 = 0;
  t2 = 0;
  millis1 = 0;
  millis2 = 0;
  diffmillis = 0;
  trigged1st = false;
  trigged2nd = false;
  
  attachInterrupt(digitalPinToInterrupt(isrPinR1), ISR_trig1, RISING);
  attachInterrupt(digitalPinToInterrupt(isrPinR2), ISR_trig2, RISING);
  
  timer1.setInterval(1000L, checkParked);
  timer2.setInterval(1000L, resetCode);
  timer3.setInterval(10L, speedCode);
  timer4.setInterval(10L, parking);
  Blynk.virtualWrite(V6, "No");
}

void loop() {
  // put your main code here, to run repeatedly:
  Blynk.run();
  timer2.run();
  timer3.run();
  timer4.run();
  
}

void parking(){
  d1 = trigger(trig1, echo1);
  d2 = trigger(trig2, echo2);

  Parking();
  if(parkedComplete){
    DateTime now = rtc.now();
    diff = now.unixtime() - startTime;
    stopTime = now.unixtime();
    timeConversion();
    String difference = (String(diffDays) + "d " + String(diffHours) + "h " + String(diffMinutes) + "m " + String(diffSeconds) + "s");
    Blynk.virtualWrite(V6, difference);
    parkedComplete = false;
    Serial.println("parking over");
  }
}

void speedCode(){
  d1 = trigger(trig3, echo3); 

  if(d1 > 5 && d1 <= 27 && trigged1st == false){
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

  d2 = trigger(trig4, echo4); 

  if(d2 > 5 && d2 <= 27 && trigged2nd == false){
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
      s = (100 / (diffmillis))* 18 / 5;
      Serial.println(s);
    }
    if(millis1 == 0 && millis2 !=0)
      Serial.println("wrong direction");
  }
}

double trigger(int trig, int echo){
  delayMicroseconds(10);
  digitalWrite(trig, HIGH);
  delayMicroseconds(12);
  digitalWrite(trig, LOW);

  t1 = pulseIn(echo, HIGH, 3000L);

  return t1 / 58;
}

void checkParked(){
  DateTime now = rtc.now();
  if(isTempParked){
    t++;
  }
  if(t>=5){
    if(count == 0){
      Serial.println("Parking detected and counting");
      Blynk.virtualWrite(V5, "PARKED!");
      count++;
      startTime = now.unixtime();
    }
    isParked = true;
    wasParked = true;
    }
}

void Parking(){
  if((d1 > 10 && d1 < 30) || d2 > 10 && d2 < 30){
    isTempParked = true;
    if(d2 > 10 && d2 < 30){      
      timer1.run();
    }
  }
  else {
    isTempParked = false;
    isParked = false;
    t = 0;
  }
  if(wasParked == true && isTempParked == false){
    Serial.println("Parking over, calculating fee");
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
