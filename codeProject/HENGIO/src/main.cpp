#include <Arduino.h>
#include <avr/interrupt.h>

#define HOUR_SET    21
#define MINUTE_SET  19
#define SECOND_SET  0

#define LED1        4
#define TG_SANG     5
int8_t curTime = 0;

int16_t countTime = 0;
int8_t HOUR = 21;
int8_t MINUTE = 21;
int8_t SECOND = 0;

void UPDATE_TIME(void);

ISR (TIMER1_OVF_vect){        //TIMER1_OVF_vect 
  countTime++;
  UPDATE_TIME();
}

void setup() {
    // timer 1 Interrupt-------------------------------------------------
    TCNT1 = 0;
    TCCR1A = 0; TCCR1B = 0;               // Reset 2 registers 
    TCCR1A |= (1 << WGM11);
    TCCR1B |= (1 << WGM12)|(1 << WGM13); // Fast PWM [TOP = ICR1]                 
    ICR1 = 15999;                        // Top value [Frequency = 16M / (ICR1 + 1) = 1000 Hz]
    TCCR1B |= (1 << CS10);              // No Prescaling = F_Clock or F_clock/1=16mhz
    TIMSK1 |= (1 << TOIE1);             // Timer1 Overflow Interrupt Enable
    sei();

    pinMode(LED1, OUTPUT);
}

void loop() {
  if (HOUR = HOUR_SET && MINUTE == MINUTE_SET && SECOND == SECOND_SET){
    curTime = SECOND;
    digitalWrite(LED1, HIGH);
  }
  if (SECOND - curTime >= TG_SANG || ((SECOND - curTime) < 0 && SECOND - curTime + 60 > TG_SANG))
    digitalWrite(LED1, LOW);
}

void UPDATE_TIME(void){
  if (countTime >= 1000){
    countTime = 0;
    SECOND++;
  }
  if (SECOND >= 59){
    SECOND = 0;
    MINUTE++;
  }
  if (MINUTE >= 59){
    HOUR++;
    MINUTE = 0;
  }
  if (HOUR >= 23)
    HOUR = 0;
}