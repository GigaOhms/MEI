#include "main.h"
#include "WifiLogin.h"
#include "MQTT.h"


void setup() {
  Serial.begin(115200);
  Serial2.begin(9600);

  watchDogTimer = timerBegin(2, 80, true);
  timerAttachInterrupt(watchDogTimer, &watchDogInterrupt, true);
  timerAlarmWrite(watchDogTimer, WATCHDOG_TIMEOUT_S * 1000000, false);
  timerAlarmEnable(watchDogTimer);

  interruptTimer = timerBegin(0, 80, true);
  timerAttachInterrupt(interruptTimer, &relayControl, true);
  timerAlarmWrite(interruptTimer, sensorUpdateTimeoutS * 1000000, true);
  timerAlarmEnable(interruptTimer);

  pinMode(RL, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(BTN1, INPUT);
  pinMode(BTN2, INPUT);

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
    checkStatusRL();
    delay(1000);
    if(digitalRead(BTN1) == 0) 
      getWiFiInfo();
  }
  // setup_wifi();
  client.setServer(MQTT_SERVER, PORT);
  client.setCallback(callback);
  reconnect();

  S_SENSOR = digitalRead(pinSENSOR);

  runWatchDog = NOW_TIME;
  sensorUpdateTime = NOW_TIME;
  randomClientTime = NOW_TIME;
  requestClientTime = NOW_TIME;
}


void loop() {
  uint16_t WAIT_TIME = 0;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    WAIT_TIME++;
    if (WAIT_TIME > WAIT_WIFI_CONNECT_S)
      ESP.restart();
    // ----------------------------- MODIFIER -------------------------------------
    uint32_t tempTime = millis();
    while ((uint32_t)(millis() - tempTime) <= 1000) {
      manualButton();
      checkStatusRL();
      if (modeControl == 1)
        rs485_receive();
    }
  }

  if (!client.connected())
    reconnect();
  client.loop();

  if ((uint32_t)(NOW_TIME - runWatchDog) >= 5000) {
    runWatchDog = NOW_TIME;
    watchDogRefresh();
  }

  if ((uint32_t)(NOW_TIME - sensorUpdateTime) >= sensorUpdateTimeoutMS) {
    sendDataMQTT();
    sensorUpdateTime = NOW_TIME;
  }

  manualButton();
  checkStatusRL();

  if (modeControl == 1) {
    rs485_receive();

    if ((uint32_t)(NOW_TIME - randomClientTime >= RANDOM_CLIENT_TIMEOUT_MS)) {
      Serial2.print("rd*");
      randomClientTime = NOW_TIME;
    }
    if ((uint32_t)(NOW_TIME - requestClientTime >= REQUEST_CLIENT_TIMEOUT_MS)) 
      clientConnect = 0;
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


void rs485_receive(void) {
  if (Serial2.available()) {
    char c = Serial2.read();
    if (c != '*') s += c;
    else {
      if (s[0] == 's') {
        S_RL_CLI = (s.substring(1, 2)).toInt();
        S_LED2 = S_RL_CLI;
        digitalWrite(LED2, S_LED2);
        sendDataMQTT();
      } else if (s[0] == 'r') {
        S_RL_CLI = (s.substring(1, 2)).toInt();
        S_LED2 = S_RL_CLI;
        digitalWrite(LED2, S_LED2);
        requestClientTime = millis();
        clientConnect = 1;
      }
      s = "";
    }
  }
}

void IRAM_ATTR relayControl(void) {
  S_SENSOR = digitalRead(pinSENSOR);

  if (lockDevice == 1) {
    if (modeControl == 1) 
      ON_RL_CLI = 1;
    else {
      S_RL = 1;
      S_LED2 = S_RL;
      digitalWrite(LED2, S_LED2);
      digitalWrite(RL, S_RL);
    }
    return ;
  }

  if (autoMode == 0) {
    if (modeControl == 1) 
      OFF_RL_CLI = 1;
    else {
      S_RL = 0;
      S_LED2 = S_RL;
      digitalWrite(LED2, S_LED2);
      digitalWrite(RL, S_RL);
    }
    return ;
  }

  if (S_SENSOR == 0) {
    controlOffCounter++;
    if (controlOffCounter >= (controlOffTimeoutM * 6)){
      controlOffCounter = 0;
      manualStt = 0;
      if (modeControl == 1)
        OFF_RL_CLI = 1;
      else if (modeControl == 0) {
        S_RL = 0;
        S_LED2 = S_RL;
        digitalWrite(LED2, S_LED2);
        digitalWrite(RL, S_RL);
      }
    }
  } else controlOffCounter = 0;

  if (manualStt == 0) {
    if (modeControl == 1) {
      if (S_SENSOR == 1) 
        ON_RL_CLI = 1;
    } else if (modeControl == 0) { 
      if (S_SENSOR == 1) {
        S_RL = S_SENSOR;
        S_LED2 = S_RL;
        digitalWrite(LED2, S_LED2);
        digitalWrite(RL, S_RL);
      }
    }
  }
}

void manualButton(void) {
  if (digitalRead(BTN1) == 0 && S_BTN1 == 0) {
    S_BTN1 = 1;
    manualStt = 1;
    delay(200);
    if (autoMode == 1 && lockDevice == 0){
      if (modeControl == 1) {
        S_RL_CLI = S_RL_CLI ? 0 : 1;
        if (S_RL_CLI) Serial2.print("s1*");
        else Serial2.print("s0*");
      } else {
        S_RL = S_RL ? 0 : 1;
        S_LED2 = S_RL;
        digitalWrite(LED2, S_LED2);
        digitalWrite(RL, S_RL);
      }
    }
  } else if (digitalRead(BTN1) == 1) S_BTN1 = 0;
}

void checkStatusRL(void) {
  if (ON_RL_CLI == 1) {
    Serial2.print("s1*");
    ON_RL_CLI = 0;
  } else if (OFF_RL_CLI == 1) {
    Serial2.print("s0*");
    OFF_RL_CLI = 0;
  }
}

void eepromSetup(void) {
  modeControl = EEPROM.read(ADDR_MODE);
  if (modeControl > 1) {
    modeControl = 1;
    EEPROM.write(ADDR_MODE, modeControl);
    EEPROM.commit();
  }
  controlOffTimeoutM = EEPROM.read(ADDR_TIME);
  if (controlOffTimeoutM < 2 || controlOffTimeoutM > 40) {
    controlOffTimeoutM = 5;
    EEPROM.write(ADDR_TIME, controlOffTimeoutM);
    EEPROM.commit();
  }
  autoMode = EEPROM.read(ADDR_AUTO);
  if (autoMode > 1) {
    autoMode = 1;
    EEPROM.write(ADDR_AUTO, autoMode);
    EEPROM.commit();
  }
  lockDevice = EEPROM.read(ADDR_LOCK);
  if (lockDevice > 1) {
    lockDevice = 0;
    EEPROM.write(ADDR_LOCK, lockDevice);
    EEPROM.commit();
  }
}