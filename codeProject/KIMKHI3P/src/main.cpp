#include "main.h"

void setup() {
  Serial.begin(115200);

  pinMode(LED, OUTPUT);
  pinMode(BZ, OUTPUT);
  pinMode(RL1, OUTPUT);
  pinMode(RL2, OUTPUT);
  pinMode(BTN1, INPUT);

  digitalWrite(LED, HIGH);
  ds18b20.begin();
  dht.begin();
  
  watchDogTimer = timerBegin(2, 80, true);
  timerAttachInterrupt(watchDogTimer, &watchDogInterrupt, true);
  timerAlarmWrite(watchDogTimer, WATCHDOG_TIMEOUT_S * 1000000, false);
  timerAlarmEnable(watchDogTimer);

  EEPROM.begin(EEPROM_SIZE);
  eepromSetup();
  
  initSPIFFS();
  // Load values saved in SPIFFS
  ssid = readFile(SPIFFS, ssidPath);
  pass = readFile(SPIFFS, passPath);
  ip = readFile(SPIFFS, ipPath);
  gateway = readFile (SPIFFS, gatewayPath);

  Serial.print("SSID set to: ");
  Serial.println(ssid);
  Serial.print("Password set to: ");
  Serial.println(pass);
          
  if(digitalRead(BTN1) == 0) 
    getWiFiInfo();
  
  while (!initWiFi()) {
    watchDogRefresh();
    delay(1000);
    if(digitalRead(BTN1) == 0) 
      getWiFiInfo();
  }

  client.setServer(MQTT_SERVER, PORT);
  client.setCallback(callback);
  reconnect();

  readSensor();
  randomMQTT();
  sendDataMQTT();
  sendSetupMQTT();

  randomCheckTimeoutMS = randomCheckTimeoutS * 1000; // Second to mili Second
  sensorUpdateTimeoutMS = sensorUpdateTimeoutS * 1000;
  sensorUpdateTime = NOW_TIME;
  preTimeLed = NOW_TIME;
  readSensorTime = NOW_TIME;
  randomCheckTime = NOW_TIME;
}

void loop() {
  if (!client.connected())
    reconnect();
  client.loop();

  if ((uint32_t)(NOW_TIME - preTimeLed) >= 500){
    digitalWrite(LED, LED_STT);
    LED_STT = LED_STT ? 0 : 1;
    preTimeLed = NOW_TIME;
  }

  if ((uint32_t)(NOW_TIME - RUN_WATCH_DOG) >= 5000) {
    RUN_WATCH_DOG = NOW_TIME;
    watchDogRefresh();
  }

  if ((uint32_t)(NOW_TIME - readSensorTime) >= READ_SENSOR_TIMEOUT_MS) { 
    readSensor();
    alarmCheck();
    readSensorTime = NOW_TIME;
  }

  if ((uint32_t)(NOW_TIME - sensorUpdateTime) >= sensorUpdateTimeoutMS) { 
    sendDataMQTT();
    sensorUpdateTime = NOW_TIME;
  }

  if ((uint32_t)(NOW_TIME - randomCheckTime) >= randomCheckTimeoutMS) { 
    randomMQTT();
    randomCheckTime = NOW_TIME;
  }
}




























// ------------------------- FUNCTION ----------------------------------------

void IRAM_ATTR watchDogInterrupt(void) {
  Serial.println("REBOOT !!!");
  ESP.restart();
}

void watchDogRefresh(void){
  timerWrite(watchDogTimer, 0);                    //reset thời gian của timer 
}

void readSensor(void){
  ds18b20.requestTemperatures();
  DS_t = ds18b20.getTempCByIndex(0);
  DHT_h = dht.readHumidity();
  DHT_t = dht.readTemperature();

  if (isnan(DS_t))
    DS_t = 0.0;
  if (isnan(DHT_h) || isnan(DHT_t)) {
    DHT_h = 0.0;
    DHT_t = 0.0;
  }
}

// Initialize SPIFFS
void initSPIFFS() {
  if (!SPIFFS.begin(true)) 
    Serial.println("An error has occurred while mounting SPIFFS");
  Serial.println("SPIFFS mounted successfully");
}

// Read File from SPIFFS
String readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path);
  if(!file || file.isDirectory()){
    Serial.println("- failed to open file for reading");
    return String();
  }
  
  String fileContent;
  while(file.available()){
    fileContent = file.readStringUntil('\n');
    break;     
  }
  return fileContent;
}

// Write file to SPIFFS
void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file){
    Serial.println("- failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("- file written");
  } else {
    Serial.println("- frite failed");
  }
}

// Initialize WiFi
bool initWiFi() {
  if(ssid=="" || ip==""){
    Serial.println("Undefined SSID or IP address.");
    return false;
  }

  WiFi.mode(WIFI_STA);
  // localIP.fromString(ip.c_str());
  // localGateway.fromString(gateway.c_str());

  // if (!WiFi.config(localIP, localGateway, subnet)){
  //   Serial.println("STA Failed to configure");
  //   return false;
  // }
  WiFi.begin(ssid.c_str(), pass.c_str());
  Serial.print("Connecting to WiFi..");

  uint16_t WAIT_TIME = 0;

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
    WAIT_TIME++;
    if (WAIT_TIME > WAIT_WIFI_CONNECT_S)
      return false;
  }
  Serial.print("\nConnected with IP: ");
  Serial.println(WiFi.localIP());
  return true;
}


void getWiFiInfo(){
  ssid = "";
  pass = "";
  ip = "";
  gateway = "";

  Serial.println("Setting AP (Access Point)");
  WiFi.softAP(WIFI_NAME, NULL);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP); 

  // Web Server Root URL
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/wifimanager.html", "text/html");
  });
  
  server.serveStatic("/", SPIFFS, "/");
  
  server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
    int params = request->params();
    for(int i=0;i<params;i++){
      AsyncWebParameter* p = request->getParam(i);
      if(p->isPost()){
        // HTTP POST ssid value
        if (p->name() == PARAM_INPUT_1) {
          ssid = p->value().c_str();
          Serial.print("SSID set to: ");
          Serial.println(ssid);
          // Write file to save value
          writeFile(SPIFFS, ssidPath, ssid.c_str());
        }
        // HTTP POST pass value
        if (p->name() == PARAM_INPUT_2) {
          pass = p->value().c_str();
          Serial.print("Password set to: ");
          Serial.println(pass);
          // Write file to save value
          writeFile(SPIFFS, passPath, pass.c_str());
        }
        // HTTP POST ip value
        if (p->name() == PARAM_INPUT_3) {
          ip = p->value().c_str();
          Serial.print("IP Address set to: ");
          Serial.println(ip);
          // Write file to save value
          writeFile(SPIFFS, ipPath, ip.c_str());
        }
        // HTTP POST gateway value
        if (p->name() == PARAM_INPUT_4) {
          gateway = p->value().c_str();
          Serial.print("Gateway set to: ");
          Serial.println(gateway);
          // Write file to save value
          writeFile(SPIFFS, gatewayPath, gateway.c_str());
        }
        Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
      }
    }
    request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: " + ip);
    delay(3000);
    ESP.restart();
  });
  server.begin();
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
    readSensor();
    randomMQTT();
    sendDataMQTT();
    sendSetupMQTT();
  } else if (strcmp(topic, SUB_TOPIC_SETUP_TIME) == 0){
    if (isValidData(s)) {
      int j = s.indexOf(':');
      uint32_t tempVar = (s.substring(0, j)).toInt();
      if (tempVar >= 1) {
        sensorUpdateTimeoutS = tempVar;
        sensorUpdateTimeoutMS = sensorUpdateTimeoutS * 1000;
        EEPROM.put(ADDR_SENSOR_TIMEOUT, sensorUpdateTimeoutS);
      }
      tempVar = (s.substring(j + 1, length)).toInt();
      if (tempVar >= 1) {
        randomCheckTimeoutS = tempVar;
        randomCheckTimeoutMS = randomCheckTimeoutS * 1000;
        EEPROM.put(ADDR_RANDOM_TIMEOUT, randomCheckTimeoutS);
      }
      EEPROM.commit();
      sendSetupMQTT();
    }
  } else if (strcmp(topic, SUB_TOPIC_SETUP_ALARM) == 0){
    if (isValidData(s)) {
      int j = s.indexOf(':');
      int k = s.indexOf(':', j + 1);
      float tempVar = (s.substring(0, j)).toFloat();
      if (tempVar > 0.1){
        alarmTempBoard = tempVar;
        EEPROM.put(ADDR_BOARD_TEM_ALARM, alarmTempBoard);
      } 
      tempVar = (s.substring(j + 1, k)).toFloat();
      if (tempVar > 0.1) {
        alarmTempSensor = tempVar;
        EEPROM.put(ADDR_SENSOR_TEM_ALARM, alarmTempSensor);
      }
      tempVar = (s.substring(k + 1, length)).toFloat();
      if (tempVar > 0.1) {
        alarmHumiSensor = tempVar;
        EEPROM.put(ADDR_SENSOR_HUM_ALARM, alarmHumiSensor);
      }
      EEPROM.commit();
      sendSetupMQTT();
    } 
  } else if (strcmp(topic, SUB_TOPIC_SETUP_MODE) == 0){
    uint8_t tempVar = (uint8_t)(s.toInt());
    if (tempVar <= 3) {
      alarmMode = tempVar;
      alarmCheck();
      EEPROM.put(ADDR_ALARM_MODE, alarmMode);
    }
    EEPROM.commit();
    sendSetupMQTT();
  }
}


void reconnect() {
  digitalWrite(LED, HIGH);
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("connected");
      client.subscribe(SUB_TOPIC_REQUEST);
      client.subscribe(SUB_TOPIC_SETUP_TIME);
      client.subscribe(SUB_TOPIC_SETUP_ALARM);
      client.subscribe(SUB_TOPIC_SETUP_MODE);
    } else {
      Serial.print("failed, rc = ");
      Serial.print(client.state());
      Serial.println(" | try again 2s");
      delay(2000);
    }
  }
}

void alarmCheck(void){
  if (DS_t >= alarmTempBoard) {
    digitalWrite(BZ, HIGH);
    sendDataMQTT();
  } else digitalWrite(BZ, LOW);

  if (alarmMode == 0){
    digitalWrite(RL1, LOW);
    return ;
  }

  if ((alarmMode == 1) && (DHT_t >= alarmTempSensor)) {
    digitalWrite(RL1, HIGH);
    sendDataMQTT();
    return ;
  } else if ((alarmMode == 2) && (DHT_h >= alarmHumiSensor)) {
    digitalWrite(RL1, HIGH);
    sendDataMQTT();
    return ;
  } else if (alarmMode == 3) {
    if ((DHT_t >= alarmTempSensor)|| (DHT_h >= alarmHumiSensor)) {
      digitalWrite(RL1, HIGH);
      sendDataMQTT();
      return ;
    }
  } else digitalWrite(RL1, LOW);
}

void randomMQTT(void){
  randomCheck = random(100, 999);
  String sendString = String(randomCheck);
  client.publish(RANDOM_TOPIC, sendString.c_str());
  delay(100);
}

void sendDataMQTT(void) {
  String sendString = String(DS_t, 1) + "/" + String(DHT_t, 1) + "/" + String(DHT_h, 1);
  client.publish(DATA_TOPIC, sendString.c_str());
  delay(100);
}

void sendSetupMQTT(void) {
  String sendString = String(resetCounter) + "/" + String(alarmMode) + "/" + String(sensorUpdateTimeoutS) + "/" + String(randomCheckTimeoutS) + "/" + String(alarmTempBoard, 1) + "/" + String(alarmTempSensor, 1) + "/" + String(alarmHumiSensor, 1);
  client.publish(ALARM_TOPIC, sendString.c_str());
  delay(100);
}

void eepromSetup(void) {
  EEPROM.get(ADDR_RESET_COUNTER, resetCounter);
  EEPROM.get(ADDR_SENSOR_TIMEOUT, sensorUpdateTimeoutS);
  EEPROM.get(ADDR_RANDOM_TIMEOUT, randomCheckTimeoutS);
  EEPROM.get(ADDR_BOARD_TEM_ALARM, alarmTempBoard);
  EEPROM.get(ADDR_SENSOR_TEM_ALARM, alarmTempSensor);
  EEPROM.get(ADDR_SENSOR_HUM_ALARM, alarmHumiSensor);
  EEPROM.get(ADDR_ALARM_MODE, alarmMode);
  
  resetCounter++;
  if (lastReset == 0)
    resetCounter = 0;
  EEPROM.put(ADDR_RESET_COUNTER, resetCounter);
  if (sensorUpdateTimeoutS < 1) {
    sensorUpdateTimeoutS = SENSOR_TIMEOUT_DEFAULT;
    EEPROM.put(ADDR_SENSOR_TIMEOUT, sensorUpdateTimeoutS);
  }
  if (randomCheckTimeoutS < 1) {
    randomCheckTimeoutS = RANDOM_TIMEOUT_DEFAULT;
    EEPROM.put(ADDR_RANDOM_TIMEOUT, randomCheckTimeoutS);
  }
  if (isnan(alarmTempBoard) || alarmTempBoard < 10.0) {
    alarmTempBoard = BOARD_TEM_ALARM_DEFAULT;
    EEPROM.put(ADDR_BOARD_TEM_ALARM, alarmTempBoard);
  }
  if (isnan(alarmTempSensor) || alarmTempSensor < 10.0) {
    alarmTempSensor = SENSOR_TEM_ALARM_DEFAULT;
    EEPROM.put(ADDR_SENSOR_TEM_ALARM, alarmTempSensor);
  }
  if (isnan(alarmHumiSensor) || alarmHumiSensor < 10.0) {
    alarmHumiSensor = SENSOR_HUM_ALARM_DEFAULT;
    EEPROM.put(ADDR_SENSOR_HUM_ALARM, alarmHumiSensor);
  }
  if (alarmMode > 3) {
    alarmMode = ALARM_MODE_DEFAULT;
    EEPROM.put(ADDR_ALARM_MODE, alarmMode);
  }
  EEPROM.commit();
}

bool isValidData(String str){
  bool isNum = false;
  for(byte i = 0; i < str.length(); i++){
    isNum = isDigit(str.charAt(i)) || str.charAt(i) == '.' || str.charAt(i) == ':'; // || str.charAt(i) == '-' || str.charAt(i) == '+';
    if(!isNum) return false;
  }
  return isNum;
}