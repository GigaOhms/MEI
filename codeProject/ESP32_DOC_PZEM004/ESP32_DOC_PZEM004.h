#include <WiFi.h>
const char* ssid = "MEIGROUP01 - 2.4GHZ";
const char* pass = "77namhai88";

// khái báo PZEM
#include <PZEM004Tv30.h>
#include <HardwareSerial.h>
#define RX2 16
#define TX2 17
#define SerialPzem Serial2
PZEM004Tv30 pzem(SerialPzem, RX2, TX2);
float voltage;
float current;
float power;
float energy;
float frequency;
float pf;

#include <EEPROM.h>

int KHOA_THIET_BI = 0;  // doc tai Ô EEPROM 511

//Khai báo MQTT
#include <PubSubClient.h>
const char* mqtt_server = "meigroup.ddns.net";
const char* mqtt_user = "meigroup";
const char* mqtt_pass = "@778899";
const int port = 1883;
WiFiClient espClient;
PubSubClient client(espClient);

String TOPIC_TONG = "GREENDATA2";
String Sdata_send = "BEGIN";

String Data;
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("TOPIC: ");
  String TOPIC = String(topic);
  Serial.print(TOPIC);

  Serial.print("   PAYLOAD: ");
  String PAYLOAD;
  for (uint8_t i = 0; i < length; i++) {
    PAYLOAD += char(payload[i]);
  }
  Serial.println(PAYLOAD);

  if (TOPIC.lastIndexOf(TOPIC_TONG + "/RESET") != -1) {
    int RESET = PAYLOAD.toInt();
    if (RESET == 1) {
      String topic_reset = TOPIC_TONG + "/RESET";
      client.publish(topic_reset.c_str(), "RESET ON");
      delay(500);
      ESP.restart();
    }
  }

  else if (TOPIC.lastIndexOf(TOPIC_TONG + "/ResetEnergy") != -1) {
    int ResetEnergy = PAYLOAD.toInt();
    if (ResetEnergy == 1) {
      String topic_ResetEnergy = TOPIC_TONG + "/ResetEnergy";
      client.publish(topic_ResetEnergy.c_str(), "RESET ON");
      delay(500);

      pzem.resetEnergy();
      Serial.println("RESET ENERGY");
    }
  }

  else if (TOPIC.lastIndexOf(TOPIC_TONG + "/KHOA_THIET_BI") != -1) {
    if (PAYLOAD.lastIndexOf("KHOA_ON") != -1) {
      String topic_khoathietbi = TOPIC_TONG + "/KHOA_THIET_BI";
      client.publish(topic_khoathietbi.c_str(), "KHOA THIET BI ON");

      KHOA_THIET_BI = 1;
      EEPROM.write(511, KHOA_THIET_BI);
      EEPROM.commit();
      delay(500);

      Serial.println("KHOA THIET BI ON");
    }

    else if (PAYLOAD.lastIndexOf("KHOA_OFF") != -1 && KHOA_THIET_BI == 1) {
      String topic_khoathietbi = TOPIC_TONG + "/KHOA_THIET_BI";
      client.publish(topic_khoathietbi.c_str(), "KHOA THIET BI OFF");

      KHOA_THIET_BI = 0;
      EEPROM.write(511, KHOA_THIET_BI);
      EEPROM.commit();
      delay(500);

      Serial.println("KHOA THIET BI OFF");
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-" + TOPIC_TONG + "_";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("connected");

      // publish
      //client.publish(TOPIC_TONG.c_str(), Sdata_send.c_str());


      // subscribe
      String topic_sub = TOPIC_TONG + "/RESET";
      client.subscribe(topic_sub.c_str());

      String topic_ResetEnergy = TOPIC_TONG + "/ResetEnergy";
      client.subscribe(topic_ResetEnergy.c_str());

      String topic_khoathietbi = TOPIC_TONG + "/KHOA_THIET_BI";
      client.subscribe(topic_khoathietbi.c_str());

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      delay(5000);
    }
  }
}

// khai báo ngoại vi
#define LED1 19
#define LED2 4
#define BNT1 27
#define BNT2 14

// Khai báo watdog
#define WATCHDOG_TIMEOUT_S 60 * 3  //3 phut
hw_timer_t* watchDogTimer = NULL;
unsigned long RUN_WATCH_DOG = 0;

void IRAM_ATTR watchDogInterrupt() {
  ESP.restart();
}

// khai báo get read time
#include <WiFiUdp.h>
#include <NTPClient.h>
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
int NGAY_OL;

void setup() {
  Serial.begin(115200);
  delay(200);

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  digitalWrite(LED1, 0);
  digitalWrite(LED2, 0);
  pinMode(BNT1, INPUT_PULLUP);
  pinMode(BNT2, INPUT_PULLUP);

  EEPROM.begin(512);
  NGAY_OL = EEPROM.read(0);
  Serial.println("NGAY_OL: " + String(NGAY_OL));

  KHOA_THIET_BI = EEPROM.read(511);
  Serial.println("KHOA_THIET_BI: " + String(KHOA_THIET_BI));

  // Setup Pzem
  SerialPzem.begin(9600, SERIAL_8N1, RX2, TX2);
  delay(1000);

  // Setup Wifi
  Serial.println();
  Serial.print("Connecting to wifi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Setup mui giờ
  timeClient.begin();
  timeClient.setTimeOffset(25200);

  // Setup mqtt
  client.setServer(mqtt_server, port);
  client.setCallback(callback);
  reconnect();

  // Setup watchDogTimer
  watchDogTimer = timerBegin(2, 80, true);
  timerAttachInterrupt(watchDogTimer, &watchDogInterrupt, true);
  timerAlarmWrite(watchDogTimer, WATCHDOG_TIMEOUT_S * 1000000, false);
  timerAlarmEnable(watchDogTimer);
}

String DeviceID = "A01";
String Vitri = "001";
String loai_thiet_bi = "01";

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    if (!client.connected()) {
      reconnect();
    }
    client.loop();

    Set_Random_Check_Online_TB();

    if (KHOA_THIET_BI == 0) {
      static uint32_t TIME_RUN_LED = millis();
      if (millis() - TIME_RUN_LED > 200) {
        TIME_RUN_LED = millis();
        digitalWrite(LED1, !digitalRead(LED1));
      }

      static uint32_t TIME_RUN_DO = millis() - 5000;
      if (millis() - TIME_RUN_DO > 5000) {
        TIME_RUN_DO = millis();

        DO_PZEM();                     // get thông số Pzem
        CheckResetEnergy();            // check qua ngày ResetEnergy
        timerWrite(watchDogTimer, 0);  // reset lại watchDogTimer
      }
    }
  }

  /*
  uint32_t TIME_OUT_BNT = millis();
  while (digitalRead(BNT1) == 0) {  // Reset Energy pzem
    Serial.println("*");
    delay(200);
    if (millis() - TIME_OUT_BNT > 5000) {
      pzem.resetEnergy();
      Serial.println("RESET ENERGY");
      break;
    }
  }
  if (digitalRead(BNT2) == 0) {
    // xuly
    delay(200);
    while (digitalRead(BNT2) == 0) {
      delay(50);
    }
  }
  */
}

void DO_PZEM() {
  int check_Pzem = 1;

  voltage = pzem.voltage();
  if (!isnan(voltage)) {
    Serial.print("Voltage: " + String(voltage) + "V");
  } else {
    Serial.println("Error reading voltage");
    check_Pzem = 0;
  }

  current = pzem.current();
  if (!isnan(current)) {
    Serial.print("  Current: " + String(current) + "A");
  } else {
    Serial.println("Error reading current");
    check_Pzem = 0;
  }

  power = pzem.power();
  if (!isnan(power)) {
    Serial.print("  Power: " + String(power) + "W");
  } else {
    Serial.println("Error reading power");
    check_Pzem = 0;
  }

  energy = pzem.energy();
  if (!isnan(energy)) {
    Serial.print("  Energy: " + String(energy) + "kWh");
  } else {
    Serial.println("Error reading energy");
    check_Pzem = 0;
  }

  frequency = pzem.frequency();
  if (!isnan(frequency)) {
    Serial.print("  Frequency: " + String(frequency) + "Hz");
  } else {
    Serial.println("Error reading frequency");
    check_Pzem = 0;
  }

  pf = pzem.pf();
  if (!isnan(pf)) {
    Serial.print("  PF: " + String(pf));
  } else {
    Serial.println("Error reading power factor");
    check_Pzem = 0;
  }
  Serial.println();

  if (check_Pzem == 1) {
    Sdata_send = String(DeviceID) + "/" + String(Vitri) + "/" + String(loai_thiet_bi) + "/";
    Sdata_send = Sdata_send + String(power) + "/" + String(energy) + "/" + String(current) + "/" + String(voltage) + "/1";

    client.publish(TOPIC_TONG.c_str(), Sdata_send.c_str());
  } else {
    String S_error = "PZEM ERROR !";

    client.publish(TOPIC_TONG.c_str(), S_error.c_str());
  }
}

void CheckResetEnergy() {
  // update date time hiên tại
  timeClient.update();
  time_t epochTime = timeClient.getEpochTime();
  struct tm* ptm = gmtime((time_t*)&epochTime);
  int NGAY = ptm->tm_mday;

  if (NGAY != NGAY_OL) {
    NGAY_OL = NGAY;
    EEPROM.write(0, NGAY);
    EEPROM.commit();
    delay(200);
    pzem.resetEnergy();
    Serial.println("RESET ENERGY");
  }
}

void Set_Random_Check_Online_TB() {
  static uint32_t Time_set_check_online = millis() - 5000;
  if (millis() - Time_set_check_online > 5000) {
    Time_set_check_online = millis();
    if (KHOA_THIET_BI == 1) {
      // publish random check online
      String topic = TOPIC_TONG + "/random";
      String S_rand = String(random(0, 999));
      S_rand = S_rand + "/K";
      client.publish(topic.c_str(), S_rand.c_str());

      Sdata_send = String(DeviceID) + "/" + String(Vitri) + "/" + String(loai_thiet_bi) + "/";
      Sdata_send = Sdata_send + "0.00/0.00/0,00/0,00/0"; // khoa thiêt bị = 0 tất cả
      client.publish(TOPIC_TONG.c_str(), Sdata_send.c_str());
    } else {
      // publish random check online
      String topic = TOPIC_TONG + "/random";
      String S_rand = String(random(0, 999));
      client.publish(topic.c_str(), S_rand.c_str());
    }
  }
}