#include "ESP8266WiFi.h"
#include <stdint.h>

#define tDelay 1000
#define latchPin 4          // (11) ST_CP [RCK] on 74HC595
#define clockPin 16         // (9) SH_CP [SCK] on 74HC595
#define dataPin 15          // (12) DS [S1] on 74HC595

uint8_t led_STT = 0;
uint8_t k = 1;
uint8_t ledON[9] = {0, 0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80};
uint8_t ledOFF[9] = {0, 0xfe, 0xfd, 0xfb, 0xf7, 0xef, 0xdf, 0xbf, 0x7f};
byte leds = 0;

void out1byte(uint8_t byteout){
   uint8_t i;
   for (i = 0; i < 8; i++){
      digitalWrite(dataPin, byteout & 0x80);
      digitalWrite(clockPin, 0x0);
      digitalWrite(clockPin, 0x1);
      byteout = byteout << 1;
   }
   digitalWrite(latchPin, 0x0);
   digitalWrite(latchPin, 0x1);
}

void ON_LED(int n){
   led_STT |= ledON[n];
   out1byte(led_STT);
}

void OFF_LED(int n){
   led_STT &= ledOFF[n];
   out1byte(led_STT);
}


void setup() {
  pinMode(latchPin, 0x1);
  pinMode(latchPin, 0x1);
  pinMode(dataPin, 0x1);
  pinMode(clockPin, 0x1);
  digitalWrite(dataPin, 0x0);
  digitalWrite(clockPin, 0x0);
  digitalWrite(latchPin, 0x0);
}

void loop() {
  for (int i = 1; i <= 5; i++){
    ON_LED(i);
    delay(200);
  }
  OFF_LED(0);
  delay(200);
}