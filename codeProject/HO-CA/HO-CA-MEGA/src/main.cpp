#include <Arduino.h>
#include <Adafruit_MAX31865.h>
#include <Wire.h>
#include "DFRobot_PH.h"
#include <PZEM004Tv30.h>
#include <avr/wdt.h>
#include <EEPROM.h>

#define tDelay 500

// ------------ PIN CONFIG ---------------------

#define PIN_OXI A6
#define PIN_PH A0
#define PIN_DUC A12

#define TRIG 38
#define ECHO 17

#define BUZZER 3

#define DC_TFT28 49
#define CS_TFT28 47
#define CS_PT100 48
#define MISO 50
#define MOSI 51
#define SCK 52

#define RL1 43
#define RL2 41
#define RL3 39
#define RL4 37
#define RL5 35
#define RL6 30
#define RL7 31
#define RL8 29

#define BT1 5
#define BT2 4
#define BT3 7
#define BT4 6
#define BT5 9
#define BT6 11
#define BT7 10
#define BT8 23

#define PHAO1 22
#define PHAO2 24
#define PHAO3 33
#define PHAO4 20

#define LED1 25
#define LED2 27
#define LED3 28
#define LED4 26
#define LED5 21
#define LED6 13
#define LED7 8
#define LED8 2

#define NOW_TIME millis()
uint32_t PRE_TIME_WDT;

// ------------ PARAMETER -----------------------

uint8_t i = 1, j = 1;

uint32_t PRE_TIME = 0;
uint16_t DISTANCE = 0;
uint8_t counter = 0;

uint8_t distanceValue[4] = { 0, 0, 0, 0 };

Adafruit_MAX31865 thermo = Adafruit_MAX31865(CS_PT100, MOSI, MISO, SCK);  // SPI: CS, DI, DO, CLK
float TEMPERATURE = 0;
#define RREF 430.0
#define RNOMINAL 100.0

float OXIGEN = 0.0;
#define VREF 5000     //VREF (mv)
#define ADC_RES 1023  //ADC Resolution
//Single point calibration OXIGEN
#define CAL_V (1600)  //mv
#define CAL_T (25)    //℃
const uint16_t DO_Table[41] = {
  14460, 14220, 13820, 13440, 13090, 12740, 12420, 12110, 11810, 11530,
  11260, 11010, 10770, 10530, 10300, 10080, 9860, 9660, 9460, 9270,
  9080, 8900, 8730, 8570, 8410, 8250, 8110, 7960, 7820, 7690,
  7560, 7430, 7300, 7180, 7070, 6950, 6840, 6730, 6630, 6530, 6410
};

DFRobot_PH ph;
float PH_VAL = 0.0;
uint16_t ADC_Raw_PH;
float ADC_Voltage_PH;
uint32_t ADC_Raw_PH_SUM = 0;
uint16_t ADC_Raw_PH_COUNTER = 0;

//  uint16_t ADC_Raw_PH = analogRead(PIN_PH);
//  uint16_t ADC_Voltage_PH = uint32_t(VREF) * ADC_Raw_PH / ADC_RES;

PZEM004Tv30 pzem(Serial1);
float VOLTAGE;
float CURRENT;
float POWER;
float ENERGY;
float FREQUENCE;
float PF;

int count = 0;

bool S_BT1 = 0;
bool S_BT2 = 0;
bool S_BT3 = 0;
bool S_BT4 = 0;
bool S_BT5 = 0;
bool S_BT6 = 0;
bool S_BT7 = 0;
bool S_BT8 = 0;

bool S_LED1 = 0;
bool S_LED2 = 0;
bool S_LED3 = 0;
bool S_LED4 = 0;
bool S_LED5 = 0;
bool S_LED6 = 0;
bool S_LED7 = 0;
bool S_LED8 = 0;

bool S_RL1 = 0;
bool S_RL2 = 0;
bool S_RL3 = 0;
bool S_RL4 = 0;
bool S_RL5 = 0;
bool S_RL6 = 0;
bool S_RL7 = 0;
bool S_RL8 = 0;

#define addr_RL5 0
#define addr_RL6 1
#define addr_RL7 2
#define addr_RL8 3

String s = "";
bool SEND_SELECT = 0;

// ------------ FUNCTION -------------------------

void getSR05(void);   // Lay gia tri Khoang cach cua SR05
void getPT100(void);  // Lay gia tri nhiet do PT100
void getOXI(void);    // Lay gia tri nong do OXI trong nuoc mg/L
void getPH(void);     // Lay gia tri cam bien pH
void getPZEM(void);   // lay thong so dien tu PZEM
void getAVG(void);    // lay gia tri trung binh chan analog


void GET_SENSOR(void);
void SEND_ESP_WATER(void);
void SEND_ESP_RL(void);
void SEND_ESP_ELEC(void);
void MANUAL_BUTTON(void);
void AUTO_BUTTON(void);
void TWO_PIP(void);  // PIP-PIP
void EEPROM_SETUP(void);


// ------------------------------ SETUP ------------------------------
void setup() {
  Serial.begin(115200);
  // Serial1.begin(9600);
  Serial2.begin(9600);
  Serial3.begin(9600);

  // ph.begin();  // --------------- SAI CODE ------------------------
  thermo.begin(MAX31865_3WIRE);  // set to 2WIRE or 4WIRE as necessary

  pinMode(BUZZER, OUTPUT);
  pinMode(TRIG, OUTPUT);
  digitalWrite(TRIG, HIGH);
  pinMode(ECHO, INPUT);

  pinMode(PIN_OXI, INPUT);
  pinMode(PIN_PH, INPUT);
  pinMode(PIN_DUC, INPUT);

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);
  pinMode(LED5, OUTPUT);
  pinMode(LED6, OUTPUT);
  pinMode(LED7, OUTPUT);
  pinMode(LED8, OUTPUT);

  pinMode(RL1, OUTPUT);
  pinMode(RL2, OUTPUT);
  pinMode(RL3, OUTPUT);
  pinMode(RL4, OUTPUT);
  pinMode(RL5, OUTPUT);
  pinMode(RL6, OUTPUT);
  pinMode(RL7, OUTPUT);
  pinMode(RL8, OUTPUT);

  pinMode(BT1, INPUT);
  pinMode(BT2, INPUT);
  pinMode(BT3, INPUT);
  pinMode(BT4, INPUT);
  pinMode(BT5, INPUT);
  pinMode(BT6, INPUT);
  pinMode(BT7, INPUT);
  pinMode(BT8, INPUT);

  pinMode(PHAO1, INPUT);
  pinMode(PHAO2, INPUT);
  pinMode(PHAO3, INPUT);
  pinMode(PHAO4, INPUT);

  EEPROM_SETUP();

  TWO_PIP();
  wdt_enable(WDTO_4S);
  PRE_TIME = NOW_TIME;
  PRE_TIME_WDT = NOW_TIME;
}

void loop() {
  if ((uint32_t)(NOW_TIME - PRE_TIME_WDT) >= 2000) {
    wdt_reset();
    PRE_TIME_WDT = NOW_TIME;
  }

  getAVG();

  if ((uint32_t)(NOW_TIME - PRE_TIME) >= 5000) {
    PRE_TIME = NOW_TIME;
    GET_SENSOR();
    // Serial.println(TEMPERATURE);
    SEND_SELECT = (SEND_SELECT == 0) ? 1 : 0;
    if (SEND_SELECT)
      SEND_ESP_ELEC();
    else SEND_ESP_WATER();
  }
  
  AUTO_BUTTON();
  MANUAL_BUTTON();
}


void getSR05(void) {
  while (Serial2.read() >= 0)
    ;
  digitalWrite(TRIG, LOW);
  while (Serial2.available() <= 4) {}
  distanceValue[0] = Serial2.read();
  if (distanceValue[0] == 0xFF)
    for (uint8_t i = 1; i <= 3; i++)
      distanceValue[i] = Serial2.read();
  digitalWrite(TRIG, HIGH);
  if ((distanceValue[3] == distanceValue[1] + distanceValue[2] - 1)) {
    DISTANCE = distanceValue[1] * 256 + distanceValue[2];
    // Serial.print("Distance:\t");
    // Serial.print(DISTANCE);
    // Serial.println(" mm");
  } else {
    // Serial.println("Overrange !!");
    DISTANCE = 2000;
  }
}

void getPT100(void) {
  if (thermo.readFault()) {
    // Serial.println("READ FAILT !!!");
    thermo.clearFault();
    TEMPERATURE = 0.0f;
  } else {
    TEMPERATURE = thermo.temperature(RNOMINAL, RREF);
    // Serial.print("Temperature:\t");
    // Serial.println(TEMPERATURE);
  }
}

void getOXI(void) {
  uint8_t temperature_c = (uint8_t)TEMPERATURE;
  uint16_t ADC_Raw = analogRead(PIN_OXI);
  uint16_t ADC_Voltage = uint32_t(VREF) * ADC_Raw / ADC_RES;
  uint16_t V_saturation = (uint32_t)CAL_V + (uint32_t)35 * temperature_c - (uint32_t)CAL_T * 35;
  OXIGEN = ((uint32_t)ADC_Voltage * DO_Table[temperature_c] / V_saturation) * 0.001;  // mg/L
  // Serial.print("Oxigen:\t\t");
  // Serial.println(OXIGEN);
}

void getPH(void) {
  if (TEMPERATURE <= 1.0f)
    PH_VAL = ph.readPH(ADC_Voltage_PH, 30.0);
  else PH_VAL = ph.readPH(ADC_Voltage_PH, TEMPERATURE);
  if (ADC_Voltage_PH > 1750.0f)
    PH_VAL = PH_VAL * 0.98;
}

void getAVG(void) {
  ADC_Raw_PH_COUNTER++;
  ADC_Raw_PH_SUM += (uint32_t)analogRead(PIN_PH);
  if (ADC_Raw_PH_COUNTER >= 10000) {
    ADC_Raw_PH = (uint16_t)(ADC_Raw_PH_SUM / 10000);
    ADC_Voltage_PH = 5000.0 * (float)ADC_Raw_PH / 1023.0;
    ADC_Raw_PH_SUM = 0;
    ADC_Raw_PH_COUNTER = 0;
    Serial.print("ADC value:\t");
    Serial.print(ADC_Voltage_PH);
    Serial.print("\tpH value:\t");
    Serial.println(PH_VAL);
  }
}

void GET_SENSOR(void) {
  getPT100();  // variable: TEMPERATURE
  // getSR05();   // variable: DISTANCE
  // getOXI();   // variable: OXIGEN
  getPH();    // variable: PH_VAL
  getPZEM();  // variable: VOLTAGE, CURRENT, POWER, ENERGY, FREQUENCE, PF
}

void getPZEM(void) {
  if (!isnan(pzem.voltage()))
    VOLTAGE = pzem.voltage();
  else
    VOLTAGE = 0;

  if (!isnan(pzem.current()))
    CURRENT = pzem.current();
  else CURRENT = 0;

  if (!isnan(pzem.power()))
    POWER = pzem.power();
  else POWER = 0;

  if (!isnan(pzem.energy()))
    ENERGY = pzem.energy();
  else ENERGY = 0;

  if (!isnan(pzem.frequency()))
    FREQUENCE = pzem.frequency();
  else FREQUENCE = 0;

  if (!isnan(pzem.pf()))
    PF = pzem.pf();
  else PF = 0;
}

void SEND_ESP_WATER(void) {
  Serial3.print("a");
  // Serial3.print(TEMPERATURE);
  // Serial3.print("/");

  // Serial3.print(DISTANCE);
  // Serial3.print("/");

  // Serial3.print(OXIGEN);
  // Serial3.print("/");

  // Serial3.print(PH_VAL);
  // Serial3.print("/");

  Serial3.print(S_RL8);
  Serial3.print("/");

  Serial3.print(S_RL7);
  Serial3.print("/");

  Serial3.print(S_RL6);
  Serial3.print("/");

  Serial3.print(S_RL5);
  Serial3.print("*");
}

void SEND_ESP_RL(void) {
  Serial3.print("c");
  Serial3.print(S_RL8);
  Serial3.print("/");

  Serial3.print(S_RL7);
  Serial3.print("/");

  Serial3.print(S_RL6);
  Serial3.print("/");

  Serial3.print(S_RL5);
  Serial3.print("*");
}

void SEND_ESP_ELEC(void) {
  Serial3.print('b');
  Serial3.print(TEMPERATURE);
  Serial3.print("/");

  Serial3.print(VOLTAGE);
  Serial3.print("/");

  Serial3.print(CURRENT);
  Serial3.print("/");

  Serial3.print(POWER);
  Serial3.print("/");

  Serial3.print(ENERGY);
  Serial3.print("/");

  Serial3.print(FREQUENCE);
  Serial3.print("/");

  Serial3.print(PF);
  Serial3.print("/");

  Serial3.print(PH_VAL);
  Serial3.println("*");
}

void AUTO_BUTTON(void) {
  if (Serial3.available()) {
    PRE_TIME = NOW_TIME;
    char c = Serial3.read();
    if (c != '/' && c != '*') s += c;
    else if (c == '/' || c == '*') {
      uint8_t TEMP_CHECK = (uint8_t)(s.toInt() / 10);
      if (TEMP_CHECK == 1) {
        S_LED8 = (s.toInt() % 10);
        S_RL8 = S_LED8;
        digitalWrite(LED8, S_LED8);
        digitalWrite(RL8, S_RL8);
        EEPROM.update(addr_RL8, (int)S_RL8);
        SEND_ESP_RL();
      } else if (TEMP_CHECK == 2) {
        S_LED7 = (s.toInt() % 10);
        S_RL7 = S_LED7;
        digitalWrite(LED7, S_LED7);
        digitalWrite(RL7, S_RL7);
        EEPROM.update(addr_RL7, (int)S_RL7);
        SEND_ESP_RL();
      } else if (TEMP_CHECK == 3) {
        S_LED6 = (s.toInt() % 10);
        S_RL6 = S_LED6;
        digitalWrite(LED6, S_LED6);
        digitalWrite(RL6, S_RL6);
        EEPROM.update(addr_RL6, (int)S_RL6);
        SEND_ESP_RL();
      } else if (TEMP_CHECK == 4) {
        S_LED5 = (s.toInt() % 10);
        S_RL5 = S_LED5;
        digitalWrite(LED5, S_LED5);
        digitalWrite(RL5, S_RL5);
        EEPROM.update(addr_RL5, (int)S_RL5);
        SEND_ESP_RL();
      }
      s = "";
    }
  }
}

void MANUAL_BUTTON(void) {
  if (digitalRead(BT1) == 0 && S_BT1 == 0) {
    S_BT1 = 1;
    S_LED8 = S_LED8 == 0 ? 1 : 0;
    S_RL8 = S_RL8 == 0 ? 1 : 0;
    digitalWrite(LED8, S_LED8);
    digitalWrite(RL8, S_RL8);
    EEPROM.update(addr_RL8, (int)S_RL8);
    SEND_ESP_RL();
    delay(50);
  } else if (digitalRead(BT1)) S_BT1 = 0;

  if (digitalRead(BT2) == 0 && S_BT2 == 0) {
    S_BT2 = 1;
    S_RL7 = S_RL7 == 0 ? 1 : 0;
    S_LED7 = S_RL7;
    digitalWrite(LED7, S_LED7);
    digitalWrite(RL7, S_RL7);
    EEPROM.update(addr_RL7, (int)S_RL7);
    SEND_ESP_RL();
    delay(50);
  } else if (digitalRead(BT2)) S_BT2 = 0;

  if (digitalRead(BT3) == 0 && S_BT3 == 0) {
    S_BT3 = 1;
    S_RL6 = S_RL6 == 0 ? 1 : 0;
    S_LED6 = S_RL6;
    digitalWrite(LED6, S_LED6);
    digitalWrite(RL6, S_RL6);
    EEPROM.update(addr_RL6, (int)S_RL6);
    SEND_ESP_RL();
    delay(50);
  } else if (digitalRead(BT3)) S_BT3 = 0;

  if (digitalRead(BT4) == 0 && S_BT4 == 0) {
    S_BT4 = 1;
    S_RL5 = S_RL5 == 0 ? 1 : 0;
    S_LED5 = S_RL5;
    digitalWrite(LED5, S_LED5);
    digitalWrite(RL5, S_RL5);
    EEPROM.update(addr_RL5, (int)S_RL5);
    SEND_ESP_RL();
    delay(50);
  } else if (digitalRead(BT4)) S_BT4 = 0;
}

void TWO_PIP(void) {
  digitalWrite(BUZZER, HIGH);
  delay(50);
  digitalWrite(BUZZER, LOW);
  delay(50);
  digitalWrite(BUZZER, HIGH);
  delay(50);
  digitalWrite(BUZZER, LOW);
}

void EEPROM_SETUP(void) {
  uint8_t STT;
  STT = EEPROM.read(addr_RL5);
  if (STT <= 1)
    S_RL5 = (bool)STT;
  else S_RL5 = 1;
  S_LED5 = S_RL5;

  STT = EEPROM.read(addr_RL6);
  if (STT <= 1)
    S_RL6 = (bool)STT;
  else S_RL6 = 0;
  S_LED6 = S_RL6;

  STT = EEPROM.read(addr_RL7);
  if (STT <= 1)
    S_RL7 = (bool)STT;
  else S_RL7 = 1;
  S_LED7 = S_RL7;

  STT = EEPROM.read(addr_RL8);
  if (STT <= 1)
    S_RL8 = (bool)STT;
  else S_RL8 = 0;
  S_LED8 = S_RL8;

  digitalWrite(RL5, S_RL5);
  digitalWrite(RL6, S_RL6);
  digitalWrite(RL7, S_RL7);
  digitalWrite(RL8, S_RL8);
  digitalWrite(LED5, S_LED5);
  digitalWrite(LED6, S_LED6);
  digitalWrite(LED7, S_LED7);
  digitalWrite(LED8, S_LED8);
}