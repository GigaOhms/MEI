#include <Arduino.h>

#define BT1 2
#define BT2 3
#define LED1 4
#define LED2 5

unsigned long time1;
unsigned long time2;

byte STT1 = 0;
byte STT2 = 0;

void EVEN_LED1(void);
void EVEN_LED2(void);

void setup()
{
  pinMode(BT1, INPUT);
  pinMode(BT2, INPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
}

void loop() {
  if (digitalRead(BT1) == 0){
    if (STT1 == 0){
      STT1 = 1;
      time1 = millis();
    }
  }
  
  if (STT1 == 1 && digitalRead(BT2) == 0){
    if (STT2 == 0){
      STT2 = 1;
      time2 = millis();
    }
  }

  if (STT1 == 1)
    EVEN_LED1();
  if (STT2 == 1)
    EVEN_LED2();
}

void EVEN_LED1(void){
  digitalWrite(LED1, HIGH);
  if (millis() - time1 >= 5000){
    STT1 = 0;
    digitalWrite(LED1, LOW);
  }
}

void EVEN_LED2(void){
  digitalWrite(LED2, HIGH);
  if (millis() - time2 >= 10000){
    STT2 = 0;
    digitalWrite(LED2, LOW);
  }
}