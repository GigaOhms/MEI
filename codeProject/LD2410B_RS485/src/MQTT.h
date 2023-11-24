#include <WiFi.h>
#include <PubSubClient.h>

// ----------------------------------- DEVICE CONFIG ----------------------------
// FORMAT [DeviceID/Vitri/loaiThietBi/cheDoCuaThietBi (on/off)/NguoiChuyenDong (trang thai cam bien)/trangThaiThietBi (online/offline)]
#define DEVICE_ID           "B01"    
#define DEVICE_LOCATION     "001"
#define DEVICE_TYPE         "02"
// GREENDATA2 -- TEST
#define DATA_TOPIC          "GREENDATA11"                              // -- DATA FORMAT []
#define SUB_TOPIC_REQUEST   "GREENDATA11/CONTROLS"                     // -- Gui truy van data [all character]
#define SUB_TOPIC_MODE      "GREENDATA11/MODE"                         // -- cai dat che do [0: relay on board -- 1: relay RS485]
#define SUB_TOPIC_TIME      "GREENDATA11/TIME"                         // -- cai dat thoi gian tat khi khong co nguoi (s)
#define SUB_TOPIC_CONTROL   "GREENDATA11/CONTROL"                      // -- cai dat khoa thiet bi [0: khoa -- 1: mo khoa]
#define SUB_TOPIC_LOCK      "GREENDATA11/LOCK"                         // -- cai dat khoa thiet bi [0: khoa -- 1: mo khoa]


// //----------------------------------- WIFI SETUP ---------------------------------
#define WIFI_SSID "SERVER"
#define WIFI_PASSWORD "meigroupiot77"
// // #define WIFI_SSID "MEIGROUP01 - 2.4GHZ"
// // #define WIFI_PASSWORD "77namhai88"
// // #define WIFI_SSID "Serena"
// // #define WIFI_PASSWORD "12345678"

// void setup_wifi(void);


//-------------------------------- MQTT SETUP ---------------------------------
#define MQTT_SERVER "meigroup.ddns.net"
#define MQTT_USER "meigroup"
#define MQTT_PASSWORD "@778899"
#define PORT 1883
WiFiClient espClient;
PubSubClient client(espClient);

uint32_t sensorUpdateTimeoutMS = 10000;
uint32_t sensorUpdateTime;

void callback (char* topic, byte* payload, unsigned int length);
void reconnect (void);
void sendDataMQTT(void);



// Initialize WiFi
void setup_wifi(void) {
  uint16_t wifiCounter = 0;
  delay(100);
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  //WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
    wifiCounter++; 
    if (wifiCounter >= 150)
      ESP.restart();
  }
  Serial.print("\nConnected with IP: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String s = "";
  for (int i = 0; i < length; i++) {
    s += (char)payload[i];
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if (strcmp(topic, SUB_TOPIC_REQUEST) == 0) { // Correct topic
    sendDataMQTT();
  } else if (strcmp(topic, SUB_TOPIC_MODE) == 0) {
    int j = s.indexOf('/');
    String id = s.substring(0, j);
    if (id == DEVICE_ID) {
      modeControl = (uint8_t)((s.substring(j + 1, length)).toInt());
      EEPROM.write(ADDR_MODE, modeControl);
      EEPROM.commit();
    }
  } else if (strcmp(topic, SUB_TOPIC_TIME) == 0) {
    int j = s.indexOf('/');
    String id = s.substring(0, j);
    if (id == DEVICE_ID) {
      uint8_t tempCounter = (uint8_t)((s.substring(j + 1, length)).toInt());
      if (tempCounter >= 2 && tempCounter <= 40) {
        controlOffTimeoutM = tempCounter;
        EEPROM.write(ADDR_TIME, controlOffTimeoutM);
        EEPROM.commit();
      }
    }
  } else if (strcmp(topic, SUB_TOPIC_CONTROL) == 0) {
    int j, k;
    j = s.indexOf('/');
    String id = s.substring(0, j);
    if (id == DEVICE_ID) {
      k = s.indexOf('/', j + 1);
      autoMode = (uint8_t)((s.substring(j + 1, k)).toInt());
      if (autoMode > 1) autoMode = 1;
      EEPROM.write(ADDR_AUTO, autoMode);
      EEPROM.commit();
      relayControl();
      sendDataMQTT();
    }
  } else if (strcmp(topic, SUB_TOPIC_LOCK) == 0) {
    int j = s.indexOf('/');
    String id = s.substring(0, j);
    if (id == DEVICE_ID) {
      lockDevice = (uint8_t)((s.substring(j + 1, length)).toInt());
      if (lockDevice > 1) lockDevice = 0;
      EEPROM.write(ADDR_LOCK, lockDevice);
      EEPROM.commit();
    }
  }
}


void reconnect() {
  uint16_t counter = 0;
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("connected");
      client.subscribe(SUB_TOPIC_REQUEST);
      client.subscribe(SUB_TOPIC_MODE);
      client.subscribe(SUB_TOPIC_TIME);
      client.subscribe(SUB_TOPIC_CONTROL);
      client.subscribe(SUB_TOPIC_LOCK);
    } else {
      Serial.print("failed, rc = ");
      Serial.print(client.state());
      Serial.println(" | try again 2s");
      // delay(2000);

      // ------------------------------------------------ MODIFIER -------------------------------------
      watchDogRefresh();
      counter++;
      if (counter >= 300)
        ESP.restart();
      uint32_t tempTime = millis();
      while ((uint32_t)(millis() - tempTime) <= 2000) {
        checkStatusRL();
        manualButton();
        if (modeControl == 1)
          rs485_receive();
      }
    }
  }
}

void sendDataMQTT(void) {
  // FORMAT [DeviceID/Vitri/loaiThietBi/cheDoCuaThietBi (on/off)/NguoiChuyenDong (trang thai cam bien)/trangThaiThietBi (khoa/auto)]
  String sendString = String(DEVICE_ID) + "/" + String(DEVICE_LOCATION) + "/" + String(DEVICE_TYPE) + "/";
  if (modeControl == 0) sendString += String(S_RL);
  else sendString += String(S_RL_CLI);
  sendString += ("/" + String(S_SENSOR) + "/" + String(autoMode));
  client.publish(DATA_TOPIC, sendString.c_str());
  delay(10);
}