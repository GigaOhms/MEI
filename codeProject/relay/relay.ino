#include "ESP8266WiFi.h"
#include <stdint.h>

int tDelay = 1000;
int latchPin = 4;       // (11) ST_CP [RCK] on 74HC595
int clockPin = 16;       // (9) SH_CP [SCK] on 74HC595
int dataPin = 15;        // (12) DS [S1] on 74HC595

void out1byte(uint8_t byteout){
   uint8_t i;
   // byte BSER = byteout.7;
   for (i = 0; i < 8; i++){
      digitalWrite(dataPin, byteout & 0x80);
      digitalWrite(clockPin, 0x0);
      digitalWrite(clockPin, 0x1);
      byteout = byteout << 1;
   }
}

void out2bit(uint8_t byteout){
  uint8_t i;
  for (i = 0; i < 2; i++){
      digitalWrite(dataPin, byteout & 0x2);
      digitalWrite(clockPin, 0x0);
      digitalWrite(clockPin, 0x1);
      byteout = byteout << 1;
   }
}

void out16led(uint16_t y){
   uint8_t yd, yc;
   yd = y; yc = y >> 8;
   out2bit(yc);
   out1byte(yd);
   digitalWrite(latchPin, 0x0);
   digitalWrite(latchPin, 0x1);
}

uint16_t y = 0;
uint8_t k = 0;

byte leds = 0;

void setup() {
  pinMode(latchPin, 0x1);
  pinMode(dataPin, 0x1);
  pinMode(clockPin, 0x1);
  digitalWrite(dataPin, 0x0);
  digitalWrite(clockPin, 0x0);
  digitalWrite(latchPin, 0x0);
}

void loop() {
  y = 0;
  out16led(y);
  delay(tDelay);
  // for (k = 0; k < 10; k++){
  //   y = (y << 1) + 1;
  //     out16led(y);
  //     delay(tDelay);
  // }
  y = 1;
  for (k = 0; k < 10; k++){
    out16led(y);
    y = y << 1;
    delay(tDelay);
  }
}
