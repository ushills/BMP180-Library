#include "bmp180.h"

bmp180::bmp180()
{
}

bool bmp180::begin(uint8_t oversampling)
{
  if (oversampling > 3)
    oss = 3;
  oss = oversampling;

  Wire.begin();

  /* Check Chip-id (register D0h): This value is fixed to 0x55 and can be used to check whether
  communication is functioning.*/

  if (_BMP180Read1Byte(0xD0) != 0x55)
    return false;

  // read calibration data
  ac1 = _BMP180Read2bytes(0xAA);
  ac2 = _BMP180Read2bytes(0xAC);
  ac3 = _BMP180Read2bytes(0xAE);
  ac4 = _BMP180Read2bytes(0xB0);
  ac5 = _BMP180Read2bytes(0xB2);
  ac6 = _BMP180Read2bytes(0xB4);
  b1 = _BMP180Read2bytes(0xB6);
  b2 = _BMP180Read2bytes(0xB8);
  mb = _BMP180Read2bytes(0xBA);
  mc = _BMP180Read2bytes(0xBC);
  md = _BMP180Read2bytes(0xBE);

#if (BMP180_DEBUG == 1)
  Serial.print("ac1 = ");
  Serial.println(ac1, DEC);
  Serial.print("ac2 = ");
  Serial.println(ac2, DEC);
  Serial.print("ac3 = ");
  Serial.println(ac3, DEC);
  Serial.print("ac4 = ");
  Serial.println(ac4, DEC);
  Serial.print("ac5 = ");
  Serial.println(ac5, DEC);
  Serial.print("ac6 = ");
  Serial.println(ac6, DEC);

  Serial.print("b1 = ");
  Serial.println(b1, DEC);
  Serial.print("b2 = ");
  Serial.println(b2, DEC);

  Serial.print("mb = ");
  Serial.println(mb, DEC);
  Serial.print("mc = ");
  Serial.println(mc, DEC);
  Serial.print("md = ");
  Serial.println(md, DEC);
#endif

  return true;
}

// Read 1 byte from the BMP180 at 'address'
// Return: the read byte;
uint8_t bmp180::_BMP180Read1Byte(uint8_t address)
{
  //Wire.begin();
  Wire.beginTransmission(BMP180_ADDRESS);
  Wire.write(address);
  Wire.endTransmission();

  Wire.requestFrom(BMP180_ADDRESS, 1);
  while (!Wire.available())
    ;
  return Wire.read();
}

// Read 2 bytes from the BMP180
// First byte will be from 'address'
// Second byte will be from 'address'+1
uint16_t bmp180::_BMP180Read2bytes(uint8_t address)
{
  byte msb, lsb;
  Wire.beginTransmission(BMP180_ADDRESS);
  Wire.write(address);
  Wire.endTransmission();
  Wire.requestFrom(BMP180_ADDRESS, 2);
  while (Wire.available() < 2)
    ;
  msb = Wire.read();
  lsb = Wire.read();
  return msb << 8 | lsb;
}

// Read the uncompensated temperature value
uint16_t bmp180::getUT()
{
  Wire.beginTransmission(BMP180_ADDRESS);
  Wire.write(0xF4);
  Wire.write(0x2E);
  Wire.endTransmission();
  delay(5);
  uint16_t ut = _BMP180Read2bytes(0xF6);
#if (BMP180_DEBUG == 1)
  Serial.Print("UT=");
  Serial.println(ut);
#endif
  return ut;
}

// Read the uncompensated pressure value
uint32_t bmp180::getUP(void)
{
  byte msb, lsb, xlsb;
  Wire.beginTransmission(BMP180_ADDRESS);
  Wire.write(0xF4);
  Wire.write(0x34 + (oss << 6));
  Wire.endTransmission();
  delay(2 + (3 << oss));

  // Read register 0xF6 (MSB), 0xF7 (LSB), and 0xF8 (XLSB)
  msb = _BMP180Read1Byte(0xF6);
  lsb = _BMP180Read1Byte(0xF7);
  xlsb = _BMP180Read1Byte(0xF8);
  up = (((unsigned int32_t)msb << 16) | ((unsigned int32_t)lsb << 8) | (unsigned int32_t)xlsb) >> (8 - oss);
#if (BMP180_DEBUG == 1)
  Serial.Print("UP=");
  Serial.println(up);
#endif
  return up;
}

int32_t bmp180::getTemperature()
{
  ut = getUT();

  int32_t x1, x2;

  x1 = (((int32_t)ut - (int32_t)ac6) * (int32_t)ac5) >> 15;
  x2 = ((int32_t)mc << 11) / (x1 + md);
  b5 = x1 + x2;

  int32_t temp = ((b5 + 8) >> 4);
  temp = temp;
#if BMP180_DEBUG > 1
  // use datasheet numbers!
  UT = 27898;
  ac6 = 23153;
  ac5 = 32757;
  mc = -8711;
  md = 2868;
#endif
#if (BMP180_DEBUG == 1)
  Serial.Print("Temperature=");
  Serial.println(temp);
#endif
  return temp;
}

int32_t bmp180::getPressure()
{
  ut = getUT();
  up = getUP();

  int32_t x1, x2, x3, b3, b6, p;
  uint32_t b4, b7;

#if BMP180_DEBUG > 1
  // use datasheet numbers!
  UT = 27898;
  UP = 23843;
  ac6 = 23153;
  ac5 = 32757;
  mc = -8711;
  md = 2868;
  b1 = 6190;
  b2 = 4;
  ac3 = -14383;
  ac2 = -72;
  ac1 = 408;
  ac4 = 32741;
  oversampling = 0;
#endif

  // calculate pressure compensation b5
  x1 = (((int32_t)ut - (int32_t)ac6) * (int32_t)ac5) >> 15;
  x2 = ((int32_t)mc << 11) / (x1 + md);
  b5 = x1 + x2;
  b6 = b5 - 4000;

#if BMP180_DEBUG == 1
  Serial.print("x1= ");
  Serial.println(x1);
  Serial.print("x2= ");
  Serial.println(x2);
  Serial.print("b5= ");
  Serial.println(b5);
  Serial.print("b6=");
  Serial.println(b6);
#endif

  x1 = (b2 * (b6 * b6) >> 12) >> 11;
  x2 = (ac2 * b6) >> 11;
  x3 = x1 + x2;
  b3 = (((((int32_t)ac1) * 4 + x3) << oss) + 2) >> 2;
#if BMP180_DEBUG == 1
  Serial.print("x1= ");
  Serial.println(x1);
  Serial.print("x2= ");
  Serial.println(x2);
  Serial.print("x3= ");
  Serial.println(x3);
  Serial.print("b3= ");
  Serial.println(b3);
#endif

  x1 = (ac3 * b6) >> 13;
  x2 = (b1 * ((b6 * b6) >> 12)) >> 16;
  x3 = ((x1 + x2) + 2) >> 2;
  b4 = (ac4 * (unsigned int32_t)(x3 + 32768)) >> 15;

  b7 = ((unsigned int32_t)(up - b3) * (50000 >> oss));
  if (b7 < 0x80000000)
    p = (b7 << 1) / b4;
  else
    p = (b7 / b4) << 1;
#if BMP180_DEBUG == 1
  Serial.print("x1= ");
  Serial.println(x1);
  Serial.print("x2= ");
  Serial.println(x2);
  Serial.print("x3= ");
  Serial.println(x3);
  Serial.print("b4= ");
  Serial.println(b4);
  Serial.print("b7= ");
  Serial.println(b7);
  Serial.print("p= ");
  Serial.println(p);
#endif

  x1 = (p >> 8) * (p >> 8);
#if BMP180_DEBUG == 1
  Serial.print("x1= ");
  Serial.println(x1);
#endif
  x1 = (x1 * 3038) >> 16;
  x2 = (-7357 * p) >> 16;
  p += (x1 + x2 + 3791) >> 4;
#if BMP180_DEBUG == 1
  Serial.print("x1= ");
  Serial.println(x1);
  Serial.print("x2= ");
  Serial.println(x2);
  Serial.print("p= ");
  Serial.println(p);
#endif
  return p;
}

int32_t bmp180::getPressureAtSeaLevel(int32_t altitude)
{
  int32_t p = getPressure();
  float pressure = (p / pow(1 - (altitude / 44330.0), 5.255));
  pressure = pressure; // convert to hPa from Pa
  return pressure;
}

int32_t bmp180::getAltitude()
{
  int32_t p = getPressure();
  int32_t altitude = 43330 * (1 - pow((p / 101325), 1 / 5.255));
  return altitude;
}