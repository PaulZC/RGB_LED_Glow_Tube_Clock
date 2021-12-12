// ESP32 WROOM RGB LED Nixie Clock Test - use the VEML7700 to adjust the LED brightness

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

#define maxBrightness 255
#define minBrightness 25
int brightness = maxBrightness;

CRGB leds[NUM_LEDS];

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#include <SparkFun_VEML7700_Arduino_Library.h> // Click here to get the library: http://librarymanager/All#SparkFun_VEML7700

VEML7700 mySensor; // Create a VEML7700 object

#define minLux 5 // The lux below which the LEDs will be at minBrightness
#define maxLux 75 // The lux at which the LEDs will reach maxBrightness
bool sensorOnline;

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void setup() {
  
  delay(1000);
  
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness( brightness );
  FastLED.clear(true); // Turn all the LEDs off

  Serial.begin(115200);
  Serial.println(F("RGB LED Nixie Clock"));

  Wire.begin(21, 22); // Tell Wire which pins to use (the SparkFun ESP32 Thing Plus defaults to 23 and 22)
  sensorOnline = mySensor.begin();
  if (sensorOnline == false)
  {
    Serial.println("Unable to communicate with the VEML7700!");
  }
  else
  {
    setBrightness();
  }

  setClock();

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

      if ((timeinfo.tm_hour == 1) && (timeinfo.tm_min == 0) && (timeinfo.tm_sec == 0)) // Resync the clock at 1AM
      //if (timeinfo.tm_sec == 0) // Resync the clock every minute - for testing
      {
        setClock();
      }
      else if ((timeinfo.tm_min % 15 == 0) && (timeinfo.tm_sec == 0)) // Set the LED brightness every 15 mins
      //else if (timeinfo.tm_sec %10 == 0) // Set the LED brightness every 10 seconds - for testing
      {
        setBrightness();
      }
      else
      {
        timeToLEDs(timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
      }
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

void setClock()
{
  FastLED.clear(true); // Turn all the LEDs off
  FastLED.show();

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Connect to WiFi.

  Serial.print(F("Connecting to local WiFi"));

  WiFi.begin(ssid, password);

  unsigned long startTime = millis();
  
  while ((WiFi.status() != WL_CONNECTED) && (millis() < (startTime + 15000))) // Timeout after 15 seconds
  {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println();

  if (millis() >= (startTime + 10000))
  {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    Serial.println(F("WiFi connection timed out!"));
    return;
  }

  Serial.println(F("WiFi connected"));

  //=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Set the RTC using network time. (Code taken from the SimpleTime example.)

  // Request the time from the NTP server and use it to set the ESP32's RTC.
  configTime(0, 3600, ntpServer); // Set the GMT offset to 0, and the daylight offset to 0 or 3600.

  struct tm timeinfo;
  if(getLocalTime(&timeinfo))
  {
    Serial.println(&timeinfo, "Network time is: %A, %B %d %Y %H:%M:%S");
  }
  else
  {
    Serial.println("Failed to obtain time!");
  }

  //=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Disconnect the WiFi as it's no longer needed

  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  Serial.println(F("WiFi disconnected"));  
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

void setBrightness()
{
  FastLED.clear(true); // Turn all the LEDs off
  FastLED.show();

  if (!sensorOnline) // Return now if the VEML7700 is offline
    return;
    
  delay(500); // Wait for half a second for the VEML7700 to settle

  // Read the lux four times and average it
  float lux = 0.0;
  for (int i = 0; i < 4; i++)
  {
    lux += mySensor.getLux();
    delay(125);
  }
  lux /= 4.0;
  Serial.print("Lux is ");
  Serial.println(lux);

  // Now scale brightness
  if (lux <= minLux)
  {
    brightness = minBrightness;
  }
  else if (lux >= maxLux)
  {
    brightness = maxBrightness;
  }
  else
  {
    lux = (lux - minLux) / (maxLux - minLux); // Convert lux to a fraction of the lux range
    brightness = minBrightness;
    brightness += (int)((float)(maxBrightness - minBrightness) * lux);
    if (brightness > maxBrightness)
      brightness = maxBrightness;
  }
  Serial.print("Brightness is ");
  Serial.println(brightness);

  FastLED.setBrightness( brightness );
}
