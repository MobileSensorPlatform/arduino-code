Arduino code for reading temperatures, talking to a GPS, and logging to an SD card and via XBee

### Pin usage ###

  * USB Communication with host (for debugging keep open) on 0, 1
  * Powerboost shield: uses only 5V and GND pins.
  * Thermocouple boards (MAX31855, qty. 3)
    * CLK, MISO: on 6-pin SPI header next to Arduino logo; check pinout
    * CS: 10, 4, 52 for CS0, CS1, and CS2 respectively
  * GPS/SD logging shield
    * For GPS, switch on shield set to SoftSerial, then jumpered 7 -> 18 (TX1), 8 -> 19 (RX1) 
    * For SD card, software SPI on 13-10, using [Adafruit SD library] (https://github.com/adafruit/SD), initialized with `SD.begin(chipSelect, 11, 12, 13)`
  * XBee shield
    * DOUT: 3
    * DIN: 2
