// OTA
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

// Blynk
#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
char auth[] = "0300c3eb91704ee48445f79e59c4d1df";
char ssid[] = "PLDTHOMEFIBRAntipoloPolice";
char pass[] = "TAGAYUNFAMILY";

// Neopixel Ring
#include <Adafruit_NeoPixel.h>
#ifdef __AVR_ATtiny85__ // Trinket, Gemma, etc.
#include <avr/power.h>
#endif

#define PIN 13

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(32, PIN);

uint8_t  mode   = 0, // Current animation effect
         offset = 0; // Position of spinny eyes
uint32_t color  = 0xFF0000; // Start red
uint32_t prevTime;

void setup()
{
  // Blynk
  Serial.begin(9600);
  Blynk.begin(auth, ssid, pass);

  // OTA
  Serial.println("Booting");
  //  WiFi.mode(WIFI_STA);
  //  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }


  // Hostname default  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // No authentication by default
  // ArduinoOTA.setPassword((const char *)"123");

  ArduinoOTA.setHostname("VJ-RGB-LRD-PixelRingGoogles");

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Neopixel Ring
#ifdef __AVR_ATtiny85__ // Trinket, Gemma, etc.
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  // Seed random number generator from an unused analog input:
  randomSeed(analogRead(2));
#else
  randomSeed(analogRead(A0));
#endif
  pixels.begin();
  pixels.setBrightness(85); // 1/3 brightness
  prevTime = millis();
}

void loop()
{
  // OTA
  ArduinoOTA.handle();
  // Blynk
  Blynk.run();

  // Pixel Ring
  
  uint8_t  i;
  uint8_t  i2;
  uint32_t t;

  switch (mode) {

    case 0:                         // Random sparks - just one LED on at a time!
      i = random(32);                //Pick Pixle at Random
      pixels.setPixelColor(i, color);//Set random pixel to current color
      pixels.show();//Update
      delay(10);
      pixels.setPixelColor(i, 0);     //Clear
      break;

    case 1: // Spinny wheels (8 LEDs on at a time)
      for (i = 0; i < 16; i++) {
        uint32_t c = 0;
        if (((offset + i) & 7) < 2) c = color; // 4 pixels on...
        pixels.setPixelColor(   i, c);        // First eye
        pixels.setPixelColor(31 - i, c);      // Second eye (flipped)
      }
      pixels.show();
      offset--;
      delay(50);
      break;

    case 2: // Spinny wheels (8 LEDs on at a time) with sparks of next Color


      for (i = 0; i < 16; i++) {
        uint32_t c = 0;
        if (((offset + i) & 7) < 2) c = color; // 4 pixels on...
        pixels.setPixelColor(   i, c);        // First eye
        pixels.setPixelColor(31 - i, c);      // Second eye (flipped)
        if (random(10) > 4)                   //adds extra pixel of a different color randomly
        {
          i2 = random(32);
          pixels.setPixelColor(i2, check(color >> 8)); //Sets random Pixel to next color
        }
      }
      pixels.show();
      pixels.setPixelColor(i2, 0);
      offset--;
      delay(50);
      break;
  }

  t = millis();
  if ((t - prevTime) > 8000) {     // Every 8 seconds...
    mode++;                        // Next mode
    if (mode > 2) {                // End of modes?
      mode = 0;                    // Start modes over
      color >>= 8;                 // Next color R->G->B
      if (!color) color = 0xFF0000; // Reset to red
    }
    for (i = 0; i < 32; i++) pixels.setPixelColor(i, 0);
    prevTime = t;
  }
}

uint32_t check(uint32_t ctest)//Used to check if current color is not 0x00 or off and if it is
//Return Red or the starting color
{
  if (ctest == 0x00)
  {
    return 0xFF0000;//returns Red
  }
  else return ctest;//returns Hex which was past to the function
}
