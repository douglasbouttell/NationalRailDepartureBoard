#ifndef __DEPARTURE_BOARD_HPP__
#define __DEPARTURE_BOARD_HPP__

#include <Logger.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

struct DepartureBoardEntry {
  const char* time;
  const char* destination;
  const char* platform;
  const char* estimated;
  boolean highlight;
};

class DepartureBoard {
  private:
    Adafruit_ILI9341& tft;
    const char* locationName;
    const char* time;
    DepartureBoardEntry entries[20];
    GFXcanvas1 line_canvas;
  
  public:
    DepartureBoard(Adafruit_ILI9341& tft) 
      : tft(tft)
      , locationName(NULL)
      , time(NULL)
      , entries{0}
      , line_canvas(320, 10) 
    {}

    void init() {
      Logger::notice("DepartureBoard::init()", "initializing");
      this->tft.begin();
      this->tft.setRotation(1);
      this->tft.fillScreen(ILI9341_BLACK);
      this->tft.setTextWrap(false);
    }

    void setLocationName(const char* str) {
      this->locationName = str;
    }

    void setTime(const char* time) {
      this->time = time;
    }

    void setEntry(
      uint8_t i, 
      const char* time, 
      const char* destination, 
      const char* platform,
      const char* estimated,
      boolean highlight
    ) {
      if (i < 0 || i >= 20) {
        return;
      }

      this->entries[i].time = time;
      this->entries[i].destination = destination;
      this->entries[i].platform = platform;
      this->entries[i].estimated = estimated;
      this->entries[i].highlight = highlight;
    }

    void clearEntry(uint8_t i) {
      this->setEntry(i, NULL, NULL, NULL, NULL, false);
    }

    void draw() {
      int i;
      
      this->tft.fillRect(0, 0, this->tft.width(), 30, ILI9341_BLUE);
      
      this->tft.setTextColor(ILI9341_WHITE, ILI9341_BLUE);
      this->tft.setCursor(2, 2);
      this->tft.setTextSize(2);
      if (this->locationName) {
        this->tft.print(this->locationName);
        this->tft.print(" departures");
      }
      
      this->tft.setTextSize(1);
      this->tft.setCursor(2, 20);
      this->tft.print("Time");
      this->tft.setCursor(50, 20);
      this->tft.print("Destination");
      this->tft.setCursor(230, 20);
      this->tft.print("Plat");
      this->tft.setCursor(260, 20);
      this->tft.print("Expected");
    
      for (i = 0; i < 20; ++i) {
        uint16_t y_offset = 30 + (i*10);
        // Clear the line
        this->line_canvas.fillRect(0, 0, 320, 10, ILI9341_BLACK);

        if (this->entries[i].time) {
          this->line_canvas.setCursor(2, 1);
          this->line_canvas.print(this->entries[i].time);
        }
        if (this->entries[i].destination) {
          this->line_canvas.setCursor(50, 1);
          this->line_canvas.print(this->entries[i].destination);
        }
        if (this->entries[i].platform) {
          this->line_canvas.setCursor(230, 1);
          this->line_canvas.print(this->entries[i].platform);
        }
        if (this->entries[i].estimated) {
          this->line_canvas.setCursor(260, 1);
          this->line_canvas.print(this->entries[i].estimated);
        }      
        if (this->entries[i].highlight) {
          this->tft.drawBitmap(0, y_offset, this->line_canvas.getBuffer(), 320, 10, ILI9341_BLACK, ILI9341_YELLOW);
        } else {    
          this->tft.drawBitmap(0, y_offset, this->line_canvas.getBuffer(), 320, 10, ILI9341_YELLOW, ILI9341_BLACK);
        }
      }
      
      this->tft.setTextColor(ILI9341_WHITE, ILI9341_BLUE);
      this->tft.fillRect(0, 230, tft.width(), 240, ILI9341_BLUE);
      this->tft.setCursor(2, 231);
      this->tft.print("Page 1 of 1");
    }

    void draw_time() {
      this->tft.setTextColor(ILI9341_WHITE, ILI9341_BLUE);
      this->tft.setCursor(270, 231);
      this->tft.print(this->time);
    }
};

#endif // __DEPARTURE_BOARD_HPP__
