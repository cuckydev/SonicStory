#include "Triangle.h"

#include <math.h>

int gSin[0x100];
const unsigned char atanTable[] =
{
	0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x04,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x05, 0x05, 0x05,
	0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x09,
	0x09, 0x09, 0x09, 0x09, 0x09, 0x0A, 0x0A, 0x0A,
	0x0A, 0x0A, 0x0A, 0x0A, 0x0B, 0x0B, 0x0B, 0x0B,
	0x0B, 0x0B, 0x0B, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C,
	0x0C, 0x0C, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D,
	0x0D, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E,
	0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x10,
	0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x11, 0x11,
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x12, 0x12,
	0x12, 0x12, 0x12, 0x12, 0x12, 0x13, 0x13, 0x13,
	0x13, 0x13, 0x13, 0x13, 0x13, 0x14, 0x14, 0x14,
	0x14, 0x14, 0x14, 0x14, 0x14, 0x15, 0x15, 0x15,
	0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x16, 0x16,
	0x16, 0x16, 0x16, 0x16, 0x16, 0x16, 0x17, 0x17,
	0x17, 0x17, 0x17, 0x17, 0x17, 0x17, 0x17, 0x18,
	0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
	0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19,
	0x19, 0x19, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A,
	0x1A, 0x1A, 0x1A, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B,
	0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1C, 0x1C, 0x1C,
	0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C,
	0x1D, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D,
	0x1D, 0x1D, 0x1D, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E,
	0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1F, 0x1F,
	0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F,
	0x1F, 0x1F, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
	0x20, 0x00
};

void InitTriangleTable()
{
	int i;
	float a;
	float b;

	//Sine
	for (i = 0; i < 0x100; ++i)
	{
		gSin[i] = (int)(sin(i * 6.2831998 / 256.0) * 512.0);
	}
}

int GetSin(unsigned char deg)
{
	return gSin[deg];
}

int GetCos(unsigned char deg)
{
	deg += 0x40;
	return gSin[deg];
}

unsigned char GetArktan(int x, int y)
{
	//If x and y is 0, return 90 degrees, then get our absolute x and y
	if (x == 0 && y == 0)
		return 0x40;
	
	unsigned int absX = abs(x);
	unsigned int absY = abs(y);
	
	//Get our absolute angle
	unsigned char angle;
	if (absY < absX)
		angle = atanTable[(absY <<= 8) /= absX];
	else
		angle = 0x40 - atanTable[(absX <<= 8) /= absY];
	
	//Invertion if negative
	if (x >= 0)
		angle = -angle + 0x80;
	if (y >= 0)
		angle = -angle + 0x100;
	return angle;
}
