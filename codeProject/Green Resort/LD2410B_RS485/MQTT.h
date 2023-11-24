#include <WiFi.h>
#include <PubSubClient.h>

// ----------------------------------- DEVICE CONFIG ----------------------------
// FORMAT [DeviceID/Vitri/loaiThietBi/cheDoCuaThietBi (on/off)/NguoiChuyenDong (trang thai cam bien)/trangThaiThietBi (online/offline)]
#define DEVICE_ID           "B01"    
#define LOCATION            "001"
#define DEVICE_TYPE         "02"
#define DEVICE_STT          "1"
// GREENDATA2 -- TEST
#define DATA_TOPIC          "GREENDATA2"                   // -- DATA FORMAT []
#define SUB_TOPIC_REQUEST   "GREENDATA2/CONTROLS"                     // -- DATA FORMAT [ALL CHARACTER]
#define SUB_TOPIC_MODE      "GREENDATA2/MODE/B01"                     // -- DATA FORMAT [ALL CHARACTER]
#define SUB_TOPIC_TIME      "GREENDATA2/TIME/B01"                     // -- DATA FORMAT [ALL CHARACTER]
// #define SUB_TOPIC_SETUP1 "KIMKHI3P/SENSOR/SETUP/TIME/TB3"   // -- DATA FORMAT [sensorUpdateTimeoutS:randomCheckTimeoutS]
// #define SUB_TOPIC_SETUP2 "KIMKHI3P/SENSOR/SETUP/ALARM/TB3" // -- DATA FORMAT [DS_t:DHT_t:DHT_h]
// #define SUB_TOPIC_SETUP3 "KIMKHI3P/SENSOR/SETUP/MODE/TB3" // -- DATA FORMAT [DS_t:DHT_t:DHT_h]


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
    modeControl = (uint8_t)(s.toInt());
    EEPROM.write(ADDR_MODE, modeControl);
    EEPROM.commit();
  } else if (strcmp(topic, SUB_TOPIC_TIME) == 0) {
    controlOffTimeoutM = (uint8_t)(s.toInt());
    EEPROM.write(ADDR_TIME, controlOffTimeoutM);
    EEPROM.commit();
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
  // FORMAT [DeviceID/Vitri/loaiThietBi/cheDoCuaThietBi (on/off)/NguoiChuyenDong (trang thai cam bien)/trangThaiThietBi (online/offline)]
  String sendString = String(DEVICE_ID) + "/" + String(LOCATION) + "/" + String(DEVICE_TYPE) + "/";
  if (modeControl == 0) sendString += String(S_RL);
  else sendString += String(S_RL_CLI);
  sendString += ("/" + String(S_SENSOR) + "/");
  if (modeControl == 0) sendString += String(DEVICE_STT);
  else sendString += String(clientConnect);
  client.publish(DATA_TOPIC, sendString.c_str());
  delay(100);
}