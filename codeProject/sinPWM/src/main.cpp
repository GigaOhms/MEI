#include <Arduino.h>
#include <avr/interrupt.h>
#include <math.h>

#define pi 3.1415926535897932384626433832795
int f = 50;
volatile float t = 0.0;
volatile float data;
char buf[12];

ISR (TIMER2_COMPA_vect){        //TIMER2_OVF_vect 
    data = (800 * sin(2*pi*f*t) + 800) / 2;
    OCR1B = data;
    Serial.println(data);
    // f2str(data, buf);
    // Serial.println(buf);
    t += 0.0001;
    if (t >= 0.0199)
        t = 0.0;
}

void setup() {
    Serial.begin(115200);
    TCCR2A = 0;
    TCCR2B = 0;
    TCNT2 = 0;

    OCR2A = 49;               // ~ 10.000 Hz = (16000000/((49 + 1)*32))
    TCCR2A |= (1 << WGM21);   // CTC
    TCCR2B |= (0 << CS22) | (1 << CS21) | (1 << CS20);  // Prescaler 32
    TIMSK2 |= (1 << OCIE2A);  // Output Compare Match A Interrupt Enable

    sei();                  // Enable global interrupts   

    // PWM by timer 1 with frequency 20kHz
    TCCR1A = 0; TCCR1B = 0; // Reset 2 registers
    DDRB |= (1 << PB2);     // PB2 is OUTPUT ( pin 10)                 
    
    TCCR1A |= (1 << WGM11);
    TCCR1B |= (1 << WGM12)|(1 << WGM13); // select Fast PWM mode select TOP_value freely ICR1                     
    TCCR1A |= (1 << COM1B1); // None-inverted mode
    ICR1 = 800;              // Top_value = 800. Frequency 20kHz = 16M / Top value
    TCCR1B |= (1 << CS10);   // No Prescaling = F_Clock or F_clock/1=16mhz
    OCR1B = 0.5*800;         // Duty cycle D%
}

void loop() {
    int a = millis();
}



// float a = -1234567.89;

// void setup(){
//     Serial.begin(115200);
// }

// void loop(){
//     // char ch[12] = {'-', '1', '2', '3', '4', '5', '6', '7', '.', '8', '9'};
//     String ch = "";
//     // f2str(a, ch);
//     ch.concat(a);
//     delay(1000);
//     Serial.println(ch);
// }