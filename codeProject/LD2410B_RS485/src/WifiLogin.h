#include "SPIFFS.h"
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>


#define WAIT_WIFI_CONNECT_S 600
#define WIFI_NAME "RESORT GREEN B01"


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



// ------------------------------ FUNCTION ---------------------------------------------
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
  if (ssid==""){
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
    // delay(1000); // OLD CODE
    WAIT_TIME++;
    if (WAIT_TIME > WAIT_WIFI_CONNECT_S)
      return false;
    
    // ------------------------------------------------ MODIFIER -------------------------------------
    uint32_t tempTime = millis();
    while ((uint32_t)(millis() - tempTime) <= 1000) {
      manualButton();
      checkStatusRL();
      if (modeControl == 1)
        rs485_receive();
    }
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