#include <SPI.h>
#include <LoRa.h>

#define LSS 5
#define LRST 16
#define DIO0 17

#define LED 33
#define BT1 25
#define BT2 26

#define strBegin "LoRa CLIENT-1-01a"
#define c1 "0"
#define c2 "1"
#define c3 "a"
#define c11 '0'
#define c33 'a'

// ------------------------------ WDT SETUP -------------------------------
uint32_t RUN_WATCH_DOG = 0;
#define WATCHDOG_TIMEOUT_S 60  // 1 minute = 60s
hw_timer_t* watchDogTimer = NULL;

void IRAM_ATTR watchDogInterrupt(void);
void watchDogRefresh(void);

uint8_t counterRx = 0;
uint8_t counterTx = 0;

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  pinMode(BT1, INPUT);
  pinMode(BT2, INPUT);

  watchDogTimer = timerBegin(2, 80, true);
  timerAttachInterrupt(watchDogTimer, &watchDogInterrupt, true);
  timerAlarmWrite(watchDogTimer, WATCHDOG_TIMEOUT_S * 1000000, false);
  timerAlarmEnable(watchDogTimer);

  while (!Serial)
    ;

  Serial.println(strBegin);
  LoRa.setPins(LSS, LRST, DIO0);

  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    delay(100);
    while (1)
      ;
  }

  digitalWrite(LED, LOW);
}

void loop() {
  if ((uint32_t)(millis() - RUN_WATCH_DOG) >= 5000) {
    RUN_WATCH_DOG = millis();
    watchDogRefresh();
  }

  if (digitalRead(BT1) == 0 && digitalRead(BT2) == 1) {
    LoRa.begin(433E6);
    LoRa.beginPacket();
    LoRa.print(c1);
    LoRa.endPacket();
    digitalWrite(LED, HIGH);
    counterRx = 0;
  } else if (digitalRead(BT2) == 0 && digitalRead(BT1) == 1) {
    LoRa.begin(433E6);
    LoRa.beginPacket();
    LoRa.print(c2);
    LoRa.endPacket();
    digitalWrite(LED, HIGH);
    counterRx = 0;
  } else if (digitalRead(BT2) == 1 && digitalRead(BT1) == 1 && counterRx < 10) {
    LoRa.begin(433E6);
    LoRa.beginPacket();
    LoRa.print(c3);
    LoRa.endPacket();
    digitalWrite(LED, LOW);
    counterRx++;
  }
}


void IRAM_ATTR watchDogInterrupt(void) {
  Serial.println("REBOOT !!!");
  ESP.restart();
}

void watchDogRefresh(void) {
  timerWrite(watchDogTimer, 0);  //reset thời gian của timer
}