
#ifndef __BME280__
#define __BME280__

int bme280Init(int iChannel, int iAddr);
int bme280ReadValues(int *T, int *P, int *H);

#endif // __BME280__