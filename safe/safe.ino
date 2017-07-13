#include "stdlib.h"

#define SERVO_PIN D6
#define STATE_CLOSE 0
#define STATE_OPEN 1

const int latchPin = D2;
const int clockPin = D3;
const int dataPin = D4;

int encoderPinA = D0;
int encoderPinB = D1;
int enterButtonPin = D7;

int inputButtonPin = D8;
int openButtonPin = D9;

char doorState = STATE_OPEN;

volatile int lastEncoded = 0;
volatile long encoderValue = 0;
long lastencoderValue = 0;

int lastMSB = 0;
int lastLSB = 0;

byte Tab[]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0xff};
byte data[8]{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
int passWord[3] = {0,0,0};  //default password is 0 0 0

long readEncoderValue(void){
    return abs(encoderValue/4);
}

boolean enterButtonPushDown(void){
  if(!digitalRead(enterButtonPin)){
    delay(5);
    if(!digitalRead(enterButtonPin))
      return true;
  }
  return false;
}

void displayNumber(int index, int value){
  data[7-index] = Tab[value];
  for(int i =0; i<8; i++){
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, MSBFIRST, data[i]);
    digitalWrite(latchPin, HIGH);  
  }
}

void openEnable(boolean en){
  for(int i=0;i<50;i++){
    if(en){
      digitalWrite(SERVO_PIN,HIGH);
      delayMicroseconds(500);
    }else{
      digitalWrite(SERVO_PIN,HIGH);
      delayMicroseconds(1500);
    }
    digitalWrite(SERVO_PIN,LOW);
    delay(20);
  }
  delay(500);
}

void checkDoor(void){
  if(digitalRead(inputButtonPin) == LOW){
    openEnable(false); //open the door
    doorState = STATE_CLOSE;
  }
}

boolean checkSetButton(void){
  if(digitalRead(openButtonPin)){
    delay(20);
    if(digitalRead(openButtonPin)){
      return true;
    }
  }
  return false;
}

boolean checkEnterButton(void){
  if(digitalRead(enterButtonPin) == LOW){
    delay(100);
    if(digitalRead(enterButtonPin) == LOW)
      return true;
  }
  return false;
}

boolean inputPassword(void){
  boolean value = true;
  for(int i=0;i<3;i++){
    displayNumber(7,readEncoderValue()%10);
    while(checkEnterButton()==false){
      displayNumber(7,readEncoderValue()%10);
      displayNumber(i,readEncoderValue()%10);
      delay(100);
      displayNumber(i,10);
      delay(100);
    }
    displayNumber(i,readEncoderValue()%10);
    if(passWord[i] != readEncoderValue() % 10)
      value = false;
  }
  displayNumber(7,10);
  delay(200);
  return value;
}

void clearDisplay(void){
  for(int i=0;i<8;i++){
    displayNumber(i,10);
  }
}

void waitPassDisplay(int index){
    clearDisplay();
    displayNumber(index,8);
    delay(50);
}

void setup() {
  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);  
  pinMode(clockPin, OUTPUT);

  pinMode(SERVO_PIN, OUTPUT);
  pinMode(openButtonPin,INPUT);
  pinMode(inputButtonPin,INPUT);
  
  pinMode(encoderPinA, INPUT); 
  pinMode(encoderPinB, INPUT);
  pinMode(enterButtonPin, INPUT);

  digitalWrite(encoderPinA, HIGH); //turn pullup resistor on
  digitalWrite(encoderPinB, HIGH); //turn pullup resistor on

  clearDisplay();
  checkDoor();
  attachInterrupt(D0, updateEncoder, CHANGE); 
  attachInterrupt(D1, updateEncoder, CHANGE);
}

void loop() {
  static int index = 0;
  if(doorState == STATE_CLOSE){
    if(checkSetButton()){
      clearDisplay();
      boolean pass = inputPassword();
      while(!checkSetButton());
      if(pass){
        openEnable(true);
        doorState = STATE_OPEN;
      }
    }else{
      waitPassDisplay(index++);
    }
  }
  if(doorState == STATE_OPEN){
    checkDoor();
  }

  if(index == 9)
    index = 0;
  delay(20);
}

void updateEncoder(){
  int MSB = digitalRead(encoderPinA); //MSB = most significant bit
  int LSB = digitalRead(encoderPinB); //LSB = least significant bit
  int encoded = (MSB << 1) |LSB; //converting the 2 pin value to single number
  int sum  = (lastEncoded << 2) | encoded; //adding it to the previous encoded value
  if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderValue ++;
  if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderValue --;
  lastEncoded = encoded; //store this value for next time
}
