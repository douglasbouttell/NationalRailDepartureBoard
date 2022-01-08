#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include "time.h"
#include <WiFi.h>
#include <WiFiMulti.h>
// https://arduinojson.org/
#include <ArduinoJson.h>
// https://github.com/arkhipenko/TaskScheduler
#include <TaskScheduler.h>

#include "wifi_settings.h"
#include "DepartureBoard.hpp"
#include "HuxleyAPI.hpp"

/**
 * JSON parser and filter for the Huxley API
 */
class HuxleyDeparturesJSONParser {
  private:
    DynamicJsonDocument filter;
    DeserializationError err;

  public:
    HuxleyDeparturesJSONParser() : filter(200), err(DeserializationError::Ok) {
      this->filter["locationName"] = true;
      this->filter["trainServices"][0]["destination"][0]["locationName"] = true;
      this->filter["trainServices"][0]["destination"][0]["crs"] = true;
      this->filter["trainServices"][0]["std"] = true;
      this->filter["trainServices"][0]["etd"] = true;
      this->filter["trainServices"][0]["platform"] = true;
    }

    DeserializationError getError() {
      return this->err;
    }

    bool parse(DynamicJsonDocument& json, String& payload) {
      if (payload.length() > 0) {
        this->err = deserializeJson(json, payload, DeserializationOption::Filter(this->filter));
        if (!this->err) {
          return true;
        }
      }
      return false;
    }
};

/* Pin allocations -------------------------------- */
const int   LED                = 13;
const int   TFT_CS             = 15;
const int   TFT_DC             = 33;
/* Magic Constants -------------------------------- */
//const char* WIFI_SSID        // See wifi_settings.h
//const char* WIFI_PASSWORD    // See wifi_settings.h
const char* STATION            = "edb";
const char* HIGHLIGHT_STATION  = "GLQ";
const char* NTP_SERVER         = "pool.ntp.org";
const long  NTP_GTMOFFSET      = 0;
const int   NTP_DAYLIGHTOFFSET = 0;
/* ------------------------------------------------ */

WiFiMulti                  wifi;
HuxleyAPI                  api;
HuxleyDeparturesJSONParser parser;
DynamicJsonDocument        json       = DynamicJsonDocument(4096);
Adafruit_ILI9341           tft        = Adafruit_ILI9341(TFT_CS, TFT_DC);
DepartureBoard             board      = DepartureBoard(tft);
struct tm                  timeinfo;
char                       timestr[9] = {0};
uint8_t                    nextUpdate = 0;

void getTrainTimes();
void drawTime();

Scheduler sched;
Task taskGetTrainTimes(60000, TASK_FOREVER, &getTrainTimes, &sched, true);
Task taskDrawTime(1000, TASK_FOREVER, &drawTime, &sched, true);

boolean callAPI() {
  int i;
  int returnedSize;
  Serial.println("I: Calling API");
  
  if (api.get(STATION)) {
    String payload = api.getBody();
    Serial.println("I: API Call OK");
    if (parser.parse(json, payload)) {
      board.setLocationName(json["locationName"].as<char*>());
      returnedSize = json["trainServices"].size();
      Serial.print("I: Got ");
      Serial.print(returnedSize);
      Serial.println(" entries");
      
      for (i = 0; i < 20; ++i) {
        if (i < returnedSize) {
          Serial.print("I: ");
          Serial.print(i);
          Serial.print(" -> ");
          Serial.print(json["trainServices"][i]["std"].as<char*>());
          Serial.print(" ");
          Serial.print(json["trainServices"][i]["destination"][0]["locationName"].as<char*>());
          Serial.print(" (");
          Serial.print(json["trainServices"][i]["destination"][0]["crs"].as<char*>());
          Serial.println(")");
          
          board.setEntry(
            i, 
            json["trainServices"][i]["std"].as<char*>(),
            json["trainServices"][i]["destination"][0]["locationName"].as<char*>(),
            json["trainServices"][i]["platform"].as<char*>(),
            json["trainServices"][i]["etd"].as<char*>(),
            strncmp(json["trainServices"][i]["destination"][0]["crs"].as<char*>(), HIGHLIGHT_STATION, 3) == 0
          );
        } else {
          board.clearEntry(i);
        }
      }
      return true;
    } else {
      Serial.print("E: Deserialization failed: ");
      Serial.println(parser.getError().c_str());
    }
  } else {
    Serial.print("E: API failed: ");
    Serial.println(api.getErrorString().c_str());
  }
  return false;
}

/* TASK */
void getTrainTimes() {
  Serial.println("I: TASK: getTrainTimes");
  digitalWrite(LED, HIGH);
  if (callAPI()) {
    Serial.println("I: Drawing");
    board.draw();
  }
  digitalWrite(LED, LOW);
}

/* TASK */
void drawTime() {
  Serial.println("I: TASK: drawTime");
  if(getLocalTime(&timeinfo)){
    strftime(timestr, sizeof(timestr), "%H:%M:%S", &timeinfo);
    board.setTime(timestr);
    board.draw_time();
  } else {
    Serial.println("E: Failed to get time");
  }
}

void setup() {
  Serial.begin(115200);

  /* Initialize the departure board */
  /* This clears the screen and sets the rotation (landscape) */
  board.init();

  /* TODO: Setup Improv over serial for Wifi Connection */
  wifi.addAP(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("I: WiFi Connecting to ");
  Serial.println(WIFI_SSID);
  while(wifi.run() != WL_CONNECTED) { delay(500); }
  Serial.println("I: WiFi Connected");

  /* Set up NTP */
  configTime(NTP_GTMOFFSET, NTP_DAYLIGHTOFFSET, NTP_SERVER);

  /* Allow the LED to flash */
  pinMode(LED, OUTPUT);
  
  Serial.println("I: Starting Scheduler");
  sched.startNow(); 
}

void loop() {
  sched.execute();
}
