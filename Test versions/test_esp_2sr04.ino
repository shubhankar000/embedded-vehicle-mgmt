// to test if 2 sr04 ultrasonic distance sensor works with esp32
// using arduino pulseIn() function

#include <SimpleTimer.h>

#define echo1 18
#define trig1 5
#define echo2 23
#define trig2 19

unsigned long t = 0;
double d1, t1, t2, s1, s2, d2;
bool isTempParked = false;
bool isParked = false;
bool wasParked = false;
int count = 0;

SimpleTimer timer;

void setup() {
  // put your setup code here, to run once:
  // begin serial port
  Serial.begin(9600);

  // pin setup
  pinMode(echo1, INPUT);
  pinMode(trig1, OUTPUT);
  pinMode(echo2, INPUT);
  pinMode(trig2, OUTPUT);
  
  // trig pin low
  digitalWrite(trig1, LOW);
  digitalWrite(trig2, LOW);
  d1 = 0;
  d2 = 0;

  timer.setInterval(1000L, checkParked);
}

void loop() {
  // put your main code here, to run repeatedly:
  trigFirst();
  trigSecond();

  Parking();
  
  delay(500);
}

void trigFirst(){
  delayMicroseconds(10); // random delay
  digitalWrite(trig1, HIGH);
  delayMicroseconds(12); // minimum 10us for trigger pulse
  digitalWrite(trig1, LOW);
  
  t1 = pulseIn(echo1, HIGH, 2500UL); // 2500us for approx 30cm timeout (max road clearance of car)

  s1 = t1 * 0.000001; 
  d1 = s1 * 340 * 50;  // distance output in cm

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

void checkParked(){
  if(isTempParked){
    t++;
  }
  if(t>=2){
    if(count == 0){
    Serial.println("Parking detected and counting");
    count++;
    }
    isParked = true;
    wasParked = true;
    }
}

void Parking(){
  if((d1 > 10 && d1 < 30) || d2 > 10 && d2 < 30){
    isTempParked = true;
    if(d2 > 10 && d2 < 30){      
      timer.run();
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
