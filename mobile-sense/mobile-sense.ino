#include <Adafruit_GPS.h>
#include <SD.h>
#include <SPI.h>
#include <DueTimer.h>

#define CS0        10
#define CS1        4
#define CS2        52
#define SD_CARD_CS 9

#define UPDATE_FREQUENCY 1 // baby don't hertz me

#define TIMESTAMP_LENGTH   20
#define LOCATION_LENGTH    19
#define TEMPERATURE_LENGTH 24

// For hardware serial 1 (recommended):
//   GPS TX to Arduino Due Serial1 RX pin 19
//   GPS RX to Arduino Due Serial1 TX pin 18
#define GPSPort Serial1
#define GPSECHO  false
// Set GPSECHO to 'false' to turn off echoing the GPS data to the Serial console
// Set to 'true' if you want to debug and listen to the raw GPS sentences. 

/* global variables */

volatile boolean timerState = false;
boolean timerPrevState = false;

Adafruit_GPS GPS(&GPSPort);

/* end global variables */

/* functions */

void timerHandler(void) {
    timerState = !timerState;
}

void setupGPS(void) {

    Serial.println("Initializing GPS . . .");

    // 9600 NMEA is the default baud rate for Adafruit MTK GPS's- some use 4800
    GPS.begin(9600);
    GPSPort.begin(9600);

    // uncomment this line to turn on RMC (recommended minimum) and GGA (fix data) including altitude
    GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
    // uncomment this line to turn on only the "minimum recommended" data
    //GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
    // For parsing data, we don't suggest using anything but either RMC only or RMC+GGA since
    // the parser doesn't care about other sentences at this time

    // Set the update rate
    GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);   // 1 Hz update rate
    // For the parsing code to work nicely and have time to sort thru the data, and
    // print it out we don't suggest using anything higher than 1 Hz

    // Request updates on antenna status, comment out to keep quiet
    GPS.sendCommand(PGCMD_ANTENNA);

    delay(1000);
    // Ask for firmware version
    GPSPort.println(PMTK_Q_RELEASE);

    Serial.println("GPS initialized.");

}

void setupSDCard(void) {
    Serial.println("Initializing SD card . . .");

    pinMode(SD_CARD_CS, OUTPUT);

    if (!SD.begin(9, 11, 12, 13)) {
        Serial.println("Card init. failed!");
    }
    Serial.println("Initialized SD card.");
}

void setupThermocouples(void) {
    Serial.println("Initializing thermocouples . . .");

    SPI.begin(CS0);
    SPI.begin(CS1);
    SPI.begin(CS2);

    Serial.println("Thermocouples initialized.");
}

void setupXBee(void) {
    Serial.println("Initializing XBee . . .");
    Serial2.begin(9600);
    Serial.println("XBee initialized.");
}

void setupTimer(void) {
    Timer3.attachInterrupt(timerHandler);
    Timer3.setFrequency(UPDATE_FREQUENCY);
    Timer3.start();
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

    float fahrenheit = 32.0 + 9.0 * temp1 / (4.0 * 5.0); // the 4.0 division is a bit shift, not part of C -> F conversion

    return fahrenheit;
}

/* end functions */

void setup()  
{
    Serial.begin(115200);

    setupGPS();
    setupSDCard();
    setupThermocouples();
    setupXBee();

    setupTimer();
}

void loop()
{
    // read data from the GPS in the 'main loop'
    char c = GPS.read();
    // if you want to debug, this is a good time to do it!
    if (GPSECHO) {
        if (c) Serial.print(c);
    }

    // if a sentence is received, we can check the checksum, parse it...
    if (GPS.newNMEAreceived()) {
        // a tricky thing here is if we print the NMEA sentence, or data
        // we end up not listening and catching other sentences!
        if (!GPS.parse(GPS.lastNMEA())) { // this also sets the newNMEAreceived() flag to false
            return;  // we can fail to parse a sentence in which case we should just wait for another
        }
    }

    // approximately every 2 seconds or so, print out the current stats
    if (timerState != timerPrevState) { 
        timerPrevState = timerState;

        int timestamp_len = 0;
        int location_len = 0;
        int temperature_len = 0;
        int output_len = 0;

        char timestamp[TIMESTAMP_LENGTH + 1];
        char location[LOCATION_LENGTH + 1];
        char temperatures[TEMPERATURE_LENGTH + 1];
        char output[100];

        File dataFile = SD.open("datalog.txt", FILE_WRITE);
        if(!dataFile) {
            Serial.print("Couldn't open log file for writing.\r\n");
        }

        timestamp_len = sprintf(timestamp, "20%02d-%02d-%02dT%02d:%02d:%02dZ", GPS.year, GPS.month, GPS.day, GPS.hour, GPS.minute, GPS.seconds);
        if(GPS.fix) {
            location_len = sprintf(location, "%6.2f%c | %6.2f%c", GPS.latitude, GPS.lat, GPS.longitude, GPS.lon);
        } else {
            location_len = sprintf(location, "Location | unknown ");
        }
        temperature_len = sprintf(temperatures, "%6.2f | %6.2f | %6.2f", readThermocouple(CS0), readThermocouple(CS1), readThermocouple(CS2));

        if(timestamp_len == TIMESTAMP_LENGTH && location_len == LOCATION_LENGTH && temperature_len == TEMPERATURE_LENGTH) {
            output_len = sprintf(output, "%s | %s | %s\r\n", timestamp, location, temperatures);
        } else {
            output_len = sprintf(output, "Something wrong with buffer lengths: %d, %d, %d\n", timestamp_len, location_len, temperature_len);
        }
        Serial.print(output);
        Serial2.print(output); // send to XBee
        if(dataFile) {
            dataFile.println(output);
            dataFile.close();
        }
    }
}
