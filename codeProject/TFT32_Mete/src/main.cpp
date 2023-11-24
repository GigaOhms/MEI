#include <Arduino.h>

// Define meter size as 1 for tft.rotation(0) or 1.3333 for tft.rotation(1)
#define M_SIZE 1.3333

#include <TFT_eSPI.h> // Hardware-specific library
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI();       // Invoke custom library

#define TFT_GREY 0x5AEB

void analogMeter();
void plotNeedle(float value, byte ms_delay);

float ltx = 0;    // Saved x coord of bottom of needle
uint16_t osx = M_SIZE*120, osy = M_SIZE*120; // Saved x & y coords
uint32_t updateTime = 0;       // time for next update

float old_analog =  -999; // Value last displayed

float value[6] = {0, 0, 0, 0, 0, 0};
int old_value[6] = { -1, -1, -1, -1, -1, -1};
int d = 0;

void setup(void) {
  tft.init();
  tft.setRotation(1);
  Serial.begin(57600); // For debug
  tft.fillScreen(TFT_BLACK);

  analogMeter(); // Draw analogue meter

  updateTime = millis(); // Next update time
}


void loop() {
  float pH = 8.5;
  if (updateTime <= millis()) {
    updateTime = millis() + 35; // Update emter every 35 milliseconds
 
    // Create a Sine wave for testing
    // d += 4; if (d >= 360) d = 0;
    // value[0] = 50 + 50 * sin((d + 0) * 0.0174532925);
    // value[0] = 50 + 50 * sin(pH / 14.0 * 360.0 * 0.0174532925);
    value[0] = pH * 100.0 / 14.0;
 
    plotNeedle(value[0], 0); // It takes between 2 and 12ms to replot the needle with zero delay
  }
}


// #########################################################################
//  Draw the analogue meter on the screen
// #########################################################################
void analogMeter()
{

  // Meter outline
  tft.fillRect(0, 0, M_SIZE*239, M_SIZE*126, TFT_GREY);
  tft.fillRect(5, 3, M_SIZE*230, M_SIZE*119, TFT_WHITE);

  tft.setTextColor(TFT_BLACK);  // Text colour

  // Draw ticks every 5 degrees from -50 to +50 degrees (100 deg. FSD swing)
  for (int i = -50; i < 51; i += 5) {
    // Long scale tick length
    int tl = 15;

    // Coodinates of tick to draw
    float sx = cos((i - 90) * 0.0174532925);
    float sy = sin((i - 90) * 0.0174532925);
    uint16_t x0 = sx * (M_SIZE*100 + tl) + M_SIZE*120;
    uint16_t y0 = sy * (M_SIZE*100 + tl) + M_SIZE*140;
    uint16_t x1 = sx * M_SIZE*100 + M_SIZE*120;
    uint16_t y1 = sy * M_SIZE*100 + M_SIZE*140;

    // Coordinates of next tick for zone fill
    float sx2 = cos((i + 5 - 90) * 0.0174532925);
    float sy2 = sin((i + 5 - 90) * 0.0174532925);
    int x2 = sx2 * (M_SIZE*100 + tl) + M_SIZE*120;
    int y2 = sy2 * (M_SIZE*100 + tl) + M_SIZE*140;
    int x3 = sx2 * M_SIZE*100 + M_SIZE*120;
    int y3 = sy2 * M_SIZE*100 + M_SIZE*140;

    // Red zone limits
    if (i >= -50 && i < -25) {
     tft.fillTriangle(x0, y0, x1, y1, x2, y2, TFT_RED);
     tft.fillTriangle(x1, y1, x2, y2, x3, y3, TFT_RED);
    }

    // Yellow zone limits
    if (i >= -25 && i < 0) {
     tft.fillTriangle(x0, y0, x1, y1, x2, y2, TFT_YELLOW);
     tft.fillTriangle(x1, y1, x2, y2, x3, y3, TFT_YELLOW);
    }

    // Green zone limits
    if (i >= 0 && i < 25) {
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, TFT_GREEN);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, TFT_GREEN);
    }

    // Orange zone limits
    if (i >= 25 && i < 50) {
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, TFT_BLUE);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, TFT_BLUE);
    }

    // Short scale tick length
    if (i % 25 != 0) tl = 8;

    // Recalculate coords incase tick lenght changed
    x0 = sx * (M_SIZE*100 + tl) + M_SIZE*120;
    y0 = sy * (M_SIZE*100 + tl) + M_SIZE*140;
    x1 = sx * M_SIZE*100 + M_SIZE*120;
    y1 = sy * M_SIZE*100 + M_SIZE*140;

    // Draw tick
    tft.drawLine(x0, y0, x1, y1, TFT_BLACK);

    // Check if labels should be drawn, with position tweaks
    if (i % 25 == 0) {
      // Calculate label positions
      x0 = sx * (M_SIZE*100 + tl + 10) + M_SIZE*120;
      y0 = sy * (M_SIZE*100 + tl + 10) + M_SIZE*140;
      switch (i / 25) {
        case -2: tft.drawCentreString("0", x0, y0 - 12, 2); break;
        case -1: tft.drawCentreString("3.5", x0, y0 - 9, 2); break;
        case 0: tft.drawCentreString("7", x0, y0 - 7, 2); break;
        case 1: tft.drawCentreString("10.5", x0, y0 - 9, 2); break;
        case 2: tft.drawCentreString("14", x0, y0 - 12, 2); break;
      }
    }

    // Now draw the arc of the scale
    sx = cos((i + 5 - 90) * 0.0174532925);
    sy = sin((i + 5 - 90) * 0.0174532925);
    x0 = sx * M_SIZE*100 + M_SIZE*120;
    y0 = sy * M_SIZE*100 + M_SIZE*140;
    // Draw scale arc, don't draw the last part
    if (i < 50) tft.drawLine(x0, y0, x1, y1, TFT_BLACK);
  }

  tft.drawString("pH", M_SIZE*(5 + 230 - 40), M_SIZE*(119 - 20), 2); // Units at bottom right
  tft.drawCentreString("pH", M_SIZE*120, M_SIZE*70, 4); // Comment out to avoid font 4
  tft.drawRect(5, 3, M_SIZE*230, M_SIZE*119, TFT_BLACK); // Draw bezel line

  plotNeedle(0, 0); // Put meter needle at 0
}

// #########################################################################
// Update needle position
// This function is blocking while needle moves, time depends on ms_delay
// 10ms minimises needle flicker if text is drawn within needle sweep area
// Smaller values OK if text not in sweep area, zero for instant movement but
// does not look realistic... (note: 100 increments for full scale deflection)
// #########################################################################
void plotNeedle(float value, byte ms_delay)
{
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  char buf[8]; dtostrf(value * 14.0 / 100.0, 5, 2, buf);
  tft.drawRightString(buf, M_SIZE*40, M_SIZE*(119 - 20), 2);

  if (value < -10) value = -10; // Limit value to emulate needle end stops
  if (value > 110) value = 110;

  // Move the needle until new value reached
  while (!(value == old_analog)) {
    if (old_analog < value) old_analog++;
    else old_analog--;

    if (ms_delay == 0) old_analog = (value * 100.0); // Update immediately if delay is 0

    float sdeg = map(old_analog, -1000, 11000, -15000, -3000); // Map value to angle
    // Calcualte tip of needle coords
    float sx = cos(sdeg/100.0 * 0.0174532925);
    float sy = sin(sdeg/100.0 * 0.0174532925);

    // Calculate x delta of needle start (does not start at pivot point)
    float tx = tan((sdeg/100.0 + 90) * 0.0174532925);

    // Erase old needle image
    tft.drawLine(M_SIZE*(120 + 20 * ltx - 1), M_SIZE*(140 - 20), osx - 1, osy, TFT_WHITE);
    tft.drawLine(M_SIZE*(120 + 20 * ltx), M_SIZE*(140 - 20), osx, osy, TFT_WHITE);
    tft.drawLine(M_SIZE*(120 + 20 * ltx + 1), M_SIZE*(140 - 20), osx + 1, osy, TFT_WHITE);

    // Re-plot text under needle
    tft.setTextColor(TFT_BLACK);
    tft.drawCentreString("pH", M_SIZE*120, M_SIZE*70, 4); // // Comment out to avoid font 4

    // Store new needle end coords for next erase
    ltx = tx;
    osx = M_SIZE*(sx * 98 + 120);
    osy = M_SIZE*(sy * 98 + 140);

    // Draw the needle in the new postion, magenta makes needle a bit bolder
    // draws 3 lines to thicken needle
    tft.drawLine(M_SIZE*(120 + 20 * ltx - 1), M_SIZE*(140 - 20), osx - 1, osy, TFT_RED);
    tft.drawLine(M_SIZE*(120 + 20 * ltx), M_SIZE*(140 - 20), osx, osy, TFT_MAGENTA);
    tft.drawLine(M_SIZE*(120 + 20 * ltx + 1), M_SIZE*(140 - 20), osx + 1, osy, TFT_RED);

    // Slow needle down slightly as it approaches new postion
    if (abs(old_analog - value*100.0) < 1000) ms_delay += ms_delay / 5;

    // Wait before next update
    delay(ms_delay);
  }
}
