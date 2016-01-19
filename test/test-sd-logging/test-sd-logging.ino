#include <SPI.h>
#include <SD.h>

// Set the pins used
#define SDCS 44
#define SDCLK_AND_LED 50
#define SDMISO 48
#define SDMOSI 46

File logfile;

void setup() {
  Serial.begin(115200);
  Serial.println("\r\nTesting SD card logging");
  pinMode(SDCLK_AND_LED, OUTPUT);
 
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  //pinMode(10, OUTPUT);
  pinMode(SDCS, OUTPUT);

  // see if the card is present and can be initialized:
  if (!SD.begin(SDCS, SDMOSI, SDMISO, SDCLK_AND_LED)) {
    Serial.println("Card init. failed!");
  } else {
    Serial.println("Card successfully initialized.");
  }
}

void loop() {
  // make a string for assembling the data to log:
  String dataString = "";

  // read three sensors and append to the string:
  for (int analogPin = 0; analogPin < 3; analogPin++) {
    int sensor = analogRead(analogPin);
    dataString += String(sensor);
    if (analogPin < 2) {
      dataString += ",";
    }
  }

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    // print to the serial port too:
    Serial.println(dataString);
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  }
  delay(5000);
}
