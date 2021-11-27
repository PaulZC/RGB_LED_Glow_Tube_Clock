// ESP32 WROOM RGB LED Nixie Clock Test

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#include <WiFi.h>
#include <HTTPClient.h>
#include "secrets.h"

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#include "time.h"

const char* ntpServer = "pool.ntp.org"; // The Network Time Protocol Server

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
  configTime(0, 0, ntpServer); // Set the GMT and daylight offsets to zero. We need UTC, not local time.

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

  delay(25);
}

void timeToLEDs(uint8_t hour, uint8_t min, uint8_t sec)
{
  for (int i = 0; i < 60; i++) // Turn all LEDs off
    leds[i] = CRGB::Black;

  //Set hue to fraction of day.
  uint8_t greenHue = (uint8_t)((sin(((float)(((uint32_t)(hour * 60 * 60)) + ((uint32_t)(min * 60)) + ((uint32_t)(sec)))) * 1.5708 / 86400.0)) * 256.0);
  uint8_t redHue = greenHue - 85;
  uint8_t blueHue = greenHue + 85;
  uint32_t hue = (((uint32_t)greenHue) << 16) | (((uint32_t)redHue) << 8) | (((uint32_t)blueHue) << 0);
  Serial.printf("Red %d Green %d Blue %d\r\n",redHue,greenHue,blueHue);

  //Set the correct LEDs to hue
  leds[0 + (sec % 10)] = hue;
  leds[10 + (sec / 10)] = hue;
  leds[20 + (min % 10)] = hue;
  leds[30 + (min / 10)] = hue;
  leds[40 + (hour % 10)] = hue;
  leds[50 + (hour / 10)] = hue;
}
