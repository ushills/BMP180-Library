#include <Wire.h>
#include <bmp180.h>

// create a instance of bmp180
bmp180 bmp180;

void setup()
{
  Serial.begin(9600);
  if (!bmp180.begin())
  {
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");
    while (1)
    {
    }
  }
}

void loop()
{
  Serial.print("Temperature = ");
  Serial.print(bmp180.getTemperature());
  Serial.println(" C in 1/10deg");

  Serial.print("Pressure = ");
  Serial.print(bmp180.getPressure());
  Serial.println(" Pa");

  // Calculate altitude assuming 'standard' barometric
  // pressure of 1013.25 millibar = 101325 Pascal
  Serial.print("Altitude = ");
  Serial.print(bmp180.getAltitude());
  Serial.println(" meters");

  Serial.print("Pressure at sealevel (calculated) = ");
  Serial.print(bmp180.getPressureAtSeaLevel());
  Serial.println(" Pa");

  Serial.print("Real altitude = ");
  Serial.print(bmp180.getAltitude());
  Serial.println(" meters");

  Serial.println();
  delay(500);
}