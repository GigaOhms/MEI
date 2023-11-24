#include <SoftwareSerial.h>

// BOX xẩm màu

#define BTN 4
#define LED 16
#define rxPin 12
#define txPin 14

String s = "";
char ch;
uint8_t counter = 0;
uint32_t pretime = 0;

SoftwareSerial mySerial(rxPin, txPin);

bool S_BTN = 0;

void setup() {
  Serial.begin(115200);
  mySerial.begin(9600);

  pinMode(BTN, INPUT);
  pinMode(LED, OUTPUT);

  Serial.println("Hello MEI GROUP");
}

void loop() {
  if (mySerial.available()) {
    char c = mySerial.read();
    if (c >= 97 && c <= 107) { // a, b, c, ... , k
      ch = c;
      counter = 3;
      pretime = millis();
      digitalWrite(LED, HIGH);
    } else {
      mySerial.print(c);    // Change -> mySerial
    }
  }

  if (counter > 0 && (uint32_t)(millis() - pretime) >= 200) {
    counter--;
    mySerial.print(ch);     // Change -> mySerial
    // Serial.println();     // XOA
    digitalWrite(LED, counter % 2);
    pretime = millis();     
  }
}

