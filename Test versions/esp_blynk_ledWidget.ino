// led control using blynk app and virtual pins

#define BLYNK_PRINT Serial
#define led 2

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "2a88d1c0091946cea25d4570d5539705";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "AndroidAP3";
char pass[] = "asdasdasd";

void setup()
{
  // Debug console
  Serial.begin(9600);

  // disconnect
  WiFi.disconnect();
  delay(100);

  // initialise led pin
  pinMode(led, OUTPUT);
  
  Blynk.begin(auth, ssid, pass);
  // You can also specify server:
  //Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 80);
  //Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,100), 8080);

}

void loop()
{
  Blynk.run();
}

// V1 LED Widget is blinking
BLYNK_WRITE(V1){
  int p = param.asInt(); // assigning incoming value from pin V1 to a variable

  // Serial.println(p);
  // process received value
  if(p == 1){
    digitalWrite(led, HIGH);
    Serial.println("LED is now on");
  }
  if(p == 0){
    digitalWrite(led, LOW);
    Serial.println("LED is now off");
  }
  
}
