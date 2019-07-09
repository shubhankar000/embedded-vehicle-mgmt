/* 
 * time diff between 2 triggering interrupts
 * NEW VERSION
 * 
 * USES micors instead of millis for more accurate 
 * reading of speed value
 * 
 */

#include <SimpleTimer.h>

#define echo1 18
#define trig1 5
#define echo2 23
#define trig2 19
#define isrPinT1 13
#define isrPinR1 12
#define isrPinT2 27
#define isrPinR2 26

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

SimpleTimer resetEvent;

float d1, t1, t2, d2, s;
volatile bool trig1st, trig2nd, wrongDirection, trigged1st, trigged2nd;
volatile unsigned long millis1, millis2, diffmillis; // millis() to check time of when they got triggered
unsigned long previousMicros = 0;

void IRAM_ATTR ISR_trig1(){
  portENTER_CRITICAL_ISR(&mux);
  trig1st = true;
  trigged1st = true;
  millis1 = micros();
  portEXIT_CRITICAL_ISR(&mux);    
}

void IRAM_ATTR ISR_trig2(){
  portENTER_CRITICAL_ISR(&mux);
  trig2nd = true;
  trigged2nd = true;
  millis2 = micros();
  portEXIT_CRITICAL_ISR(&mux);  
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  
  pinMode(echo1, INPUT);
  pinMode(trig1, OUTPUT);
  pinMode(echo2, INPUT);
  pinMode(trig2, OUTPUT);
  pinMode(isrPinT1, OUTPUT);
  pinMode(isrPinR1, INPUT);
  pinMode(isrPinT2, OUTPUT);
  pinMode(isrPinR2, INPUT);

  digitalWrite(isrPinT1, LOW);
  digitalWrite(isrPinT2, LOW);
  digitalWrite(trig1, LOW);
  digitalWrite(trig2, LOW);

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

  resetEvent.setInterval(2000, resetCode);
}

void loop() {
  // put your main code here, to run repeatedly:
  previousMicros = millis();
  resetEvent.run();
  trigger1(); 

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

  trigger2(); 

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
      s = (0.1 / (diffmillis * 0.000001))* 18 / 5;
      Serial.println(s);
      Serial.println(millis() - previousMicros);
    }
    if(millis1 == 0 && millis2 !=0)
      Serial.println("wrong direction");
  }
}

void trigger1(){
  delayMicroseconds(10);
  digitalWrite(trig1, HIGH);
  delayMicroseconds(13);
  digitalWrite(trig1, LOW);

  
  t1 = pulseIn(echo1, HIGH, 2500L);
  d1 = t1 / 58;
}

void trigger2(){
  delayMicroseconds(10);
  digitalWrite(trig2, HIGH);
  delayMicroseconds(13);
  digitalWrite(trig2, LOW);

  t2 = pulseIn(echo2, HIGH, 2500L);

  d2 = t2 / 58;
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
