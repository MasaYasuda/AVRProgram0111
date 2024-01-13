#include <Arduino.h>

unsigned long tmpTime=0;
int tmpState=0;

void setup(){
  Serial.begin(9600);
  pinMode(13,OUTPUT);
  digitalWrite(13,LOW);
}

void loop(){
  if(Serial.available()>0){
    unsigned char tmpData=Serial.read();
    if(tmpData==0b01000000){
      digitalWrite(13,1-tmpState);
      tmpState=1-tmpState;
    }
  }
}