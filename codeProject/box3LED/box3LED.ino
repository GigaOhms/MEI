#include <SoftwareSerial.h>

#define ID_DEVICE 8

#define LED_X 16
#define LED_V 5
#define LED_D 4

#define XANH 1
#define VANG 2
#define DO   3

#define rxPin 12
#define txPin 14


uint32_t preTime;
uint32_t time30s;
uint32_t time500ms;
uint32_t timeBlink;

uint8_t LED = LED_X;

bool S_LED_X = 0;
bool S_LED_V = 0;
bool S_LED_D = 0;
bool STT_LED = 0;

bool blinkCheck = 0;

bool checkData = 0;
uint16_t id;
uint16_t mode;
String s = "";

void checkMode(uint16_t inMode);
void offLed(void);
void blink30s(void);

SoftwareSerial mySerial(rxPin, txPin);


void setup() {
  // put your setup code here, to run once:
  pinMode(LED_X, OUTPUT);
  pinMode(LED_V, OUTPUT);
  pinMode(LED_D, OUTPUT);

  mySerial.begin(9600);
  Serial.begin(115200);

  offLed();
}

void loop() {
  if (blinkCheck == 1)
    blink30s();

  if (mySerial.available()) {
    char c = mySerial.read();
    if (c == '*') checkData = 1;
    else if (c != '*' && checkData == 1) {
      if (c != '#') s += c;
      else {
        uint16_t j = s.indexOf('/');
        uint16_t k = s.indexOf('/', j + 1);
        id = (s.substring(j + 1, k)).toInt();
        if (id == ID_DEVICE) {
          mode = (s.substring(k + 1, s.length())).toInt();
          offLed();
          checkMode(mode);
          blinkCheck = 1;
          time30s = millis();
          time500ms = millis();
        }
        checkData = 0;
        s = "";
      }
    }
  }
}

void checkMode(uint16_t inMode) {
  if (inMode == XANH) {
    LED = LED_X;
    S_LED_X = 1;
    S_LED_D = 0;
    S_LED_V = 0;
  } else if (inMode == DO) {
    LED = LED_D;
    S_LED_X = 0;
    S_LED_D = 1;
    S_LED_V = 0;
  } else if (inMode == VANG) {
    LED = LED_V;
    S_LED_X = 0;
    S_LED_D = 0;
    S_LED_V = 1;
  }
}

void offLed(void) {
  S_LED_X = 0;
  S_LED_D = 0;
  S_LED_V = 0;

  digitalWrite(LED_X, S_LED_X);
  digitalWrite(LED_D, S_LED_D);
  digitalWrite(LED_V, S_LED_V);

  blinkCheck = 0;
}

void blink30s(void) {
  if ((uint32_t)(millis() - time30s) <= 30000) {
    if ((uint32_t)(millis() - time500ms) >= 500) {
      time500ms = millis();
      digitalWrite(LED, STT_LED);
      STT_LED = STT_LED ? 0 : 1;
    }
  } else offLed();
}
