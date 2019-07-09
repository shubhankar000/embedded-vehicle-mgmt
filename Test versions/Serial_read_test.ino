void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  while (Serial.available() == 0){}
  
  fbyte b = Serial.read();
  Serial.println(b);
}
