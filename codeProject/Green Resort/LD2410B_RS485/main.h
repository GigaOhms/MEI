#include <EEPROM.h>

#define RL 13
#define BTN1 21
#define BTN2 18
#define LED1 19
#define LED2 5
#define pinSENSOR 4

bool S_BTN1 = 0;
bool S_BTN2 = 0;
bool S_BTN12 = 0;
bool S_LED1 = 0;
bool SS_LED1 = 0;
bool S_LED2 = 0;
bool SS_LED2 = 0;
bool S_RL = 0;
bool SS_RL = 0;
bool S_RL_CLI = 0;
bool S_SENSOR;
bool SS_SENSOR;

bool ON_RL_CLI = 0;
bool OFF_RL_CLI = 0;

bool clientConnect = 1;     // 0: when lost connect RS485
uint8_t modeControl = 1;    // 1: Dieu khien relay RS485
                            // 0: Dieu khien relay Onboard
bool manualStt = 0;         // 1: Dieu khien bang nut nhan
                            // 0: Dieu khien tu dong

uint8_t offRelayTime;

String s = "";

uint32_t randomClientTime = 0;
uint32_t requestClientTime = 0;
#define RANDOM_CLIENT_TIMEOUT_MS    10000U
#define REQUEST_CLIENT_TIMEOUT_MS   30000U

uint32_t sensorUpdateTimeoutS = 10;
uint8_t controlOffTimeoutM;
uint8_t controlOffCounter = 0;


#define NOW_TIME millis()

void rs485_receive(void);
void IRAM_ATTR relayControl(void);
void manualButton(void);
void checkStatusRL(void);





// ------------------------------ WDT SETUP -------------------------
uint32_t runWatchDog = 0;
#define WATCHDOG_TIMEOUT_S 180 // 3 minute
hw_timer_t * watchDogTimer = NULL;
hw_timer_t * interruptTimer = NULL;

void IRAM_ATTR watchDogInterrupt(void);
void watchDogRefresh(void);








// ----------------------------- EEPROM SETUP --------------------------------
#define EEPROM_SIZE    2

#define ADDR_MODE      0
#define ADDR_TIME      1