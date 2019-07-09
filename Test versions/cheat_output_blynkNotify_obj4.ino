/*
 * cheat output obj 4
 */

#define BLYNK_PRINT Serial

#include <BlynkSimpleEsp32.h>

char auth[] = "1fab35203e30472badfcabe5958c361b";
const char apn[]  = "www";
const char user[] = "";
const char ssid[] = "AndroidAP4";
const char pass[] = "asdasdasd";

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  Blynk.begin(auth, ssid, pass);

  Blynk.virtualWrite(V0, "New report!");
  Blynk.virtualWrite(V1, 1, 13.355549, 74.795610, "current location");
  
}

void loop() {
  // put your main code here, to run repeatedly:
  Blynk.run();
}
