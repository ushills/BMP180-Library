/*
  bmp180.h - Library for reading data from the
  Bosch BMP180 sensor.
  Created by ushills, 14 January 2020.
  Released into the public domain.
*/
#ifndef bmp180_h
#define bmp180_h

#include "Arduino.h"
#include "Wire.h"

// Define chip address
#define BMP180_ADDRESS 0x77

/* Debugging mode
   1 = Serial output
   2 = testing using datasheet values
*/
#define BMP180_DEBUG 0

class bmp180
{
public:
  bmp180();
  bool begin(uint8_t oss = 3);
  int32_t getTemperature(void);
  int32_t getPressure(void);
  uint16_t getUT(void);
  uint32_t getUP(void);
  int32_t getPressureAtSeaLevel(int32_t altitude = 0);
  int32_t getAltitude(void);
  uint32_t ut;
  uint32_t up;

private:
  uint8_t oss;
  uint8_t _BMP180Read1Byte(uint8_t address);
  uint16_t _BMP180Read2bytes(uint8_t address);
  int16_t ac1, ac2, ac3, b1, b2, b5, mb, mc, md;
  uint16_t ac4, ac5, ac6;
};

#endif