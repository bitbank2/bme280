//
// BME280 sample app
//
#define USE_OLED

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include "bme280.h"
#ifdef USE_OLED
#include "oled96.h"
#endif

// delay between samples in microseconds
#define DELAY 1000000
// Width of the graph (128 - 40)
#define WIDTH 88
void updateOLED(int T, int P, int H)
{
static int iPos = 0; // current sample position
static int iOldTemp[WIDTH], iOldHum[WIDTH]; // keep older samples for graphing
char szTemp[16];
int i, x, y, t, h;

	if (iPos == 0) // first time through
	{
		memset(iOldTemp, 0, sizeof(iOldTemp));
		memset(iOldHum, 0, sizeof(iOldHum));
		oledWriteString(0,0,"Temp",0);
		oledWriteString(0,2,"Pres",0);
		oledWriteString(0,5,"Hum",0);
	}
// Draw the new values and erase the old; shift everything down 1
	sprintf(szTemp, "%2.1fC", (float)T/100.0);
	oledWriteString(0,1,szTemp,0);
	sprintf(szTemp, "%2.2f", (float)P/256.0);
	oledWriteString(0,3,szTemp,0);
	sprintf(szTemp, "%2.1f%%", (float)H/1024.0);
	oledWriteString(0,6,szTemp,0);
// Draw the graph lines
	for (i=iPos-1; i>=0; i--)
	{
		t = iOldTemp[i+1];
		h = iOldHum[i+1];
		x = 127-i; y = (32 * t) / 4000; // erase old pixels
		oledSetPixel(x, 31-y, 0);
		y = (32 * h)/102400;
		oledSetPixel(x, 63-y,0);
		iOldTemp[i+1] = iOldTemp[i];
		iOldHum[i+1] = iOldHum[i];
		t = iOldTemp[i+1];
		h = iOldHum[i+1];
		x = 127-i; y = (32 * t) / 4000; // scale 0-40C to 32 pixels
		oledSetPixel(x, 31-y, 1);
		y = (32 * h)/102400; // scale 0-99% to 32 pixels
		oledSetPixel(x, 63-y, 1);
	}
	iOldTemp[0] = T; iOldHum[0] = H; // store latest values
	iPos++; // move to the right
	if (iPos >= WIDTH)
		iPos = WIDTH-1; // keep it at the last position
} /* updateOLED() */

int main(int argc, char *argv[])
{
int i;
int T, P, H; // calibrated values


	i = bme280Init(0, 0x76);
	if (i != 0)
	{
		return -1; // problem - quit
	}
#ifdef USE_OLED
	i = oledInit(0, 0x3c, 0, 0);
	if (i != 0)
	{
		return -1;
	}
	oledFill(0);
	oledSetContrast(85); // set it to a lower brightness to not burn it
#endif // USE_OLED

	printf("BME280 device successfully opened.\n");
	usleep(1000000); // wait for data to settle for first read

	for (i=0; i<120; i++) // read values twice a second for 1 minute
	{
		bme280ReadValues(&T, &P, &H);
		T -= 150; // for some reason, the sensor reports temperatures too high
#ifdef USE_OLED
		updateOLED(T, P, H); // show temp/humidity graph
#else
		printf("Calibrated temp. = %3.2f C, pres. = %6.2f Pa, hum. = %2.2f%%\n", (float)T/100.0, (float)P/256.0, (float)H/1024.0);
#endif
		usleep(DELAY);
	}

return 0;
} /* main() */
