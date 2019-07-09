/* THIS MODULE WILL BE "TEST" PROJECT
 * SIM808 Module
 * Sim808 TX -> ESP32 RX2
 * Sim808 RX -> ESP32 TX2
 *
 * 
 *  ======================================================
 *  Final Year Project on 
 *  "Vehicle Management System & e-Governance on Road"
 *  Date = 26-Jan-2019
 *  Version = 2.0b
 *  Author = Shubhankar Kalele, 130907482 
 *  Guide = Prof. Suhas M. V.
 *  ======================================================
 *  
 *  Updates include:
 *  Bridge between speed/direction esp and this module
 *  Information sent from s/d to this(p/t) module, this module gives output
 *  Toll using distance algorithm
 *  illegal parking detection with point in polygon algorithm
 *  
 */

/*
 * pending:
 * bridge to rx parking status
 * bridge to rx rtc values
 * bridge to rx speed value
 * bridge to different project on viveks phone
 * 
 */

/*
 * Bridges in use:
 * V100 - Rx Speed value
 * 
 */

#define BLYNK_PRINT Serial
#define TINY_GSM_MODEM_SIM808
#define earthRadiusKm 6363

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleSIM800.h>
// #include <BlynkSimpleEsp32.h>
#include <TimeLib.h>
#include <Wire.h>
#include <RTClib.h>
#include <TinyGsmClient.h>

/*
 * REMEMBER TO CHANGE TO GSM MODULE AFTER TESTING
 * ASFASF
 * ASFAFS
 * ASFASF
 */
 
char auth[] = "2a88d1c0091946cea25d4570d5539705";
const char apn[]  = "www";
const char user[] = "";
const char ssid[] = "AndroidAP5";
const char pass[] = "asdasdasd";

// variable declaration
bool checkTolled = false;
float lat1, lon1, lat2, lon2, dLat, dLon, a, c, d, lat, lon;
int q, r, z, count, i, j, counter;


// declratation of geofence
int    polyCorners  =  4; // how many corners the polygon has (no repeats)
float  polyX[]      =  {13.354702, 13.354053, 13.354180, 13.354953}; // horizontal coordinates of corners
float  polyY[]      =  {74.794187, 74.794335, 74.795756, 74.795752}; // vertical coordinates of corners
float x = 13.354537; 
float y = 74.795190; // point to be tested
int led = 2;

BlynkTimer gpsEvent; // code to send GPS data to app
BlynkTimer tollEvent; // continuously calculate distance to toll booth
BlynkTimer geofenceEvent; // check if point inside declared geofence

RTC_DS3231 rtc;

HardwareSerial SerialAT(2);

TinyGsm modem(SerialAT);

// initialize bridge widget
WidgetBridge bridge1(V100);

// connect to device with its auth token
BLYNK_CONNECTED() {
  bridge1.setAuthToken("f8ebf0f6abb049f28c203f68b6e1d88a");
}

BLYNK_WRITE(V98){
  // for parking status
  int h = param.asInt();

  if(h == 1){
    Blynk.notify("Parking detected and counting");
    Blynk.virtualWrite(V6, "PARKED!");
  }
  if(h != 0){
    Blynk.virtualWrite(V6, "Not Parked Yet");
  }
}

BLYNK_WRITE(V97){
  // for rtc values related to parking
  String k = param.asStr();

  Blynk.notify(k);
  Blynk.virtualWrite(V13, k);
}

// LED control
BLYNK_WRITE(V1){
  int p = param.asInt(); // assigning incoming value from pin V1 to a variable

  // process received value
  if(p == 1){
    digitalWrite(led, HIGH);
  }
  if(p == 0){
    digitalWrite(led, LOW);
  }
}

// TO RUN GPSEVENT
BLYNK_WRITE(V7){
  q = param.asInt(); // assigning incoming value from pin V1 to a variable
}

// REPORT VEHICLE AS STOLEN
BLYNK_WRITE(V14){
  r = param.asInt();
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  SerialAT.begin(9600);
  WiFi.disconnect();
  pinMode(led, OUTPUT);
  
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
  }

  modem.enableGPS();

  // Blynk.begin(auth, ssid, pass);
  Blynk.begin(auth, modem, apn, user, pass);
  
  gpsEvent.setInterval(500L, gpsCode);
  tollEvent.setInterval(200L, tollCode);
  geofenceEvent.setInterval(1000L, geofenceCode);
  
  Blynk.virtualWrite(V6, "Not Parked Yet");
  
  // initializing variables
  lat1 = 13.35555164;
  lon1 = 74.79572919;
  lat2 = 13.3553607;
  lon2 = 74.79572919;
  dLon = 0;
  dLat = 0;
  a = 0;
  c = 0;
  d = 0;
  q = 0;
  r = 0;
  z = 0;
  i = 0;
  j = 0;
  count = 0;
  counter = 0;
}

void loop() {
  Blynk.run();
  geofenceEvent.run();
  tollEvent.run();
  if(q == 1){
    gpsEvent.run();
  }
  if(q == 0){
    Blynk.virtualWrite(V0, 1, "", "", "");
  }
  if(q == 1 && r == 1){
    bridge1.virtualWrite(V25, 1);
  }
  if(!(q == 1 && r == 1)){
    bridge1.virtualWrite(V25, 0);
  }
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

  
  Blynk.virtualWrite(V0, 1, lat1, lon1, "Vehicle Position");  
  Blynk.virtualWrite(V8, fix);
}

void tollCode(){
  float dist = distance(lat1, lon1, lat2, lon2);
  
  if(dist < 10 && checkTolled == false){
    Blynk.notify("Crossed Toll, charging");
    Blynk.virtualWrite(V11, "Toll crossed!");
    checkTolled = true;
  }
  
}

float distance(float lat1, float lon1, float lat2, float lon2) {

    dLat = degreesToRadians(lat2-lat1);
    dLon = degreesToRadians(lon2-lon1);

    lat1 = degreesToRadians(lat1);
    lat2 = degreesToRadians(lat2);

    a = sin(dLat/2) * sin(dLat/2) + sin(dLon/2) * sin(dLon/2) * cos(lat1) * cos(lat2); 
    c = 2 * atan2(sqrt(a), sqrt(1-a));
    z++;
    return earthRadiusKm * c * 1000;
  
}

float degreesToRadians(float d) {
  return d * PI / 180;
}

void geofenceCode(){
  i = polyCorners - 1;
  j = polyCorners - 1;
  bool oddNodes = false;

  /*
  // recieve GPS data
  String gps_raw = modem.getGPSraw();
  //parse data
  SerialAT.readStringUntil(','); // mode
  SerialAT.readStringUntil(','); // fix status
  SerialAT.readStringUntil(','); // UTC time 
  x = SerialAT.readStringUntil(',').toFloat(); // lat
  y = SerialAT.readStringUntil(',').toFloat(); // lon
  */
  
  for (i = 0; i < polyCorners; i++) {
    if (polyY[i] < y && polyY[j] >= y
    ||  polyY[j] < y && polyY[i] >= y) {
      if (polyX[i]+(y-polyY[i])/(polyY[j]-polyY[i])*(polyX[j]-polyX[i])<x) {
        oddNodes =! oddNodes; }}
    j = i;
  }

  if(oddNodes == true){
    counter++;
    if(counter == 5){
      Blynk.notify("Parked illegally, please move");
      Blynk.virtualWrite(V2, "Parked illegally, please move");
      bridge1.virtualWrite(V26, 1);
    }
  }
  if(oddNodes == false){
    counter = 0;
    Blynk.virtualWrite(V2, "Not parked illegally");
    bridge1.virtualWrite(V26, 0); 
  }
}
