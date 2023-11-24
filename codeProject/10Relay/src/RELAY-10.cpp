#include "ESP8266WiFi.h"
#include <stdint.h>

#define tDelay 1000
#define latchPin 4          // (11) ST_CP [RCK] on 74HC595
#define clockPin 16         // (9) SH_CP [SCK] on 74HC595
#define dataPin 15          // (12) DS [S1] on 74HC595

uint16_t RELAY_STT = 0;
uint8_t k = 1;
uint16_t relayON[11] = {0, 0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80, 0x100, 0x200};
uint16_t relayOFF[11] = {0, 0xfffe, 0xfffd, 0xfffb, 0xfff7, 0xffef, 0xffdf, 0xffbf, 0xff7f, 0xfeff, 0xfdff};
byte leds = 0;

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

void ON_RELAY(int n){
   RELAY_STT |= relayON[n];
   out16led(RELAY_STT);
}

void OFF_RELAY(int n){
   RELAY_STT &= relayOFF[n];
   out16led(RELAY_STT);
}


void setup() {
  pinMode(latchPin, 0x1);
  pinMode(dataPin, 0x1);
  pinMode(clockPin, 0x1);
  digitalWrite(dataPin, 0x0);
  digitalWrite(clockPin, 0x0);
  digitalWrite(latchPin, 0x0);
}

void loop() {
   OFF_RELAY(0);

   ON_RELAY(1);         // ON relay
   delay(tDelay);

   ON_RELAY(3);         // ON relay
   delay(tDelay);

   ON_RELAY(5);         // ON relay
   delay(tDelay);

   OFF_RELAY(1);        // OFF relay
   delay(tDelay);

   OFF_RELAY(5);        // OFF relay
   delay(tDelay);

   OFF_RELAY(3);        // OFF relay
   delay(tDelay);
}