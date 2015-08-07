void setup()
{
  Serial.begin(9600);  // USB port on Arduino, talks to Arduino IDE's serial monitor
  Serial2.begin(9600); // XBee
}

void loop()
{
  if (Serial.available())
  { // If data comes in from serial monitor, send it out to XBee
    Serial2.write(Serial.read());
  }
  if (Serial2.available())
  { // If data comes in from XBee, send it out to serial monitor
    Serial.write(Serial2.read());
  }
}
