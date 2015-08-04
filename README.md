Arduino code for reading temperatures, talking to a GPS, and logging to an SD card and via XBee

### Pin usage ###

  * USB Communication with host (for debugging keep open) on 0, 1
  * Powerboost shield: uses only 5V and GND pins.
  * Thermocouple boards (MAX31855, qty. 3)
    * CLK: 13
    * MISO: 12
    * CS: 9, 6, 5
  * GPS/SD logging shield
    * For GPS, software serial on 7, 8
    * For SD card, SPI on 13-10
  * XBee shield
    * DOUT: 3
    * DIN: 2
