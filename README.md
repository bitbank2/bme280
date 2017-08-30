BME280 Temperature/Pressure/Humidity sensor test code
Written by Larry Bank
Copyright (c) 2017 BitBank Software, Inc.

bitbank@pobox.com

A simple C library to initialize and read the calibrated values from an
I2C connected BME280. The interface code and calibration formulas are based
on the Bosch BME280 datasheet. A sample program is included to call the
library and display the values to the TTY or an OLED display (my OLED_96 lib).

