#include <DueTimer.h>

int myLed = 13;
int period = 1; // in hertz

bool ledOn = false;
bool ledState = false;

void myHandler(){
  ledOn = !ledOn;

  digitalWrite(myLed, ledOn); // Led on, off, on, off...
}

void setup(){
  pinMode(myLed, OUTPUT);
  Serial.begin(9600);

  Timer3.attachInterrupt(myHandler);
  Timer3.setFrequency(period);
  Timer3.start();
}

void loop(){
  if(ledOn != ledState){
    ledState = ledOn;
    Serial.println(millis());
  //  Serial.println(ledState);
  }
}
