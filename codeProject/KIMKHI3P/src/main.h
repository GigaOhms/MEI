/*********
  Rui Santos
  Complete instructions at https://RandomNerdTutorials.com/esp32-wi-fi-manager-asyncwebserver/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*********/

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "SPIFFS.h"
#include <PubSubClient.h>
#include <OneWire.h> 
#include <DallasTemperature.h>
#include "DHT.h"
#include <EEPROM.h>


// ----------------------------------- DEVICE CONFIG ----------------------------
const bool lastReset = 1;     // lastReset set to 1 when last loading code
#define WIFI_NAME "KIMKHI3P - WIFI SENSOR3"
#define DATA_TOPIC "KIMKHI3P/SENSOR/DATA/TB3"                   // -- DATA FORMAT []
#define RANDOM_TOPIC "KIMKHI3P/SENSOR/RD/TB3"
#define ALARM_TOPIC "KIMKHI3P/SENSOR/ALARM/TB3"                 // -- DATA FORMAT [resetCounter:sensorUpdateTimeoutS:randomCheckTimeoutS:alarmTempBoard:alarmTempSensor:alarmHumiSensor]
#define SUB_TOPIC_REQUEST "KIMKHI3P/SENSOR"                     // -- DATA FORMAT [ALL CHARACTER]
#define SUB_TOPIC_SETUP_TIME "KIMKHI3P/SENSOR/SETUP/TIME/TB3"   // -- DATA FORMAT [sensorUpdateTimeoutS:randomCheckTimeoutS]
#define SUB_TOPIC_SETUP_ALARM "KIMKHI3P/SENSOR/SETUP/ALARM/TB3" // -- DATA FORMAT [DS_t:DHT_t:DHT_h]
#define SUB_TOPIC_SETUP_MODE "KIMKHI3P/SENSOR/SETUP/MODE/TB3" // -- DATA FORMAT [DS_t:DHT_t:DHT_h]


// ----------------------------------- PIN CONFIG --------------------------------
#define LED         12
#define BTN1        27
#define pinDHT      4
#define pinDS18B20  13
#define BZ          32
#define RL1         23
#define RL2         22

float DHT_t = 0.0, DHT_h = 0.0, DS_t = 0.0;
float alarmTempBoard = 40.0f;
float alarmTempSensor = 45.0f;
float alarmHumiSensor = 95.0f;

uint8_t alarmMode;

uint32_t preTimeLed = 0;
bool LED_STT = 0;
uint32_t readSensorTime;
#define READ_SENSOR_TIMEOUT_MS 5000

OneWire oneWire(pinDS18B20);
DallasTemperature ds18b20(&oneWire);
DHT dht(pinDHT, DHT21);
void readSensor(void);
void alarmCheck(void);


// --------------------------------- EEPROM CONFIG -------------------------------
#define EEPROM_SIZE            25

#define ADDR_RESET_COUNTER      0
#define ADDR_SENSOR_TIMEOUT     4
#define ADDR_RANDOM_TIMEOUT     8
#define ADDR_BOARD_TEM_ALARM   12
#define ADDR_SENSOR_TEM_ALARM  16
#define ADDR_SENSOR_HUM_ALARM  20
#define ADDR_ALARM_MODE        24

#define SENSOR_TIMEOUT_DEFAULT   30UL
#define RANDOM_TIMEOUT_DEFAULT   10UL
#define BOARD_TEM_ALARM_DEFAULT  40.0f
#define SENSOR_TEM_ALARM_DEFAULT 45.0f
#define SENSOR_HUM_ALARM_DEFAULT 95.0f
#define ALARM_MODE_DEFAULT       1U

uint32_t resetCounter = 0;
void eepromSetup(void);


//----------------------------------- WIFI SETUP ---------------------------------
AsyncWebServer server(80);
// Search for parameter in HTTP POST request
const char* PARAM_INPUT_1 = "ssid";
const char* PARAM_INPUT_2 = "pass";
const char* PARAM_INPUT_3 = "ip";
const char* PARAM_INPUT_4 = "gateway";
//Variables to save values from HTML form
String ssid;
String pass;
String ip;
String gateway;
// File paths to save input values permanently
const char* ssidPath = "/ssid.txt";
const char* passPath = "/pass.txt";
const char* ipPath = "/ip.txt";
const char* gatewayPath = "/gateway.txt";
IPAddress localIP;
//IPAddress localIP(192, 168, 1, 200); // hardcoded
// Set your Gateway IP address
IPAddress localGateway;
//IPAddress localGateway(192, 168, 1, 1); //hardcoded
IPAddress subnet(255, 255, 0, 0);

void initSPIFFS();
String readFile(fs::FS &fs, const char * path);
void writeFile(fs::FS &fs, const char * path, const char * message);
bool initWiFi();
void getWiFiInfo();



//-------------------------------- MQTT SETUP ---------------------------------
#define MQTT_SERVER "meigroup.ddns.net"
#define MQTT_USER "meigroup"
#define MQTT_PASSWORD "@778899"
#define PORT 1883
WiFiClient espClient;
PubSubClient client(espClient);

uint16_t randomCheck = 100;
uint32_t randomCheckTime;
uint32_t randomCheckTimeoutMS;
uint32_t sensorUpdateTimeoutMS;
uint32_t sensorUpdateTime;
uint32_t randomCheckTimeoutS = 10;
uint32_t sensorUpdateTimeoutS = 30;

#define MQTT_UPDATE_TIME_MS 30000

void callback (char* topic, byte* payload, unsigned int length);
void reconnect (void);
void sendDataMQTT(void);
void randomMQTT(void);
void sendSetupMQTT(void);
bool isValidData(String str);



// ------------------------------ WDT SETUP -------------------------
uint32_t RUN_WATCH_DOG = 0;
#define WATCHDOG_TIMEOUT_S 180 // 3 minute
hw_timer_t * watchDogTimer = NULL;

void IRAM_ATTR watchDogInterrupt(void);
void watchDogRefresh(void);



// ------------------------------ TIME SETUP ----------------------------------
#define NOW_TIME millis()
#define WAIT_WIFI_CONNECT_S 60

