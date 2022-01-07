#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include "time.h"
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

#ifdef ESP8266
   #define STMPE_CS 16
   #define TFT_CS   0
   #define TFT_DC   15
   #define SD_CS    2
#elif defined(ESP32) && !defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S2)
   #define STMPE_CS 32
   #define TFT_CS   15
   #define TFT_DC   33
   #define SD_CS    14
#elif defined(TEENSYDUINO)
   #define TFT_DC   10
   #define TFT_CS   4
   #define STMPE_CS 3
   #define SD_CS    8
#elif defined(ARDUINO_STM32_FEATHER)
   #define TFT_DC   PB4
   #define TFT_CS   PA15
   #define STMPE_CS PC7
   #define SD_CS    PC5
#elif defined(ARDUINO_NRF52832_FEATHER)  /* BSP 0.6.5 and higher! */
   #define TFT_DC   11
   #define TFT_CS   31
   #define STMPE_CS 30
   #define SD_CS    27
#elif defined(ARDUINO_MAX32620FTHR) || defined(ARDUINO_MAX32630FTHR)
   #define TFT_DC   P5_4
   #define TFT_CS   P5_3
   #define STMPE_CS P3_3
   #define SD_CS    P3_2
#else
    // Anything else, defaults!
   #define STMPE_CS 6
   #define TFT_CS   9
   #define TFT_DC   10
   #define SD_CS    5
#endif

#define STATION "edb"
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 0;


Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

WiFiMulti WiFiMulti;

HTTPClient http;
DynamicJsonDocument doc(4096);
DynamicJsonDocument departures_filter(200);

GFXcanvas1 line_canvas(320, 10);

struct tm timeinfo;
uint8_t lastUpdate = 255;

void tft_setup() {
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextWrap(false);

  // Draw the header
}

void wifi_setup() {
  // TODO: Setup Improv over serial for Wifi Connection
  WiFiMulti.addAP("NETGEAR69", "strongtrumpet231");
  while(WiFiMulti.run() != WL_CONNECTED) {
      Serial.print(".");
      delay(500);
  }
}

void get_train_times() {
  Serial.printf("[HTTP] GET\n");
  http.begin("http://huxley.apphb.com/departures/" STATION "/20?accessToken=DA1C7740-9DA0-11E4-80E6-A920340000B1");
  int httpCode = http.GET();
  
  if(httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    
    Serial.println(F("[HTTP] GET.. OK"));
    
    DeserializationError error = deserializeJson(doc, payload, DeserializationOption::Filter(departures_filter));
    if (error) {
      Serial.print(F("[HTTP] GET.. deserializeJson() failed: "));
      Serial.println(error.f_str());
    }
  } else {
      Serial.print("[HTTP] GET... failed: ");
      Serial.println(http.errorToString(httpCode).c_str());
  }
  
  http.end();
}

void draw_everything() {
  int i;
  
  //tft.fillScreen(ILI9341_BLACK);
  tft.fillRect(0, 0, tft.width(), 30, ILI9341_BLUE);
  
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLUE);
  tft.setCursor(2, 2);
  tft.setTextSize(2);
  tft.print(doc["locationName"].as<char*>());
  tft.print(" departures");
  
  tft.setTextSize(1);
  tft.setCursor(2, 20);
  tft.print("Time");
  tft.setCursor(50, 20);
  tft.print("Destination");
  tft.setCursor(230, 20);
  tft.print("Plat");
  tft.setCursor(260, 20);
  tft.print("Expected");

  for (i = 0; i < 20; ++i) {
    uint16_t y_offset = 30 + (i*10);
    if (i < doc["trainServices"].size()) {
      // Clear the line
      line_canvas.fillRect(0, 0, 320, 10, ILI9341_BLACK);
  
      line_canvas.setCursor(2, 1);
      line_canvas.print(doc["trainServices"][i]["std"].as<char*>());
      line_canvas.setCursor(50, 1);
      line_canvas.print(doc["trainServices"][i]["destination"][0]["locationName"].as<char*>());
      line_canvas.setCursor(230, 1);
      line_canvas.print(doc["trainServices"][i]["platform"].as<char*>());
      line_canvas.setCursor(260, 1);
      line_canvas.print(doc["trainServices"][i]["etd"].as<char*>());
  
      // Highlight Glasgow Queen Street Lines
      if (strncmp(doc["trainServices"][i]["destination"][0]["crs"].as<char*>(), "GLQ", 3) == 0) {
        tft.drawBitmap(0, y_offset, line_canvas.getBuffer(), 320, 10, ILI9341_BLACK, ILI9341_YELLOW);
      } else {    
        tft.drawBitmap(0, y_offset, line_canvas.getBuffer(), 320, 10, ILI9341_YELLOW, ILI9341_BLACK);
      }
    } else {
      // No data
      tft.fillRect(0, y_offset, tft.width(), 10, ILI9341_BLACK);
    }
  }
  
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLUE);
  tft.fillRect(0, 230, tft.width(), 240, ILI9341_BLUE);
  tft.setCursor(2, 231);
  tft.print("Page 1 of 1");
}


void draw_the_time() {
  char timestr[9];
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLUE);
  strftime(timestr, sizeof(timestr), "%H:%M:%S", &timeinfo);
  tft.setCursor(270, 231);
  tft.print(timestr);
}

void setup() {
  departures_filter["locationName"] = true;
  departures_filter["trainServices"][0]["destination"][0]["locationName"] = true;
  departures_filter["trainServices"][0]["destination"][0]["crs"] = true;
  departures_filter["trainServices"][0]["std"] = true;
  departures_filter["trainServices"][0]["etd"] = true;
  departures_filter["trainServices"][0]["platform"] = true;
    
  Serial.begin(115200);
  tft_setup();
  wifi_setup();

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  pinMode(13, OUTPUT);
}

void loop() {
  if (lastUpdate >= 60) {
    lastUpdate = 0;
    digitalWrite(13, HIGH);
    get_train_times();
    draw_everything();
    
    digitalWrite(13, LOW);
  } else {
    lastUpdate++;
  }

  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
  } else {
    draw_the_time();
  }
  
  delay(1000);
}
