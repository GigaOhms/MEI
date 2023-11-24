#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP_EEPROM.h>
#include <Esp.h>


WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

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

#define EEPROM_SIZE 16

volatile uint8_t wdt_count = 0;

// void ICACHE_RAM_ATTR ISR_WDT() {
//   wdt_count++;
//   if (wdt_count == 5) {
//     Serial.println("RESET!");
//     ESP.restart();
//   }
//   timer1_write(5000000);
// }

uint8_t SET_HOURS_OFF_RL1 = 10;
uint8_t SET_MINUTE_OFF_RL1 = 23;
uint8_t SET_HOURS_ON_RL1 = 10;
uint8_t SET_MINUTE_ON_RL1 = 21;

uint8_t SET_HOURS_OFF_RL2 = 10;
uint8_t SET_MINUTE_OFF_RL2 = 23;
uint8_t SET_HOURS_ON_RL2 = 10;
uint8_t SET_MINUTE_ON_RL2 = 21;

uint8_t SET_HOURS_OFF_RL3 = 10;
uint8_t SET_MINUTE_OFF_RL3 = 23;
uint8_t SET_HOURS_ON_RL3 = 10;
uint8_t SET_MINUTE_ON_RL3 = 21;

uint8_t SET_HOURS_OFF_RL4 = 10;
uint8_t SET_MINUTE_OFF_RL4 = 23;
uint8_t SET_HOURS_ON_RL4 = 10;
uint8_t SET_MINUTE_ON_RL4 = 21;

uint8_t HOURS_MQTT = 0;
uint8_t MINUTE_MQTT = 0;

uint8_t address = 0;
uint8_t SIZE_VAR = (uint8_t)sizeof(SET_HOURS_OFF_RL1);

WiFiClient espClient;
PubSubClient client(espClient);
uint16_t lastMsg = 0;
#define MSG_BUFFER_SIZE 80
char msg[MSG_BUFFER_SIZE];
int value = 0;
String STT_RL = "";
String temp = "";


uint8_t DAY, HOURS, MINUTE;
bool S_TIME = 0;

#define RXD D6
#define TXD D5

#define NOW_TIME millis()

SoftwareSerial mySerial(RXD, TXD);

float TEMPERATURE = 0.0;
uint16_t DISTANCE = 0;
float OXIGEN = 0.0;
float PH_VAL = 0.0;

float VOLTAGE = 0.0;
float CURRENT = 0.0;
float POWER = 0.0;
float ENERGY = 0.0;
float FREQUENCE = 0.0;
float PF = 0.0;

uint32_t PRE_TIME;
uint32_t PRE_TIME_MQTT;
uint8_t count = 0;
String s = "";
bool READ_WATER = 0;
bool READ_RL = 0;
bool READ_ELEC = 0;

bool S_RL8, S_RL7, S_RL6, S_RL5;
bool SS_RL;

// ------------ FUNCTION -------------------------

void callback(char* topic, byte* payload, unsigned int length);
void reconnect();
void GET_TIME(void);
void CHECK_TIME(void);
void READ_SENSOR();
void SEND_MQTT();
void setup_wifi();



void setup() {
  mySerial.begin(9600);
  Serial.begin(115200);

  // timer1_attachInterrupt(ISR_WDT);
  // timer1_enable(TIM_DIV16, TIM_EDGE, TIM_SINGLE);
  // timer1_write(WDT*1000000);  // 5000000 / 5 ticks per us from TIM_DIV16 == 1,000,000 us interval

  ESP.wdtDisable();
  ESP.wdtEnable(4000);

  setup_wifi();
  client.setServer(MQTT_SERVER, PORT);
  client.setCallback(callback);
  reconnect();

  EEPROM.begin(EEPROM_SIZE);
  EEPROM.get(0, SET_HOURS_OFF_RL1);
  EEPROM.get(1, SET_MINUTE_OFF_RL1);
  EEPROM.get(2, SET_HOURS_ON_RL1);
  EEPROM.get(3, SET_MINUTE_ON_RL1);
  EEPROM.get(4, SET_HOURS_OFF_RL2);
  EEPROM.get(5, SET_MINUTE_OFF_RL2);
  EEPROM.get(6, SET_HOURS_ON_RL2);
  EEPROM.get(7, SET_MINUTE_ON_RL2);
  EEPROM.get(8, SET_HOURS_OFF_RL3);
  EEPROM.get(9, SET_MINUTE_OFF_RL3);
  EEPROM.get(10, SET_HOURS_ON_RL3);
  EEPROM.get(11, SET_MINUTE_ON_RL3);
  EEPROM.get(12, SET_HOURS_OFF_RL4);
  EEPROM.get(13, SET_MINUTE_OFF_RL4);
  EEPROM.get(14, SET_HOURS_ON_RL4);
  EEPROM.get(15, SET_MINUTE_ON_RL4);

  Serial.println("READY!");

  wdt_count = 0;
  PRE_TIME = NOW_TIME;
  PRE_TIME_MQTT = NOW_TIME;
}

void loop() {
  if (WiFi.status() != WL_CONNECTED)
    setup_wifi();

  if (!client.connected())
    reconnect();
  client.loop();

  ESP.wdtFeed();

  READ_SENSOR();

  if ((uint32_t)(NOW_TIME - PRE_TIME_MQTT) >= 5000) {
    PRE_TIME_MQTT = NOW_TIME;
    SEND_MQTT();
  }

  if ((uint32_t)(NOW_TIME - PRE_TIME) >= 50000) {
    PRE_TIME = NOW_TIME;
    GET_TIME();
  }
}


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  if (length == 1) {
    STT_RL += (char)payload[0];
    SS_RL = (bool)(STT_RL.toInt());
    Serial.println((char)payload[0]);  //23456
  } else {
    for (int i = 0; i < length; i++) {
      STT_RL += (char)payload[i];
      Serial.print((char)payload[i]);  //23456
    }
    Serial.println();
    int j = STT_RL.indexOf(':');
    HOURS_MQTT = (STT_RL.substring(0, j)).toInt();
    MINUTE_MQTT = (STT_RL.substring(j + 1, length)).toInt();
  }

  if (strcmp(topic, "HOCA/RL1") == 0) {
    if (SS_RL == 1)
      mySerial.print("11*");
    else mySerial.print("10*");
  } else if (strcmp(topic, "HOCA/RL2") == 0) {
    if (SS_RL == 1)
      mySerial.print("21*");
    else mySerial.print("20*");
  } else if (strcmp(topic, "HOCA/RL3") == 0) {
    if (SS_RL == 1)
      mySerial.print("31*");
    else mySerial.print("30*");
  } else if (strcmp(topic, "HOCA/RL4") == 0) {
    if (SS_RL == 1)
      mySerial.print("41*");
    else mySerial.print("40*");
  } else if (strcmp(topic, "HOCA/HG/RL1_ON") == 0) {  /// --------------
    SET_HOURS_ON_RL1 = HOURS_MQTT;
    SET_MINUTE_ON_RL1 = MINUTE_MQTT;
    EEPROM.put(2, SET_HOURS_ON_RL1);
    EEPROM.put(3, SET_MINUTE_ON_RL1);
    EEPROM.commit();
  } else if (strcmp(topic, "HOCA/HG/RL1_OFF") == 0) {  /// --------------
    SET_HOURS_OFF_RL1 = HOURS_MQTT;
    SET_MINUTE_OFF_RL1 = MINUTE_MQTT;
    EEPROM.put(0, SET_HOURS_OFF_RL1);
    EEPROM.put(1, SET_MINUTE_OFF_RL1);
    EEPROM.commit();
  } else if (strcmp(topic, "HOCA/HG/RL2_ON") == 0) {  /// --------------
    SET_HOURS_ON_RL2 = HOURS_MQTT;
    SET_MINUTE_ON_RL2 = MINUTE_MQTT;
    EEPROM.put(6, SET_HOURS_ON_RL2);
    EEPROM.put(7, SET_MINUTE_ON_RL2);
    EEPROM.commit();
  } else if (strcmp(topic, "HOCA/HG/RL2_OFF") == 0) {  /// --------------
    SET_HOURS_OFF_RL2 = HOURS_MQTT;
    SET_MINUTE_OFF_RL2 = MINUTE_MQTT;
    EEPROM.put(4, SET_HOURS_OFF_RL2);
    EEPROM.put(5, SET_MINUTE_OFF_RL2);
    EEPROM.commit();
  } else if (strcmp(topic, "HOCA/HG/RL3_ON") == 0) {  /// --------------
    SET_HOURS_ON_RL3 = HOURS_MQTT;
    SET_MINUTE_ON_RL3 = MINUTE_MQTT;
    EEPROM.put(10, SET_HOURS_ON_RL3);
    EEPROM.put(11, SET_MINUTE_ON_RL3);
    EEPROM.commit();
  } else if (strcmp(topic, "HOCA/HG/RL3_OFF") == 0) {  /// --------------
    SET_HOURS_OFF_RL3 = HOURS_MQTT;
    SET_MINUTE_OFF_RL3 = MINUTE_MQTT;
    EEPROM.put(8, SET_HOURS_OFF_RL3);
    EEPROM.put(9, SET_MINUTE_OFF_RL3);
    EEPROM.commit();
  } else if (strcmp(topic, "HOCA/HG/RL4_ON") == 0) {  /// --------------
    SET_HOURS_ON_RL4 = HOURS_MQTT;
    SET_MINUTE_ON_RL4 = MINUTE_MQTT;
    EEPROM.put(14, SET_HOURS_ON_RL4);
    EEPROM.put(15, SET_MINUTE_ON_RL4);
    EEPROM.commit();
  } else if (strcmp(topic, "HOCA/HG/RL4_OFF") == 0) {  /// --------------
    SET_HOURS_OFF_RL4 = HOURS_MQTT;
    SET_MINUTE_OFF_RL4 = MINUTE_MQTT;
    EEPROM.put(12, SET_HOURS_OFF_RL4);
    EEPROM.put(13, SET_MINUTE_OFF_RL4);
    EEPROM.commit();
  }
  STT_RL = "";
}

void reconnect() {
  uint8_t countWifi = 0;
  // timer1_disable();
  ESP.wdtFeed();
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("connected");
      client.subscribe("HOCA/RL1");
      client.subscribe("HOCA/RL2");
      client.subscribe("HOCA/RL3");
      client.subscribe("HOCA/RL4");
      client.subscribe("HOCA/HG/RL1_ON");
      client.subscribe("HOCA/HG/RL1_OFF");
      client.subscribe("HOCA/HG/RL2_ON");
      client.subscribe("HOCA/HG/RL2_OFF");
      client.subscribe("HOCA/HG/RL3_ON");
      client.subscribe("HOCA/HG/RL3_OFF");
      client.subscribe("HOCA/HG/RL4_ON");
      client.subscribe("HOCA/HG/RL4_OFF");
    } else {
      ESP.wdtFeed();
      countWifi++;
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.print(" try again ");
      Serial.print(countWifi);
      Serial.print(" time - wdt = ");
      Serial.println(wdt_count);
      delay(2000);
      if (countWifi >= 150) {
        // Serial.println("deep sleep 1 hours");
        // ESP.deepSleep(3600e6);  // uS
        ESP.restart();
      }
    }
  }
}


void GET_TIME(void) {
  timeClient.update();
  HOURS = timeClient.getHours() + 7;
  if (HOURS >= 24)
    HOURS = HOURS - 24;
  MINUTE = timeClient.getMinutes();

  CHECK_TIME();
}

void CHECK_TIME(void) {
  if (HOURS == SET_HOURS_OFF_RL1)
    if (MINUTE == SET_MINUTE_OFF_RL1)
      mySerial.print("10*");
  if (HOURS == SET_HOURS_ON_RL1)
    if (MINUTE == SET_MINUTE_ON_RL1)
      mySerial.print("11*");

  if (HOURS == SET_HOURS_OFF_RL2)
    if (MINUTE == SET_MINUTE_OFF_RL2)
      mySerial.print("20*");
  if (HOURS == SET_HOURS_ON_RL2)
    if (MINUTE == SET_MINUTE_ON_RL2)
      mySerial.print("21*");

  if (HOURS == SET_HOURS_OFF_RL3)
    if (MINUTE == SET_MINUTE_OFF_RL3)
      mySerial.print("30*");
  if (HOURS == SET_HOURS_ON_RL3)
    if (MINUTE == SET_MINUTE_ON_RL3)
      mySerial.print("31*");

  if (HOURS == SET_HOURS_OFF_RL4)
    if (MINUTE == SET_MINUTE_OFF_RL4)
      mySerial.print("40*");
  if (HOURS == SET_HOURS_ON_RL4)
    if (MINUTE == SET_MINUTE_ON_RL4)
      mySerial.print("41*");
}

void READ_SENSOR() {
  if (mySerial.available()) {
    char c = mySerial.read();
    if (c == 'a' || c == 'c') {
      READ_WATER = 1;
      if (c == 'c')
        READ_RL = 1;
    } else if (READ_WATER == 1) {
      if (c != '/' && c != '*') s += c;
      else if (c == '/' || c == '*') {
        count++;
        // else if (count == 2)
        //   DISTANCE = s.toInt();
        // else if (count == 3)
        //   OXIGEN = s.toFloat();
        // else if (count == 4)
        //   PH_VAL = s.toFloat();
        if (count == 1)
          S_RL8 = (bool)(s.toInt());
        else if (count == 2)
          S_RL7 = (bool)(s.toInt());
        else if (count == 3)
          S_RL6 = (bool)(s.toInt());
        else if (count == 4 || c == '*') {
          S_RL5 = (bool)(s.toInt());
          count = 0;
          READ_WATER = 0;
          if (READ_RL) {
            SEND_MQTT();
            READ_RL = 0;
          }
        }
        s = "";
      }
    }

    else if (c == 'b') READ_ELEC = 1;
    else if (READ_ELEC == 1) {
      if (c != '/' && c != '*') s += c;
      else if (c == '/' || c == '*') {
        count++;
        if (count == 1)
          TEMPERATURE = s.toFloat();
        else if (count == 2)
          VOLTAGE = s.toFloat();
        else if (count == 3)
          CURRENT = s.toFloat();
        else if (count == 4)
          POWER = s.toFloat();
        else if (count == 5)
          ENERGY = s.toFloat();
        else if (count == 6)
          FREQUENCE = s.toFloat();
        else if (count == 7)
          PF = s.toFloat();
        else if (count == 8 || c == '*') {
          PH_VAL = s.toFloat();
          count = 0;
          READ_ELEC = 0;
        }
        s = "";
      }
    }
  }
}

void SEND_MQTT() {
  snprintf(msg, MSG_BUFFER_SIZE, "%d%d%d%d_%.2f/%.2f/%.2f/%.2f/%.2f/%.2f/%.2f/%.2f", S_RL8, S_RL7, S_RL6, S_RL5, TEMPERATURE, VOLTAGE, CURRENT, POWER, ENERGY, FREQUENCE, PF, PH_VAL);
  client.publish("HOCA/STT", msg);
}

void setup_wifi() {
  uint16_t countSetup = 0;
  ESP.wdtFeed();
  delay(100);
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    ESP.wdtFeed();
    // wdt_count = 0;
    countSetup++;
    Serial.print(".");
    delay(300);
    if (countSetup >= 1000) {
      ESP.restart();
      // Serial.println("deep sleep 1 hours !!");
      // ESP.deepSleep(3600e6);  // uS
    }
  }
  Serial.print("\nConnected with IP: ");
  Serial.println(WiFi.localIP());
}
