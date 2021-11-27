// ESP32 WROOM RGB LED Nixie Clock Test

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#include <WiFi.h>
#include <HTTPClient.h>
#include "secrets.h"

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#include "time.h"

const char* ntpServer = "europe.pool.ntp.org"; // The Network Time Protocol Server

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#include <FastLED.h> // http://librarymanager/All#FastLED

#define LED_PIN     17 //GPIO17 on ESP32 WROOM is connected to SEC_1 WS2812 LED
#define COLOR_ORDER GRB
#define CHIPSET     WS2812
#define NUM_LEDS    60

#define BRIGHTNESS  50

CRGB leds[NUM_LEDS];

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void setup() {
  delay(1000); // sanity delay
  
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness( BRIGHTNESS );
  FastLED.clear(true); // Turn all the LEDs off

  Serial.begin(115200);
  Serial.println(F("RGB LED Nixie Clock"));

  //=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Connect to WiFi.

  Serial.print(F("Connecting to local WiFi"));

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println();

  Serial.println(F("WiFi connected!"));

  //=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Set the RTC using network time. (Code taken from the SimpleTime example.)

  // Request the time from the NTP server and use it to set the ESP32's RTC.
  configTime(0, 3600, ntpServer); // Set the GMT offset to 0, and the daylight offset to 0 or 3600.

  struct tm timeinfo;
  if(getLocalTime(&timeinfo))
  {
    Serial.println(&timeinfo, "Netweork time is: %A, %B %d %Y %H:%M:%S");
  }
  else
  {
    Serial.println("Failed to obtain time");
  }

  //=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Disconnect the WiFi as it's no longer needed

  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  Serial.println(F("WiFi disconnected"));
}

void loop()
{
  static uint8_t lastSecs = 0;
  
  struct tm timeinfo;
  if(getLocalTime(&timeinfo))
  {
    if (lastSecs != timeinfo.tm_sec)
    {
      Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
      timeToLEDs(timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
      lastSecs = timeinfo.tm_sec;
    }
  }
  else
  {
    Serial.println("Failed to obtain time");
  }
  
  FastLED.show();

  delay(20);
}

void timeToLEDs(uint8_t hour, uint8_t min, uint8_t sec)
{
  //Set the hue to the fraction of day.
  uint8_t hue = (uint8_t)(((((float)(((uint32_t)(hour * 60 * 60)) + ((uint32_t)(min * 60)) + ((uint32_t)(sec)))) / 86400.0)) * 256.0);

  //Apply the rainbow to the LEDs
  fill_rainbow( leds, NUM_LEDS, hue, 4); // 256 / 60 = 4 (approx.)

  // Change the other LEDs to Black
  for (int i = 0; i < 60; i++)
  {
    if ((i != 00 + (sec % 10))
    &&  (i != 10 + (sec / 10))
    &&  (i != 20 + (min % 10))
    &&  (i != 30 + (min / 10))
    &&  (i != 40 + (hour % 10))
    &&  (i != 50 + (hour / 10)))
      leds[i] = CRGB::Black;
  }
}
