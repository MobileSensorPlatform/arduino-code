/*************************************************** 
  This is an example for the Adafruit Thermocouple Sensor w/MAX31855K

  Designed specifically to work with the Adafruit Thermocouple Sensor
  ----> https://www.adafruit.com/products/269

  These displays use SPI to communicate, 3 pins are required to  
  interface
  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#include <SPI.h>
#include "Adafruit_MAX31855.h"

#define CS_A   9
#define CS_B   6
#define CS_C   5

Adafruit_MAX31855 thermoA(CS_A);
Adafruit_MAX31855 thermoB(CS_B);
Adafruit_MAX31855 thermoC(CS_C);

void setup() {
  Serial.begin(9600);
  
  Serial.println("MAX31855 test");
  // wait for MAX chip to stabilize
  delay(500);
}

void loop() {
  // basic readout test, just print the current temp
   Serial.print("Thermo A Internal Temp (in Celsius) = ");
   Serial.println(thermoA.readInternal());

   double f = thermoA.readFarenheit();
   if (isnan(f)) {
     Serial.println("Something wrong with thermocouple!");
   } else {
     Serial.print("F = "); 
     Serial.println(f);
   } 
   delay(1000);
}
