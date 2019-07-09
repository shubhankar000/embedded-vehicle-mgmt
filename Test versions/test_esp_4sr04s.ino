/*
 * testing 4 sr04 output working or not
 * 
 */

#include <SimpleTimer.h>

#define echo1 18
#define trig1 5
#define echo2 23
#define trig2 19
#define echo3 16
#define trig3 17
#define echo4 22
#define trig4 21

double t;

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
  pinMode(echo3, INPUT);
  pinMode(trig3, OUTPUT);
  pinMode(echo4, INPUT);
  pinMode(trig4, OUTPUT);
    
  // trig pin low
  digitalWrite(trig1, LOW);
  digitalWrite(trig2, LOW);
  digitalWrite(trig3, LOW);
  digitalWrite(trig4, LOW);
  
  t = 0;

  timer.setInterval(100, timerCode);
}

void loop() {
  // put your main code here, to run repeatedly:
  timer.run();
 
}

void timerCode(){
  Serial.print(trigger(trig1, echo1));
  Serial.print(" , ");
  Serial.print(trigger(trig2, echo2));
  Serial.print(" , ");
  Serial.print(trigger(trig3, echo3));
  Serial.print(" , ");
  Serial.print(trigger(trig4, echo4));
  Serial.println("");
  
}

double trigger(int trig, int echo){
  delayMicroseconds(10);
  digitalWrite(trig, HIGH);
  delayMicroseconds(12);
  digitalWrite(trig, LOW);

  t = pulseIn(echo, HIGH, 6000L);

  return t / 58;
}
