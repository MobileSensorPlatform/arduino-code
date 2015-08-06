/* MAX31855 thermocouple amp with Arduino Due */
/* test code based on http://forums.adafruit.com/viewtopic.php?f=25&t=45336#p364994 */

#include <SPI.h>

#define CS2 52

void setup() {
    Serial.begin(9600);
    SPI.begin(CS2);
}

double readThermocouple(int slaveSelectPin)
{
    byte data1 = SPI.transfer(slaveSelectPin, 0, SPI_CONTINUE);
    byte data2 = SPI.transfer(slaveSelectPin, 0, SPI_CONTINUE);
    byte data3 = SPI.transfer(slaveSelectPin, 0, SPI_CONTINUE);
    byte data4 = SPI.transfer(slaveSelectPin, 0, SPI_LAST);

    word temp1 = word(data1, data2);
    word temp2 = word(data3, data4);

    bool ned = false;
    if (temp1 &0x8000){
        ned = true;
    }

    if (temp1 & 0x1)
    {
        Serial.println("Thermocouple error!");
    if (temp2 & 0x1)
        Serial.println("Open circuit");
    if (temp2 & 0x2)
        Serial.println("VCC Short");
    if (temp2 & 0x4)
        Serial.println("GND Short");
    }

    temp1 &= 0x7FFC;
    temp1 >>= 2;
    double celcius = temp1;

    double temp = celcius / 4;

    return temp;
}

void loop() {
    Serial.println(readThermocouple(CS2), 2);
    delay(500);
}
