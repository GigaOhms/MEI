#include <SPI.h>
#include <LoRa.h>

#define LSS 5
#define LRST 16
#define DIO0 17

#define LED1 26
#define LED2 25

#define INA 32
#define INB 33

#define strBegin "LoRa MASTER-1-01a"
#define c1 '0'
#define c2 '1'
#define c3 'a'
#define c11 "0"
#define c33 "a"

uint8_t STT = 0;
uint8_t PRE_STT = 0;
uint32_t TIME = 0;

// ------------------------------ WDT SETUP -------------------------------
uint32_t RUN_WATCH_DOG = 0;
#define WATCHDOG_TIMEOUT_S 60  // 1 minute = 60s
hw_timer_t* watchDogTimer = NULL;

void IRAM_ATTR watchDogInterrupt(void);
void watchDogRefresh(void);

uint8_t counter = 0;

void forward(void);
void backward(void);
void stoppp(void);


void setup() {
  Serial.begin(115200);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(INA, OUTPUT);
  pinMode(INB, OUTPUT);

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

  stoppp();
  TIME = millis();
}

void loop() {
  if ((uint32_t)(millis() - RUN_WATCH_DOG) >= 5000) {
    RUN_WATCH_DOG = millis();
    watchDogRefresh();
  }

  if ((uint32_t)(millis() - TIME > 200)) {
    LoRa.begin(433E6);
    TIME = millis();
  }

  if (LoRa.parsePacket()) {
    if (LoRa.available()) {
      char c = (char)LoRa.read();
      if (c == c1)
        STT = 1;
      else if (c == c2)
        STT = 2;
      else if (c == c3)
        STT = 0;
    }
  }

  if (STT == 0) 
    stoppp();
  else if (STT == 1) 
    forward();
  else if (STT == 2) 
    backward();
  
}

void IRAM_ATTR watchDogInterrupt(void) {
  Serial.println("REBOOT !!!");
  ESP.restart();
}

void watchDogRefresh(void) {
  timerWrite(watchDogTimer, 0);  //reset thời gian của timer
}

void forward(void) {
  digitalWrite(INA, LOW);
  digitalWrite(INB, HIGH);
}

void backward(void) {
  digitalWrite(INA, HIGH);
  digitalWrite(INB, LOW);
}

void stoppp(void) {
  digitalWrite(INA, LOW);
  digitalWrite(INB, LOW);
}
