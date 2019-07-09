/*
 * cheat for output of toll
 */

#define BLYNK_PRINT Serial

#include <BlynkSimpleEsp32.h>

char auth[] = "2a88d1c0091946cea25d4570d5539705";
const char apn[]  = "www";
const char user[] = "";
const char ssid[] = "AndroidAP4";
const char pass[] = "asdasdasd";

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  Blynk.begin(auth, ssid, pass);

  Blynk.notify("You crossed a toll and will be charged!");  
}

void loop() 
  // put your main code here, to run repeatedly:
  Blynk.run();
}
