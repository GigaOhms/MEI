#include <Arduino.h>
#include <stdint.h>

// -------------- BEGIN PIN CONFIG -----------------------

#define batteryPin  36      // Dien ap tren Pin
#define ntcPin      39      // Cam bien nhiet NTC on-board ESP32
#define dht11Pin    34      // Cam bien nhiet NTC on-board ESP32

#define line_A0_Pin 35      // Chan A0 cam bien do line

#define RF01        26      // RF433 chan D0
#define RF02        25      // RF433 chan D1
#define RF03        33      // RF433 chan D2
#define RF04        22      // RF433 chan D3

#define latchPin    14      // ST_CP [RCK] on 74HC595
#define clockPin    12      // SH_CP [SCK] on 74HC595
#define dataPin     27      // DS [S1] on 74HC595

#define rgbPin      13      // Led RGB

#define trigPin     2       // Chan TRIG cam bien SR05
#define echoPin     15      // Chan ECHO cam bien SR05

#define servoPin    4       // Servo Pin

#define motor11     16      // Module L298N 1, chan 1
#define motor12     5       // Module L298N 1, chan 2
#define motor13     19      // Module L298N 1, chan 3
#define motor14     21      // Module L298N 1, chan 4

#define motor21     17      // Module L298N 2, chan 1
#define motor22     18      // Module L298N 2, chan 2
#define motor23     22      // Module L298N 2, chan 3
#define motor24     23      // Module L298N 2, chan 4

// -------------- END PIN CONFIG -----------------------


// ----------------------- Timer --------
#define tDelay 400
hw_timer_t * timer = NULL;
uint32_t isrCounter = 0;

void IRAM_ATTR onTimer();
void delay1s()

// ----------------------- ADC Battery Voltage ---------
volatile float BAT_VAL;
#define CALIB_BAT (4.468864469e-3)

// ----------------------- ADC NTC value ---------------
volatile float NTC_VAL;

// ----------------------- 4 LED and LIGHT -------------
uint8_t led_STT = 0;
uint8_t k = 1;
uint8_t ledON[6] = {0xff, 0x1, 0x2, 0x4, 0x8, 0x10};
uint8_t ledOFF[6] = {0, 0xfe, 0xfd, 0xfb, 0xf7, 0xef};
byte leds = 0;

void out1byte(uint8_t byteout);
void ON_LED(int n);
void OFF_LED(int n);


void setup() {
  Serial.begin(9600);

// -------- Pin setup ---------
  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  digitalWrite(dataPin, LOW);
  digitalWrite(clockPin, LOW);
  digitalWrite(latchPin, LOW);

//--------- Timer setup ------
  timer = timerBegin(0, 80000, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 1000, true);
  timerAlarmEnable(timer);
}

void loop() {
  for (int i = 1; i <= 4; i++){
    for (int j = 1; j <= 4 - i + 1; j++){
      ON_LED(j);
      if(j != 1)
        OFF_LED(j - 1);
      delay(tDelay);
    }
  }

  delay(tDelay);
  OFF_LED(0);
  delay(tDelay);
  ON_LED(0);
  delay(tDelay);
  OFF_LED(0);
  delay(tDelay);
  ON_LED(0);
  delay(tDelay);
  OFF_LED(0);
  delay(tDelay);
}


void IRAM_ATTR onTimer(){
  isrCounter++;
}


void out1byte(uint8_t byteout){
  uint8_t i;
  for (i = 0; i < 8; i++){
    digitalWrite(dataPin, byteout & 0x80);
    digitalWrite(clockPin, LOW);
    digitalWrite(clockPin, HIGH);
    byteout = byteout << 1;
  }
  digitalWrite(latchPin, LOW);
  digitalWrite(latchPin, HIGH);
}

void ON_LED(int n){
   led_STT |= ledON[n];
   out1byte(led_STT);
}

void OFF_LED(int n){
   led_STT &= ledOFF[n];
   out1byte(led_STT);
}