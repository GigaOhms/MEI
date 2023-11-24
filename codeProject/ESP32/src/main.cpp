#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>


// ------------------------------ INTERNET SETUP -------------------------------
#define WIFI_SSID "MEIGROUP01 - 2.4GHZ"
#define WIFI_PASSWORD "77namhai88"
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

// #define MQTT_SERVER "ngoinhaiot.com"
// #define MQTT_USER "ctrinh01028485"
// #define MQTT_PASSWORD "368C8E7871A644CD"
// #define PORT 1111

WiFiClient espClient;
PubSubClient client(espClient);

#define MSG_BUFFER_SIZE 80
char msg[MSG_BUFFER_SIZE];


#define WIFI_TIME_OUT_S 180   // Sleep after 3 minute if not connect Wifi

// ------------------------------ DEEP SLEEP SETUP -------------------------------
#define uS_TO_S_FACTOR 1000000ULL  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  3600        /* Time ESP32 will go to sleep (in seconds) */


// ------------------------------ WDT SETUP -------------------------------
uint32_t RUN_WATCH_DOG = 0;
#define WATCHDOG_TIMEOUT_S 180 // 3 minute
hw_timer_t * watchDogTimer = NULL;

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
float FREQUENCE = 0.0;
float PF = 0.0;

void setup_wifi (void);
void callback (char* topic, byte* payload, unsigned int length);
void reconnect (void);
void SEND_MQTT(void);
void READ_SERIAL(void);

void IRAM_ATTR watchDogInterrupt(void);
void watchDogRefresh(void);

void setup() {
  Serial.begin(115200);
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR); // DEEP SLEEP SETUP
  watchDogTimer = timerBegin(2, 80, true);
  timerAttachInterrupt(watchDogTimer, &watchDogInterrupt, true);
  timerAlarmWrite(watchDogTimer, WATCHDOG_TIMEOUT_S * 1000000, false);
  timerAlarmEnable(watchDogTimer);

  setup_wifi();
  client.setServer(MQTT_SERVER, PORT);
  client.setCallback(callback);
  reconnect();

  PRE_TIME = NOW_TIME;
  RUN_WATCH_DOG = NOW_TIME;
}

void loop() {
  if (!client.connected())
    reconnect();
  client.loop();

  // if ((uint32_t)(NOW_TIME - PRE_TIME) >= 5000) {
  //   PRE_TIME = NOW_TIME;
  //   void SEND_MQTT();
  // }

  READ_SERIAL();

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

void SEND_MQTT(void) {
  snprintf(msg, MSG_BUFFER_SIZE, "%d%d%d%d_%.2f/%.2f/%.2f/%.2f/%.2f/%.2f", LED_MEI, BOM_OXI, LED_HOCA, BOM_LOC, TEMPERATURE, VOLTAGE, CURRENT, POWER, FREQUENCE, PF);
  client.publish("HOCA/STT", msg);
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
  uint16_t countSetup = 0;
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);

    watchDogRefresh();
    countSetup++;
    if (countSetup >= WIFI_TIME_OUT_S * 2){    // Go to sleep after 5 minute (5 * 30)
      Serial.println("Going to sleep now");
      esp_deep_sleep_start();
    }
  }
  Serial.print("\nConnected with IP: ");
  Serial.println(WiFi.localIP());
}

// RECIVE   "%d%d%d%d_%.2f/%.2f/%.2f/%.2f/%.2f/%.2f/%.2f", S_RL8, S_RL7, S_RL6, S_RL5, TEMPERATURE, VOLTAGE, CURRENT, POWER, ENERGY, FREQUENCE, PF);

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

    // if (check_stt())
    
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
    j = k + 1;
    k = s.indexOf('/', j);
    ss = s.substring(j, k);
    FREQUENCE = ss.toFloat();

    j = k + 1;
    k = s.indexOf('/', j);
    ss = s.substring(j, length);
    PF = ss.toFloat();
  }
}

void reconnect() {
  uint16_t countSetup = 0;
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("connected");
      client.subscribe("HOCA/STT");
    } else {
      Serial.print("failed, rc = ");
      Serial.print(client.state());
      Serial.println(" | try again 2s");
      delay(2000);

      watchDogRefresh();
      countSetup = countSetup + 2;
      if (countSetup >= WIFI_TIME_OUT_S){    // Go to sleep after 5 minute (5 * 30)
        Serial.println("Going to sleep now");
        esp_deep_sleep_start();
      }
    }
  }
}

String s = "", ss = "";
uint8_t j, k;

void READ_SERIAL(void){
  if (Serial.available()){
    char c = Serial.read();
    if (c != '\n')
      s += c;
    else {
      j = s.indexOf(' ');
      k = s.indexOf('\n');
      ss = s.substring(0, j);

      if (ss.equals("HOCA/RL3")){
        ss = s.substring(j + 1, k);
        msg[0] = ss[0];
        msg[1] = '\0';
        Serial.print(msg);
        Serial.print("heo");
        Serial.println();
        // snprintf(msg, MSG_BUFFER_SIZE, "%d%d%d%d_%.2f/%.2f/%.2f/%.2f/%.2f/%.2f/%.2f", S_RL8, S_RL7, S_RL6, S_RL5, TEMPERATURE, VOLTAGE, CURRENT, POWER, FREQUENCE, PF);
        // client.publish("HOCA/STT", msg);
      }
      s = "";
      ss = "";
    }
  }

}