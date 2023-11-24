#include <SoftwareSerial.h>

#define rxPin 7
#define txPin 6
#define RL    5
#define BTN   8
#define LED   9

uint8_t CNT1, CNT2;
uint32_t randomTime = 0;
#define RANDOM_TIMEOUT_MS 5000

String s = "";
bool checkRead = 0;

bool S_BTN = 0;
bool S_LED = 0;
bool S_RL = 0;
bool SS_RL = 0;

SoftwareSerial mySerial (rxPin, txPin);

void setup() {
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  pinMode(RL, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(BTN, INPUT);
  mySerial.begin(9600);
  Serial.begin(115200);
  Serial.println("Reboot ...!");

  randomTime = millis();
}

void loop() {
  if (mySerial.available()) {
    char c = mySerial.read();
    if (c != '*') s += c;
    else {
      if (s[0] == 's') {
        S_RL = (s.substring(1, 2)).toInt();
        digitalWrite(RL, S_RL);
      } else if (s[0] == 'r') {
        if (s.equals("rd")) {
          mySerial.print('r');
          mySerial.print(S_RL);
          mySerial.print(random(100, 999));
          mySerial.print('*');
        }
      }
      Serial.println(s);
      s = "";
    }
  }

  if (SS_RL != S_RL) {
    SS_RL = S_RL;
    S_LED = S_RL;
    digitalWrite(LED, S_LED);
    if (S_RL == 1) mySerial.print("s1*");
    else if (S_RL == 0) mySerial.print("s0*");
  }
}
