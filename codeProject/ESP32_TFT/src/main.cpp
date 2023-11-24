#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include "bitmap.h"


// ------------------------------ INTERNET SETUP -------------------------------
// #define WIFI_SSID "SERVER"
// #define WIFI_PASSWORD "meigroupiot77"
#define WIFI_SSID "MEIGROUP_DC2.4"
#define WIFI_PASSWORD "meigroupiot88"
// #define WIFI_SSID "Serena"
// #define WIFI_PASSWORD "12345678"

// #define MQTT_SERVER "broker.mqttdashboard.com"
// // https://www.hivemq.com/demos/websocket-client/
// #define MQTT_USER "trinh"
// #define MQTT_PASSWORD "trinhcong"
#define MQTT_SERVER "meigroup.ddns.net"
#define MQTT_USER "meigroup"
#define MQTT_PASSWORD "@778899"
#define PORT 1883

WiFiClient espClient;
PubSubClient client(espClient);


// ------------------------------ WDT SETUP -------------------------------
uint32_t RUN_WATCH_DOG = 0;
#define WATCHDOG_TIMEOUT_S 180 // 3 minute
hw_timer_t * watchDogTimer = NULL;


// ------------------------------ TFT SETUP -------------------------------
#undef TFT_MOSI
#undef TFT_CLK
#undef TFT_RST
#undef TFT_MISO
#undef TFT_DC
#undef TFT_CS

#define TFT_MOSI 23
#define TFT_CLK 18
#define TFT_RST 4
#define TFT_MISO 19
#define TFT_DC 2
#define TFT_CS 5
TFT_eSPI tft = TFT_eSPI();

#define NOW_TIME millis()
uint32_t PRE_TIME;


// ------------------------------ STATUS SETUP -------------------------------
bool LED_MEI = 0;
bool BOM_OXI = 0;
bool LED_HOCA = 0;
bool BOM_LOC = 0;

bool sLED_MEI = 0;
bool sBOM_OXI = 0;
bool sLED_HOCA = 0;
bool sBOM_LOC = 0;

float TEMPERATURE = 0.0;

float VOLTAGE = 0.0;
float CURRENT = 0.0;
float POWER = 0.0;
float ENERGY = 0.0;
float FREQUENCE = 0.0;
float PF = 0.0;

float PH_VAL = 0.0;

void setup_wifi (void);
void callback (char* topic, byte* payload, unsigned int length);
void reconnect (void);
void displayLOGO(void);
void setupTFT(void);
void display_sensor_tft(void);
void display_stt_tft (void);

void IRAM_ATTR watchDogInterrupt(void);
void watchDogRefresh(void);

void setup() {
  Serial.begin(115200);
  watchDogTimer = timerBegin(2, 80, true);
  timerAttachInterrupt(watchDogTimer, &watchDogInterrupt, true);
  timerAlarmWrite(watchDogTimer, WATCHDOG_TIMEOUT_S * 1000000, false);
  timerAlarmEnable(watchDogTimer);

  displayLOGO();

  setup_wifi();
  client.setServer(MQTT_SERVER, PORT);
  client.setCallback(callback);
  reconnect();

  setupTFT();

  display_stt_tft();
  display_sensor_tft();

  PRE_TIME = NOW_TIME;
  RUN_WATCH_DOG = NOW_TIME;
}

void loop() {
  if (!client.connected())
    reconnect();
  client.loop();

  if ((uint32_t)(NOW_TIME - PRE_TIME) >= 5000) {
    PRE_TIME = NOW_TIME;
    display_sensor_tft();
  }
  if ((uint32_t)(NOW_TIME - RUN_WATCH_DOG) >= 5000) {
    RUN_WATCH_DOG = NOW_TIME;
    watchDogRefresh();
  }
}

void IRAM_ATTR watchDogInterrupt(void) {
  Serial.println("REBOOT !!!");
  ESP.restart();
}

void watchDogRefresh(void){
  timerWrite(watchDogTimer, 0);                    //reset thời gian của timer 
}

void display_sensor_tft(void){
  tft.setTextSize(3);
  // ------------------------- TEMPERATURE ---------------------- COLUME 1 ----
  tft.setCursor(10, 14);
  tft.print(String(TEMPERATURE, 2));
  tft.setCursor(110, 14);
  tft.print(char(247));
  tft.setCursor(130, 14);
  tft.print("C");

  // ------------------------- VOLTAGE ------------------------- COLUME 2 ----
  tft.setCursor(170, 14);
  tft.print(String(PH_VAL, 2));
  tft.setCursor(270, 14);
  tft.print("pH");
  // ------------------------- CURRENT -------------------------
  tft.setCursor(170, 62);
  tft.print(String(VOLTAGE, 1));
  tft.setCursor(270, 62);
  tft.print("V");
  // ------------------------- POWER -------------------------
  tft.setCursor(170, 110);
  tft.print(String(CURRENT, 2));
  tft.setCursor(270, 110);
  tft.print("A");
  // ------------------------- FREQUENCE -------------------------
  tft.setCursor(170, 158);
  tft.print(String(POWER, 1));
  tft.setCursor(270, 158);
  tft.print("W");
  // ------------------------- POWER FACTOR -------------------------
  tft.setCursor(170, 206);
  tft.print(String(ENERGY, 1));
  tft.setCursor(270, 206);
  tft.print("kWh");
}

void display_stt_tft(void){
  tft.setTextSize(3);
  // ------------------------- STATUS ---------------------- COLUME 1 ----
  tft.setCursor(10, 62);
  tft.print("MEI");
  tft.setCursor(100, 62);
  if (LED_MEI)
    tft.print("ON ");
  else tft.print("OFF");

  tft.setCursor(10, 110);
  tft.print("OXI");
  tft.setCursor(100, 110);
  if (BOM_OXI)
    tft.print("ON ");
  else tft.print("OFF");

  tft.setCursor(10, 158);
  tft.print("LED");
  tft.setCursor(100, 158);
  if (LED_HOCA)
    tft.print("ON ");
  else tft.print("OFF");

  tft.setCursor(10, 206);
  tft.print("LOC");
  tft.setCursor(100, 206);
  if (BOM_LOC)
    tft.print("ON ");
  else tft.print("OFF");
}

bool check_stt(void){
  bool chk = 0;
  if (sLED_MEI != LED_MEI){
    sLED_MEI = LED_MEI;
    chk = 1;
  }
  if (sBOM_OXI != BOM_OXI){
    sBOM_OXI = BOM_OXI;
    chk = 1;
  }
  if (sLED_HOCA != LED_HOCA){
    sLED_HOCA = LED_HOCA;
    chk = 1;
  }
  if (sBOM_LOC != BOM_LOC){
    sBOM_LOC = BOM_LOC;
    chk = 1;
  }
  return chk;
}

void setup_wifi(void) {
  delay(100);
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  //WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.print("\nConnected with IP: ");
  Serial.println(WiFi.localIP());
}

// RECIVE   "%d%d%d%d_%.2f/%.2f/%.2f/%.2f/%.2f/%.2f/%.2f", S_RL8, S_RL7, S_RL6, S_RL5, TEMPERATURE, PH, VOLTAGE, CURRENT, POWER, ENERGY, FREQUENCE, PF);

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String s = "";

  for (int i = 0; i < length; i++) {
    s += (char)payload[i];
    Serial.print((char)payload[i]);  //23456
  }
  Serial.println();

  int j, k;
  String ss;

  if (strcmp(topic, "HOCA/STT") == 0) {
    LED_MEI = (bool)(s.substring(0, 1).toInt());
    BOM_OXI = (bool)(s.substring(1, 2).toInt());
    LED_HOCA = (bool)(s.substring(2, 3).toInt());
    BOM_LOC = (bool)(s.substring(3, 4).toInt());

    if (check_stt())
      display_stt_tft();
    
    j = s.indexOf('_') + 1;
    k = s.indexOf('/');
    ss = s.substring(j, k);
    TEMPERATURE = ss.toFloat();

    j = k + 1;
    k = s.indexOf('/', j);
    ss = s.substring(j, k);
    VOLTAGE = ss.toFloat();

    j = k + 1;
    k = s.indexOf('/', j);
    ss = s.substring(j, k);
    CURRENT = ss.toFloat();

    j = k + 1;
    k = s.indexOf('/', j);
    ss = s.substring(j, k);
    POWER = ss.toFloat();

    j = k + 1;
    k = s.indexOf('/', j);
    ss = s.substring(j, k);
    ENERGY = ss.toFloat();

    j = k + 1;
    k = s.indexOf('/', j);
    ss = s.substring(j, k);
    FREQUENCE = ss.toFloat();

    j = k + 1;
    k = s.indexOf('/', j);
    ss = s.substring(j, k);
    PF = ss.toFloat();
    
    j = k + 1;
    k = s.indexOf('/', j);
    ss = s.substring(j, length);
    PH_VAL = ss.toFloat();
  } else if (strcmp(topic, "HOCA/STT/SETUPTFT") == 0) {
    setupTFT();
  }
}

void reconnect() {
  displayLOGO();
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("connected");
      client.subscribe("HOCA/STT");
      client.subscribe("HOCA/STT/SETUPTFT");
    } else {
      Serial.print("failed, rc = ");
      Serial.print(client.state());
      Serial.println(" | try again 2s");
      delay(2000);
    }
  }
  setupTFT();
}

void setupTFT(void) {
  tft.begin();
  tft.setRotation(1);

  tft.fillScreen(TFT_BLACK);
  tft.drawRoundRect(0, 0, 159, 48, 4, TFT_WHITE);
  tft.drawRoundRect(160, 0, 159, 48, 4, TFT_WHITE);

  tft.drawRoundRect(0, 48, 159, 48, 4, TFT_WHITE);
  tft.drawRoundRect(160, 48, 159, 48, 4, TFT_WHITE);

  tft.drawRoundRect(0, 96, 159, 48, 4, TFT_WHITE);
  tft.drawRoundRect(160, 96, 159, 48, 4, TFT_WHITE);

  tft.drawRoundRect(0, 144, 159, 48, 4, TFT_WHITE);
  tft.drawRoundRect(160, 144, 159, 48, 4, TFT_WHITE);

  tft.drawRoundRect(0, 192, 159, 48, 4, TFT_WHITE);
  tft.drawRoundRect(160, 192, 159, 48, 4, TFT_WHITE);

  display_stt_tft();
  display_sensor_tft();
}

void displayLOGO(void) {
  tft.begin();
  tft.setRotation(1);
  tft.setSwapBytes(true); // Swap the byte order for pushImage() - corrects endianness
  tft.fillScreen(TFT_WHITE);
  tft.pushImage(0, 0, 320, 240, mercy);
}