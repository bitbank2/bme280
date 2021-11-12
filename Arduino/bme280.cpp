#include <Arduino.h>
#include <avr/pgmspace.h>
#include <BitBang_I2C.h>
#include <bme280.h>

//
// BME280 temperature/pressure/humidity sensor
// code to initialize and read+calculate the calibrated values
// from the I2C bus
//
// Written by Larry Bank - 2/1/2017
// Copyright (c) 2017 BitBank Software, Inc.
// bitbank@pobox.com
// updated 2/13/2018 for Arduino
//
// Sensor calibration data
static int iSDAPin, iSCLPin;
static byte bAddr;
static int32_t calT1,calT2,calT3;
static int32_t calP1, calP2, calP3, calP4, calP5, calP6, calP7, calP8, calP9;
static int32_t calH1, calH2, calH3, calH4, calH5, calH6;
static BBI2C bbi2c;

// Wrapper function to write I2C data on Arduino
static void _I2CWrite(byte bAddr, byte *pData, byte bLen)
{
    I2CWrite(&bbi2c, bAddr, pData, bLen);
} /* _I2CWrite() */

static byte _I2CRead(byte bAddr, byte bRegister, byte *pData, byte iLen)
{
byte x;

  x = I2CReadRegister(&bbi2c, bAddr, bRegister, pData, iLen);
  if (x > 0)
     x = iLen; // turn TRUE into number of bytes read
  return x;
  
} /* _I2CRead() */

//
// Opens a file system handle to the I2C device
// reads the calibration data and sets the device
// into auto sensing mode
//
int bme280Init(byte addr, int iSDA, int iSCL)
{
byte i;
byte ucTemp[32];
byte ucCal[36];

   bAddr = addr;
   memset(&bbi2c, 0, sizeof(bbi2c));
   bbi2c.iSDA = iSDA;
   bbi2c.iSCL = iSCL;
   bbi2c.bWire = 1;
   I2CInit(&bbi2c, 100000L); // use BitBang library
   i = _I2CRead(bAddr, 0xd0, ucTemp, 1); // get ID
   if (i != 1)
   {
      Serial.println("bme280 initialization failed");
      return -1;
   }
   else if (ucTemp[0] != 0x60)
   {
      Serial.println("bme280 sensor ID doesn't match");
     return -1;
   }
      Serial.println("bme280 ID matches!");
   // Read 24 bytes of calibration data
   i = _I2CRead(bAddr, 0x88, ucCal, 24); // 24 from register 0x88
   if (i != 24)
   {
//                printf("calibration data not read correctly\n");
      return -1;
   }
   i = _I2CRead(bAddr, 0xa1, &ucCal[24], 1); // get humidity calibration byte
   i = _I2CRead(bAddr, 0xe1, &ucCal[25], 7); // get 7 more humidity calibration bytes
   if (i < 7) // something went wrong
     {}
        // Prepare temperature calibration data
        calT1 = (uint32_t)ucCal[0] + ((uint32_t)ucCal[1] << 8);
        calT2 = (uint32_t)ucCal[2] + ((uint32_t)ucCal[3] << 8);
        if (calT2 > 32767L) calT2 -= 65536L; // negative value
        calT3 = (uint32_t)ucCal[4] + ((uint32_t)ucCal[5] << 8);
        if (calT3 > 32767L) calT3 -= 65536L;

        // Prepare pressure calibration data
        calP1 = (uint32_t)ucCal[6] + ((uint32_t)ucCal[7] << 8);
        calP2 =(uint32_t) ucCal[8] + ((uint32_t)ucCal[9] << 8);
        if (calP2 > 32767L) calP2 -= 65536L; // signed short
        calP3 = (uint32_t)ucCal[10] + ((uint32_t)ucCal[11] << 8);
        if (calP3 > 32767L) calP3 -= 65536L;
        calP4 = (uint32_t)ucCal[12] + ((uint32_t)ucCal[13] << 8);
        if (calP4 > 32767L) calP4 -= 65536L;
        calP5 = (uint32_t)ucCal[14] + ((uint32_t)ucCal[15] << 8);
        if (calP5 > 32767L) calP5 -= 65536L;
        calP6 = (uint32_t)ucCal[16] + ((uint32_t)ucCal[17] << 8);
        if (calP6 > 32767L) calP6 -= 65536L;
        calP7 = (uint32_t)ucCal[18] + ((uint32_t)ucCal[19] << 8);
        if (calP7 > 32767L) calP7 -= 65536L;
        calP8 = (uint32_t)ucCal[20] + ((uint32_t)ucCal[21] << 8);
        if (calP8 > 32767L) calP8 -= 65536L;
        calP9 = (uint32_t)ucCal[22] + ((uint32_t)ucCal[23] << 8);
        if (calP9 > 32767L) calP9 -= 65536L;

        // Prepare humidity calibration data
        calH1 = (uint32_t)ucCal[24];
        calH2 = (uint32_t)ucCal[25] + ((uint32_t)ucCal[26] << 8);
        if (calH2 > 32767L) calH2 -= 65536L;
        calH3 = (uint32_t)ucCal[27];
        calH4 = ((uint32_t)ucCal[28] << 4) + ((uint32_t)ucCal[29] & 0xf);
        if (calH4 > 2047L) calH4 -= 4096L; // signed 12-bit
        calH5 = ((uint32_t)ucCal[30] << 4) + ((uint32_t)ucCal[29] >> 4);
        if (calH5 > 2047L) calH5 -= 4096L;
        calH6 = (uint32_t)ucCal[31];
        if (calH6 > 127L) calH6 -= 256L; // signed char

        ucTemp[0] = 0xf2;
        ucTemp[1] = 0x01; // humidity over sampling rate = 1
        _I2CWrite(bAddr, ucTemp, 2); // control humidity register

        ucTemp[0] = 0xf4;
        ucTemp[1] = 0x27; // normal mode, temp and pressure over sampling rate=1
        _I2CWrite(bAddr, ucTemp, 2); // control measurement register

        ucTemp[0] = 0xf5;
        ucTemp[1] = 0xa0; // set stand by time to 1 second
        _I2CWrite(bAddr, ucTemp, 2); // config
 
        return 0;

} /* bme280Init() */
//
// Read the sensor register values
// and translate them into calibrated readings
// using the previously loaded calibration data
// Temperature is expressed in Celsius degrees as T * 100 (for 2 decimal places)
// Pressure is <future>
// Humidity is express as H * 1024 (10 bit fraction)
//
int bme280ReadValues(uint32_t *T, uint32_t *P, uint32_t *H)
{
byte ucTemp[16];
byte i;
int32_t t, p, h; // raw sensor values
int32_t var1,var2,t_fine;
int64_t P_64;
int64_t var1_64, var2_64;

        i = _I2CRead(bAddr, 0xf7, ucTemp, 8); // start of data regs
        if (i <= 0)
        {
            return -1; // something went wrong
        }
        p = ((uint32_t)ucTemp[0] << 12) + ((uint32_t)ucTemp[1] << 4) + ((uint32_t)ucTemp[2] >> 4);
        t = ((uint32_t)ucTemp[3] << 12) + ((uint32_t)ucTemp[4] << 4) + ((uint32_t)ucTemp[5] >> 4);
        h = ((uint32_t)ucTemp[6] << 8) + (uint32_t)ucTemp[7];
//      printf("raw values: p = %d, t = %d, h = %d\n", p, t, h);
        // Calculate calibrated temperature value
        // the value is 100x C (e.g. 2601 = 26.01C)
        var1 = ((((t >> 3) - (calT1 <<1))) * (calT2)) >> 11;
        var2 = (((((t >> 4) - (calT1)) * ((t>>4) - (calT1))) >> 12) * (calT3)) >> 14;
        t_fine = var1 + var2;
        t_fine = ((t_fine * 5 + 128) >> 8);
        *T = (uint32_t)(t_fine - 150L); // for some reason, the reported temp is too high, subtract 1.5C
        
        // Calculate calibrated pressure value
        var1_64 = t_fine - 128000LL;
        var2_64 = var1_64 * var1_64 * (int64_t)calP6;
        var2_64 = var2_64 + ((var1_64 * (int64_t)calP5) << 17);
        var2_64 = var2_64 + (((int64_t)calP4) << 35);
        var1_64 = ((var1_64 * var1_64 * (int64_t)calP3)>>8) + ((var1_64 * (int64_t)calP2)<<12);
        var1_64 = (((((int64_t)1)<<47)+var1_64))*((int64_t)calP1)>>33;
        if (var1_64 == 0)
        {
                *P = 0;
        }
        else
        {
                P_64 = 1048576LL - p;
                P_64 = (((P_64<<31)-var2_64)*3125LL)/var1_64;
                var1_64 = (((int64_t)calP9) * (P_64>>13) * (P_64>>13)) >> 25;
                var2_64 = (((int64_t)calP8) * P_64) >> 19;
                P_64 = ((P_64 + var1_64 + var2_64) >> 8) + (((int64_t)calP7)<<4);
                *P = (uint32_t)(P_64 / 100LL);
        }
        // Calculate calibrated humidity value
        var1 = (t_fine - 76800L);
        var1 = (((((h << 14) - ((calH4) << 20) - ((calH5) * var1)) +
                (16384L)) >> 15) * (((((((var1 * (calH6)) >> 10) * (((var1 * (calH3)) >> 11) + (32768L))) >> 10) + (2097152L)) * (calH2) + 8192L) >> 14));
        var1 = (var1 - (((((var1 >> 15) * (var1 >> 15)) >> 7) * (calH1)) >> 4));
        var1 = (var1 < 0? 0 : var1);
        var1 = (var1 > 419430400UL ? 419430400UL : var1);
        *H = (uint32_t)(var1 >> 12);
        return 0;

} /* bme280ReadValues() */
