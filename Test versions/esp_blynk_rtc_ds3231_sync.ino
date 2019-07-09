// sync time given by blynk into rtc. should be 1 time affair
// fail. gets drownout error

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <TimeLib.h>
#include <WidgetRTC.h>
#include "RTClib.h"

char auth[] = "2a88d1c0091946cea25d4570d5539705";
char ssid[] = "AndroidAP3";
char pass[] = "asdasdasd";

BlynkTimer timer;
WidgetRTC rtc;
RTC_DS3231 rtcmod;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  WiFi.disconnect();
  delay(100);

  if (! rtcmod.begin()) {
    Serial.println("Couldn't find RTC module");
  }
  
  Blynk.begin(auth, ssid, pass);
  rtc.begin();
  // DateTime rightnow = rtc.now();
  timer.setInterval(1000L, clockDisplay);
}

void loop() {
  // put your main code here, to run repeatedly:
  Blynk.run();
  timer.run();
}

void clockDisplay(){
  rtcmod.adjust(DateTime(year(), month(), day(), hour(), minute(), second()));
  
  DateTime now = rtcmod.now();
  
  String currentDate =  String(now.day()) + "/" + now.month() + "/" + now.year();
  String currentTime = String(now.hour()) + ":" + now.minute() + ":" + now.second();
  Blynk.virtualWrite(V4, currentTime);
  Blynk.virtualWrite(V3, currentDate);
  Serial.println("Date: " + currentDate);
  Serial.println("Time: " + currentTime);
  Serial.println("UNIX time: " + now.unixtime());
  Serial.println("");
}
