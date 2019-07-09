/* testing response of code to 2 interrupts by 2 sr04
 * will display values only if condition is met
 * will also include code to detect wrong direction
 * of travel based on which sr04 gets triggered first
 * 
 * great success!
 */

#define echo1 18
#define trig1 5
#define echo2 23
#define trig2 19
#define isrPinT1 13
#define isrPinR1 12
#define isrPinT2 27
#define isrPinR2 26

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

double d1, t1, t2, d2;
volatile bool trig1st, trig2nd, wrongDirection, trigged1st, trigged2nd;
volatile unsigned long millis1, millis2; // millis() to check time of when they got triggered

void IRAM_ATTR ISR_trig1(){
  portENTER_CRITICAL_ISR(&mux);
  trig1st = true;
  millis1 = millis();
  portEXIT_CRITICAL_ISR(&mux);    
}

void IRAM_ATTR ISR_trig2(){
  portENTER_CRITICAL_ISR(&mux);
  trig2nd = true;
  millis2 = millis();
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
  d1 = 0;
  d2 = 0;
  t1 = 0;
  t2 = 0;
  millis1 = 0;
  millis2 = 0;
  
  attachInterrupt(digitalPinToInterrupt(isrPinR1), ISR_trig1, RISING);
  attachInterrupt(digitalPinToInterrupt(isrPinR2), ISR_trig2, RISING);  
}

void loop() {
  // put your main code here, to run repeatedly:
  trigger1(); 

  if(d1 >= 15 && d1 <= 25){
    portENTER_CRITICAL(&mux);
    digitalWrite(isrPinT1, HIGH);
    portEXIT_CRITICAL(&mux);
  }

  if(trig1st){
    portENTER_CRITICAL(&mux);
    digitalWrite(isrPinT1, LOW);
    trig1st = false;
    portEXIT_CRITICAL(&mux);    
    if((millis1 < millis2) || (millis2 == 0 && millis1 !=0)){
    Serial.print(d1);
    Serial.print("    ,  ");
    Serial.println(d2);
    }
  }

  trigger2(); 

  if(d2 >= 15 && d2 <= 25){
    portENTER_CRITICAL(&mux);
    digitalWrite(isrPinT2, HIGH);
    portEXIT_CRITICAL(&mux);
  }

  if(trig2nd){
    portENTER_CRITICAL(&mux);
    trig2nd = false;
    digitalWrite(isrPinT2, LOW);
    if((millis1 > millis2) || (millis1 == 0 && millis2 != 0))
      wrongDirection = true;
    portEXIT_CRITICAL(&mux);
  }
  
  if(wrongDirection){
    Serial.println("Wrong direction");
    portENTER_CRITICAL(&mux);
    wrongDirection = false;
    portEXIT_CRITICAL(&mux);
  }

  millis1 = 0;
  millis2 = 0;
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
