#pragma once
#include <stdint.h>

int GetCollisionH(int x, int y, bool flipped, uint8_t *angle);
int GetCollisionV(int x, int y, bool flipped, uint8_t *angle);
void HitMyCharNpChar();
void HitMyCharBoss();
