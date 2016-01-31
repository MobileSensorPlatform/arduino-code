/* * * * * * * * * * * * * * * * * * * * * * * *
*      Mobile Sensor Platform Arduino DUE      *
*   Comments coming soon to a commit near you  *
*                                              *
* * * * * * * * * * * * * * * * * * * * * * * */

/* Include Libraries */
#include <DueTimer.h>
#include <SD.h>
#include <SPI.h>
#include <Adafruit_GPS.h>

/* Program Settings */
#define UPDATE_FREQUENCY 1 // baby don't hertz me
#define ARDUINO_NAME "BRD3"

/*
 * Here ye'! Proceed with caution!
 */


/* Miscellaneous Definitions and Functions */
#define debugPort Serial
void debugPrint(char *txt, ...) {
  char buf[128];
  sprintf(buf, "%lu\t| %s", (unsigned uint32_t) millis(), txt);
  debugPort.println(buf);
}

char* byte_to_binary( byte x )
{
    static char b[8+1] = {0};
    int y;
    long long z;
    for (z=1LL<<7,y=0; z>0; z>>=1,y++)
    {
        b[y] = ( ((x & z) == z) ? '1' : '0');
    }

    b[y] = 0;

    return b;
}

/* Timer Definitions and Functions */
#define TIMER Timer3
int timerLED = 13; // standard Arduino on-board LED pin
volatile bool timerState = false;
bool timerPrevState = false;

void timerHandler(void) {
    timerState = !timerState;
    digitalWrite(timerLED, timerState);
}

void setupTimer(void) {
    pinMode(timerLED, timerState);

    TIMER.attachInterrupt(timerHandler);
    TIMER.setFrequency(UPDATE_FREQUENCY);
    TIMER.start();
}

/* Thermocouple Definitions and Function */
#define CS0 10
#define CS1 4
#define CS2 52
byte thermocoupleState = B00000000;

void setupThermocouples(void){
    SPI.begin(CS0);
    SPI.begin(CS1);
    SPI.begin(CS2);
}

double readThermocouple(short cspin) {
    byte data1 = SPI.transfer(cspin, 0, SPI_CONTINUE);
    byte data2 = SPI.transfer(cspin, 0, SPI_CONTINUE);
    byte data3 = SPI.transfer(cspin, 0, SPI_CONTINUE);
    byte data4 = SPI.transfer(cspin, 0, SPI_LAST);

    word temp1 = word(data1, data2);
    word temp2 = word(data3, data4);

    byte state = B00;
    if (temp1 & 0x1)
    {
        debugPrint("Thermocouple error!");
        if (temp2 & 0x1){
            //("Open circuit");
            state = B01;
        }
        if (temp2 & 0x2){
            //("VCC Short");
            state = B10;
        }
        if (temp2 & 0x4){
            //("GND Short");
            state = B10;
        }
        state = B11;
    }
    if (cspin == CS0) {
        thermocoupleState &= ~(B11);
        thermocoupleState |= state;
    } else if (cspin == CS1) {
        thermocoupleState &= ~(B11 << 2);
        thermocoupleState |= state << 2;
    } else if (cspin == CS2) {
        thermocoupleState &= ~(B11 << 4);
        thermocoupleState |= state << 4;
    }

    temp1 &= 0x7FFC;
    temp1 >>= 2;

    //double celcius = (temp1 / 4);
    double fahrenheit = (32.0 + 9.0 * temp1) / (4.0 * 5.0);

    return fahrenheit;
}

/* SD-Card Definition and Functions */
#define SDCS 44 
#define SDCLK_AND_LED 50
#define SDMISO 48
#define SDMOSI 46

File logfile;
boolean logging = false;
boolean SDActive = false;
File dataFile;

void setupSD(){
    pinMode(SDCLK_AND_LED, OUTPUT);
    pinMode(SDCS, OUTPUT);
    
    SDActive = SD.begin(SDCS, SDMOSI, SDMISO, SDCLK_AND_LED);
    dataFile = SD.open("datalog.txt", FILE_WRITE);
}

/* GPS Definitions and Functions */
#define GPSPort Serial1
#define GPSECHO false

Adafruit_GPS GPS(&GPSPort);

void setupGPS(void) {
    GPS.begin(9600);
    GPSPort.begin(9600);
}

/* XBee Definitions and Functions */
#define XBeePort Serial2
void setupXBee(void) {
    XBeePort.begin(9600);
}

/* System Status */
/*  *  *  *  *  *  *  *  *  *  *  *  *  *
  Status status:                    
    SYSTEMS OK:             0x0000
    MISSING GPS:            0x0001
    MISSING SD:             0x0010 
    MISSING XBEE/WIFI:      0x0100 
    MISSING FILELOG:        0x1000  
*  *  *  *  *  *  *  *  *  *  *  *  *  */
byte systemState = B000000;

byte systemStatus(void) {
    byte newStatus = B000000;

    if (!GPSPort)
        systemState |= 1;
    if (!SDActive)
        systemState |= 1 << 1;
    if (!XBeePort)
        systemState |= 1 << 2;
    if (!dataFile)
        systemState |= 1 << 3;

    return systemState;
}


/* Main Arduino Setup and Loop */
void setup() {
    debugPort.begin(9600);
    delay(10);
    debugPrint("Initializing Startup Sequence . . .");

    setupGPS();
    setupSD();
    setupThermocouples();
    setupXBee();
    //setupESP8266();

    setupTimer();
    debugPrint("All systems are go. Please enjoy your stay.");
}

#define TIMESTAMP_LENGTH 20
#define LOCATION_LENGTH 19
#define STATES_LENGTH 19
#define TEMPERATURE_LENGTH 24

void loop() {
    // read data from the GPS in the 'main loop'
    char c = GPS.read();

    // if a sentence is received, we can check the checksum, parse it...
    if (GPS.newNMEAreceived()) {
        // a tricky thing here is if we print the NMEA sentence, or data
        // we end up not listening and catching other sentences!
        if (!GPS.parse(GPS.lastNMEA())) { // this also sets the newNMEAreceived() flag to false
            return;  // we can fail to parse a sentence in which case we should just wait for another
        }
    }
    
    if (timerState != timerPrevState){
        timerPrevState = timerState;
        
        systemStatus();
        // structure and send data
        int timestamp_len = 0;
        int location_len = 0;
        int states_len = 0;
        int temperature_len = 0;
        int output_len = 0;

        char timestamp[TIMESTAMP_LENGTH + 1];
        char location[LOCATION_LENGTH + 1];
        char states[STATES_LENGTH + 1];
        char temperatures[TEMPERATURE_LENGTH + 1];
        char output[156];

        if(!dataFile) {
            Serial.print("Couldn't open log file for writing.\r\n");
        }

        char sysbuf[9];
        char tcbuf[9];
        itoa(systemState, sysbuf, 2);
        itoa(thermocoupleState, tcbuf, 2);
        states_len = sprintf(states, "%s | %s", sysbuf, tcbuf);

        timestamp_len = sprintf(timestamp, "20%02d-%02d-%02d %02d:%02d:%02d", GPS.year, GPS.month, GPS.day, GPS.hour, GPS.minute, GPS.seconds);
        if(GPS.fix) {
            location_len = sprintf(location, "%6.2f%c | %6.2f%c", GPS.latitude/100, GPS.lat, GPS.longitude/100, GPS.lon);
        } else {
            location_len = sprintf(location, "0 | 0");
        }
        temperature_len = sprintf(temperatures, "%6.2f | %6.2f | %6.2f", readThermocouple(CS0), readThermocouple(CS1), readThermocouple(CS2));

        output_len = sprintf(output, "%s | %s | %s | %s | %s\r\n", ARDUINO_NAME, timestamp, location, states, temperatures);
        debugPrint(output);
        Serial2.print(output); // send to XBee
        if(dataFile) {
            dataFile.println(output);
            dataFile.flush();
        } 
    }
}
