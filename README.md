Arduino code for reading temperatures, talking to a GPS, and logging to an SD card and via XBee

### Software prerequisites ###

Install Adafruit-SD, Adafruit-GPS, and Adafruit-MAX31855 libraries.

Tested with Arduino IDE 1.6.5. For Due support, go to Tools > Board > Boards Manager and install 32-bit Cortex M3 package.

### Pin usage ###

  * USB communication with host on 0, 1 (keep open for debugging)
  * Powerboost shield: uses only 5V and GND pins.
  * Thermocouple boards (MAX31855, qty. 3)
    * CLK, MISO: on 6-pin SPI header next to Arduino logo; check pinout
    * CS: 10, 4, 52 for CS0, CS1, and CS2 respectively
  * GPS/SD logging shield
    * For GPS, switch on shield set to SoftSerial, then jumpered 7 -> 18 (TX1), 8 -> 19 (RX1)
    * For SD card, software SPI on 13-11, 9, using [Adafruit SD library](https://github.com/adafruit/SD), initialized with `SD.begin(9, 11, 12, 13)`
    * Modify shield: cut trace right next to pin 10 on underside of shield. Jump pin 9 to hole labeled CS.
  * XBee shield
    * Switch set to DLINE
    * Blobs for pins 2, 3 desoldered
    * DOUT under XBee wired to: 17 (RX2)
    * DIN under XBee wired to: 16 (TX2)
