#include <Arduino.h>

#include <FastLED.h>

#define LED_PIN 3
#define BRIGHTNESS 50
#define NUM_LEDS 8

CRGB leds[NUM_LEDS];

bool arr[6] = {1, 1, 1, 1, 0, 1};

void setup() {
    FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(BRIGHTNESS);
}

// void loop() { 
//     for (int k = 0; k < 10; k++){
//         for (int i = 0; i < 8; i++){
//             for (int j = 0; j < 8; j++){
//                 if (dgt[k][i][j] == 1)
//                     leds[(txt[i][j])] = CRGB::BlueViolet;
//                 else leds[(txt[i][j])] = 0;
//             }
//             FastLED.show();
//         }
//         delay(1200);
//     }
// }

// int arrColor[6] = {0x8A2BE2}

// uint_t16 arrColor[]

uint8_t cnt = 0;
uint32_t arrColor[6] = {0x228B22, 0x8A2BE2, 0xFFE4B5, 0x8B0000, 0xFFC0CB, 0xFFD700};

void loop(){
  for (int k = 0; k < 8; k++){
    // leds[k] = CRGB::DarkRed;
    leds[k] = arrColor[cnt++];
    if (cnt > 5) cnt = 0;
    delay(500);
    FastLED.show();
  }
}