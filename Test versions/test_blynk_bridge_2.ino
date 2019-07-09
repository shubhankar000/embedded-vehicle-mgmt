/*
 * Blynk bridge test
 * device 2 (PARKING/TOLL DEVICE)
 * 
 * GREAT SUCCESS!
 */

#define BLYNK_PRINT Serial

#include <BlynkSimpleEsp32.h>

WidgetBridge bridge1(V100);

char auth[] = "2a88d1c0091946cea25d4570d5539705";
const char apn[]  = "www";
const char user[] = "";
const char ssid[] = "AndroidAP4";
const char pass[] = "asdasdasd";
int p = 0;

BLYNK_CONNECTED() {
  bridge1.setAuthToken("808f4fb58c10445c8517439cb5fcd4b7"); // Place the AuthToken of the second hardware here
}

BLYNK_WRITE(V99){
  p = param.asInt(); 
  Blynk.virtualWrite(V13, p);  
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
  
  Blynk.begin(auth, ssid, pass);
}

void loop() {
  // put your main code here, to run repeatedly:
  Blynk.run();
}
