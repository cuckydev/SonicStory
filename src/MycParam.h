#pragma once
#include "NpChar.h"
#include "WindowsWrapper.h"

struct REC
{
	long counter[4];
	unsigned char random[4];
};

void AddRings(int add);
void HurtMyChar(NPCHAR *hit);
void PutHUD(BOOL flash);
void PutMyAir(int x, int y);
void PutTimeCounter(int x, int y);
BOOL SaveTimeCounter();
int LoadTimeCounter();
