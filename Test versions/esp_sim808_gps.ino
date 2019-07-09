// program to recieve and output raw gps data from gps
// using tinygsm library

#define SerialMon Serial
#define TINY_GSM_MODEM_SIM808

HardwareSerial SerialAT(2); // start UART 2

#include <TinyGsmClient.h>
TinyGsm modem(SerialAT);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(100);
  
  TinyGsmAutoBaud(SerialAT);
  // SerialAT.begin(9600); 
}

void loop() {
  // put your main code here, to run repeatedly:
  modem.enableGPS();
  String gps_raw = modem.getGPSraw();
  modem.disableGPS();
  Serial.println("GPS raw data:" + gps_raw);
  delay(1000);
}
