# National Rail Departure Board

This is an arduino project which displays a departure board for a national rail
station on a 320x240 TFT screen module using an ESP32.

This uses the [Huxley API](https://huxley.unop.uk/) which the board calls directly.

This project depends on:
* [ArduinoJson](https://github.com/bblanchon/ArduinoJson)
* [TaskScheduler](https://github.com/arkhipenko/TaskScheduler)
* [Adafruit GFX](https://github.com/adafruit/Adafruit-GFX-Library)
* [Adafruit ILI9341](https://github.com/adafruit/Adafruit_ILI9341)