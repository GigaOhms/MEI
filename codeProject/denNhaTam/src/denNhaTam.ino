// #include <ESP8266WiFi.h>

// const int PIR_PIN = 5;          // Pin D1 connected to the PIR sensor 
// const int LED_PIN = 4;          // Pin D2 connected to the LED (or relay)
// const int outTimer = 10000;   // Time to keep the lights on after motion detection

// unsigned long currentTimer = 0;
// bool motionDetected = 0x0;

// // void setup() {
// //   Serial.begin(9600);
// //   pinMode(PIR_PIN, INPUT);
// //   pinMode(LED_PIN, OUTPUT);
// // }

// // void loop() {
// //   int pirState = digitalRead(PIR_PIN);
// //   if (pirState == HIGH && !motionDetected) {
// //     motionDetected = true;
// //     digitalWrite(LED_PIN, HIGH);
// //     currentTimer = millis();
// //     Serial.println("1");
// //   } else if (motionDetected && (millis() - currentTimer > outTimer)) {
// //     motionDetected = false;
// //     digitalWrite(LED_PIN, LOW);
// //     Serial.println("0");
// //   }
// // }

// ---------------------------------------------------------- Hi U ------------------------------------

// unsigned long timer = 0;  // Timer variable
// int state = 0x0;          // Current state of LED

// void ICACHE_RAM_ATTR isr() {
//   timer = millis();
//   motionDetected = 0x1;
// }

// void setup() {
//   Serial.begin(9600);
//   pinMode(LED_PIN, OUTPUT);
//   pinMode(PIR_PIN, INPUT_PULLDOWN_16);
//   attachInterrupt(PIR_PIN, isr, RISING);
// }

// void loop() {
//   if (millis() - timer < outTimer && motionDetected){
//     digitalWrite(LED_PIN, HIGH);
//     Serial.printf("light off after %6ld s\n\r", 9 - (millis() - timer)/1000);
//     delay(1000);
//   } 
//   else {
//     digitalWrite(LED_PIN, LOW);
//     motionDetected = 0x0;
//   }
// }

// -------------------------------------------- Long time no see ------------------------------

// #include <ESP8266WiFi.h>
// #include <Ticker.h>


// Ticker timer;

// const int PIR_PIN = 5;
// const int LED_PIN = 4;
// const int outTimer = 10;
// volatile unsigned long currentTimer = 0;
// volatile unsigned long realTimer = 0;
// bool motionDetected = 0x0;

// // Ngat timer, 1s thuc hien 1 lan
// void ICACHE_RAM_ATTR delay1s() {
//   if (motionDetected)
//     Serial.printf("light off after %6ld s\n\r", 9 - (realTimer - currentTimer));
//   realTimer++;
// 	timer1_write(5000000);
// }

// // Ngat ngoai, thuc hien khi co tin hieu muc cao tu cam bien
// void ICACHE_RAM_ATTR detect() {
//   currentTimer = realTimer;   // reset bien dem khi phat hien chuyen dong
//   motionDetected = 0x1;       // Chuyen sang trang thai phat hien chuyen dong
// }


// void setup() {
// 	Serial.begin(9600);

// 	timer1_attachInterrupt(delay1s);
// 	timer1_enable(TIM_DIV16, TIM_EDGE, TIM_SINGLE);
// 	timer1_write(5000000); // 5000000 / 5 ticks per us from TIM_DIV16 == 1,00,000 us interval 

//   pinMode(LED_PIN, OUTPUT);
//   pinMode(PIR_PIN, INPUT_PULLDOWN_16);
//   attachInterrupt(PIR_PIN, detect, RISING); // setup ngat ngoai
// }

// void loop() {
//   if ((realTimer - currentTimer < outTimer) && motionDetected)
//     digitalWrite(LED_PIN, HIGH);
//   else {
//     digitalWrite(LED_PIN, LOW);
//     motionDetected = 0x0;
//   }
// }


// ----------------------- 10 relay ------------------------------------
#include "ESP8266WiFi.h"
#include <stdint.h>

int tDelay = 1000;
int latchPin = 4;       // (11) ST_CP [RCK] on 74HC595
int clockPin = 16;       // (9) SH_CP [SCK] on 74HC595
int dataPin = 15;        // (12) DS [S1] on 74HC595

void xuat1byte(uint8_t byteout){
   uint8_t i; 
   // byte BSER = byteout.7;
   for (i = 0; i < 8; i++){
      digitalWrite(dataPin, byteout & 0x80 );
      digitalWrite(clockPin, LOW);
      digitalWrite(clockPin, HIGH);
      byteout = byteout << 1;
   }
}

void xuat16led(uint16_t y){
   uint8_t yd, yc;
   yd = y; yc = y >> 8;
   xuat1byte(yc);
   xuat1byte(yd);
   digitalWrite(latchPin, LOW);
   digitalWrite(latchPin, HIGH);
}

uint16_t y;
uint8_t k;

byte leds = 0;

void setup() {
  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);  
  pinMode(clockPin, OUTPUT);
}

void loop() {
  y = 0;
  xuat16led(y);
  delay(tDelay);
  for (k = 0; k < 16; k++){
    y = (y << 1) + 1;
      xuat16led(y);
      delay(tDelay);
  }
}