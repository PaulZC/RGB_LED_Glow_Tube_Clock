// ESP32 WROOM RGB LED Nixie Clock Test

#include <FastLED.h> // http://librarymanager/All#FastLED

#define LED_PIN     17 //GPIO17 on ESP32 WROOM is connected to SEC_1 WS2812 LED
#define COLOR_ORDER GRB
#define CHIPSET     WS2812
#define NUM_LEDS    60

#define BRIGHTNESS  50

CRGB leds[NUM_LEDS];

uint8_t gHue = 0; // rotating "base color" used by many of the patterns

void setup() {
  delay(30); // sanity delay
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness( BRIGHTNESS );
}

void loop()
{
  fill_rainbow( leds, NUM_LEDS, gHue, 4); // 256 / 60 = 4 (approx.)
  FastLED.show();
  
  EVERY_N_MILLISECONDS( 25 ) { gHue++; } // cycle the "base color" through the rainbow
}
