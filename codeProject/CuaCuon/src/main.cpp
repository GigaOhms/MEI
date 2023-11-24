#include <Arduino.h>
#include <avr/interrupt.h>

#define BT_UP   1
#define BT_DOWN 2
#define BT_STOP 3
#define SW_TOP  4
#define SW_BOT  5
#define LED_UP  6
#define LED_DOWN   7
#define MOTOR_UP   8
#define MOTOR_DOWN 9

int countTime = 0;
unsigned long SECOND = 0;
byte TT_BT_UP = 0;
byte TT_BT_DOWN = 0;
byte TT_BT_STOP = 0;
byte TT_SW_TOP = 0;
byte TT_SW_BOT = 0;

void ROLL_UP(void);         // cuon len
void ROLL_UP_SLOW(void);    // cuon len cham 5s cuoi
void ROLL_DOWN(void);       // cuon xuong
void ROLL_DOWN_SLOW(void);  // cuon xuong cham 5s cuoi
void ROLL_STOP(void);       // dung

ISR (TIMER1_OVF_vect){      //TIMER1_OVF_vect
  countTime++;
  if (countTime >= 1000){
    countTime = 0;
    SECOND++;
  }
}

void setup() {

  TCNT1 = 0;
  TCCR1A = 0; TCCR1B = 0;               // Reset 2 registers 
  TCCR1A |= (1 << WGM11);
  TCCR1B |= (1 << WGM12)|(1 << WGM13); // Fast PWM [TOP = ICR1]                 
  ICR1 = 15999;                        // Top value [Frequency = 16M / (ICR1 + 1) = 1000 Hz]
  TCCR1B |= (1 << CS10);              // No Prescaling = F_Clock or F_clock/1=16mhz
  TIMSK1 |= (1 << TOIE1);             // Timer1 Overflow Interrupt Enable
  sei();

  pinMode(BT_UP, INPUT);
  pinMode(BT_DOWN, INPUT);
  pinMode(BT_STOP, INPUT);
  pinMode(SW_BOT, INPUT);
  pinMode(SW_TOP, INPUT);
  pinMode(LED_UP, OUTPUT);
  pinMode(LED_DOWN, OUTPUT);
  pinMode(MOTOR_UP, OUTPUT);
  pinMode(MOTOR_DOWN, OUTPUT);

  digitalWrite(LED_UP, LOW);
  digitalWrite(LED_DOWN, LOW);
  digitalWrite(MOTOR_UP, LOW);
  digitalWrite(MOTOR_DOWN, LOW);
}

void loop() {
  if (TT_BT_STOP)
}

// -------------------------------------------- Funtion --------------------------------------

void ROLL_STOP(void){
  digitalWrite(LED_UP, LOW);
  digitalWrite(LED_DOWN, LOW);
  digitalWrite(MOTOR_UP, LOW);
  digitalWrite(MOTOR_DOWN, LOW);
}

void ROLL_UP(void){
  digitalWrite(MOTOR_UP, HIGH);
}

void ROLL_UP_SLOW(void){
  digitalWrite(MOTOR_UP, LOW);
  digitalWrite(LED_UP, HIGH);
}

void ROLL_DOWN(void){
  digitalWrite(MOTOR_DOWN, HIGH);
}

void ROLL_DOWN_SLOW(void){
  digitalWrite(MOTOR_DOWN, LOW);
  digitalWrite(LED_DOWN, HIGH);
}