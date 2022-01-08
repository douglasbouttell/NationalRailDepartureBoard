#ifndef __DEPARTURE_BOARD_HPP__
#define __DEPARTURE_BOARD_HPP__

#include <Logger.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

struct DepartureBoardEntry {
  const char* time;
  const char* destination;
  const char* via;
  const char* platform;
  const char* estimated;
  boolean highlight;
};

class DepartureBoard {
  private:
    Adafruit_ILI9341& tft;
    const char* locationName;
    DepartureBoardEntry entries[20];
    GFXcanvas1 line_canvas;
    int animationTick;
  
  public:
    DepartureBoard(Adafruit_ILI9341& tft) 
      : tft(tft)
      , locationName(NULL)
      , entries{0}
      , line_canvas(320, 10) 
      , animationTick(0)
    {}

    void init() {
      Logger::notice("DepartureBoard::init()", "initializing");
      tft.begin();
      tft.setRotation(1);
      tft.fillScreen(ILI9341_BLACK);
      tft.setTextWrap(false);
    }

    void setLocationName(const char* str) {
      locationName = str;
    }

    void setEntry(
      uint8_t i, 
      const char* time, 
      const char* destination, 
      const char* via,
      const char* platform,
      const char* estimated,
      boolean highlight
    ) {
      if (i < 0 || i >= 20) {
        return;
      }

      entries[i].time = time;
      entries[i].destination = destination;
      entries[i].via = via;
      entries[i].platform = platform;
      entries[i].estimated = estimated;
      entries[i].highlight = highlight;
    }

    void clearEntry(uint8_t i) {
      setEntry(i, NULL, NULL, NULL, NULL, NULL, false);
    }

    void tick() {
      animationTick++;
      if (animationTick > 1) {
        animationTick = 0;
      }
    }

    void tick(int value) {
      animationTick = value;
    }

    void draw_header() {
      tft.fillRect(0, 0, tft.width(), 30, ILI9341_BLUE);
      
      tft.setTextColor(ILI9341_WHITE, ILI9341_BLUE);
      tft.setCursor(2, 2);
      tft.setTextSize(2);
      if (locationName) {
        tft.print(locationName);
        tft.print(" departures");
      }
      
      tft.setTextSize(1);
      tft.setCursor(2, 20);
      tft.print("Time");
      tft.setCursor(50, 20);
      tft.print("Destination");
      tft.setCursor(230, 20);
      tft.print("Plat");
      tft.setCursor(260, 20);
      tft.print("Expected");
    }

    void draw_body(boolean force) {
      int i;    
      for (i = 0; i < 20; ++i) {
        uint16_t y_offset = 30 + (i*10);

        if (!force && !entries[i].via) {
          /* Don't update anything since we have nothing to animate */
          continue;
        }
        
        // Clear the line
        line_canvas.fillRect(0, 0, 320, 10, ILI9341_BLACK);

        line_canvas.setCursor(2, 1);
        if (entries[i].time) {
          line_canvas.print(entries[i].time);
        }
        
        line_canvas.setCursor(50, 1);
        if (entries[i].destination && animationTick == 0) {
          line_canvas.print(entries[i].destination);
        } else if (entries[i].via && animationTick == 1) {
          line_canvas.print(entries[i].via);
        }
        
        line_canvas.setCursor(230, 1);
        if (entries[i].platform) {
          line_canvas.print(entries[i].platform);
        }
        
        line_canvas.setCursor(260, 1);
        if (entries[i].estimated) {
          line_canvas.print(entries[i].estimated);
        }      
        
        if (entries[i].highlight) {
          tft.drawBitmap(0, y_offset, line_canvas.getBuffer(), 320, 10, ILI9341_BLACK, ILI9341_YELLOW);
        } else {    
          tft.drawBitmap(0, y_offset, line_canvas.getBuffer(), 320, 10, ILI9341_YELLOW, ILI9341_BLACK);
        }
      }
      
      tft.setTextColor(ILI9341_WHITE, ILI9341_BLUE);
      tft.fillRect(0, 230, tft.width(), 240, ILI9341_BLUE);
      tft.setCursor(2, 231);
      tft.print("Page 1 of 1");
    }

    void draw_time(const char* str) {
      tft.setTextColor(ILI9341_WHITE, ILI9341_BLUE);
      tft.setCursor(270, 231);
      tft.print(str);
    }
};

#endif // __DEPARTURE_BOARD_HPP__
