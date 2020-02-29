#include "MycParam.h"

#include <stdio.h>

#include "SDL.h"

#include "WindowsWrapper.h"

#include "ArmsItem.h"
#include "CommonDefines.h"
#include "Caret.h"
#include "Draw.h"
#include "File.h"
#include "Game.h"
#include "MyChar.h"
#include "NpChar.h"
#include "Sound.h"
#include "Tags.h"
#include "TextScr.h"
#include "ValueView.h"
#include "KeyControl.h"

int time_count;

void AddRings(int add)
{
	PlaySoundObject(SND_RING_LEFT, 1);
	PlaySoundObject(SND_RING_RIGHT, 1);
	gMC.rings += add;
}

void HurtMyChar(NPCHAR *hit)
{
	if ((g_GameFlags & 2) == 0)
		return;
	
	// Death / lose rings
	if ((gMC.rings == 0 && gMC.shock == 0) || (hit != nullptr && hit->damage < 0))
	{
		//Die
		PlaySoundObject(SND_DEATH, 1);
		gMC.routine = ROUTINE_DEATH;
		LandOnFloor_ExitBall();
		gMC.status.inAir = true;
		gMC.xm = 0;
		gMC.ym = (gMC.status.underwater ? -0x350 : -0x700);
		gMC.gm = 0;
		gMC.anim = ANIM_DEAD;
		gMC.spindashing = false;
		StartTextScript(40);
	}
	else if (gMC.shock == 0)
	{
		// Damage player
		PlaySoundObject(SND_RING_LOSS, 1);
		gMC.routine = ROUTINE_HURT;
		LandOnFloor_ExitBall();
		gMC.shock = 120;
		gMC.gm = 0;
		gMC.spindashing = false;
		gMC.status.inAir = true;
		gMC.anim = ANIM_HURT;
		
		if (hit != nullptr)
		{
			//Rebound from enemy
			gMC.xm = (gMC.status.underwater ? 0x100 : 0x200) * ((gMC.x < hit->x) ? -1 : 1);
		}
		else
		{
			//Go in opposite of facing direction
			if (gMC.direct)
				gMC.xm = -(gMC.status.underwater ? 0x100 : 0x200);
			else
				gMC.xm = (gMC.status.underwater ? 0x100 : 0x200);
		}
		gMC.ym = gMC.status.underwater ? -0x200 : -0x400;
		
		if ((gMC.key & KEY_SHIFT) == 0)
		{
			//Lose rings
			if (gMC.rings > 32)
				gMC.rings = 32;
			SetExpObjects(gMC.x, gMC.y, gMC.rings);
			gMC.rings = 0;
		}
	}
}

void PutHUD(BOOL flash)
{
	//Label blink counter
	static int labelBlink;
	labelBlink++;
	
	//Draw rings label and value
	RECT rcRings = {80, 72, 112, 80};
	if (gMC.rings || labelBlink & 0x8)
		PutBitmap3(&grcGame, PixelToScreenCoord(8), PixelToScreenCoord(24), &rcRings, SURFACE_ID_TEXT_BOX);
	PutNumber4(32, 24, gMC.rings, FALSE);
}

void PutMyAir(int x, int y)
{
	int seconds = gMC.air / 60;
	if (seconds && gMC.status.underwater && (gMC.equip & 0x10) == 0)
	{
		//Draw label
		RECT rcAir[2] = {
			{112, 72, 144, 80},
			{112, 80, 144, 88},
		};
		
		if (gMC.air % 30 > 10)
			PutBitmap3(&grcGame, PixelToScreenCoord(x), PixelToScreenCoord(y), &rcAir[0], SURFACE_ID_TEXT_BOX);
		else
			PutBitmap3(&grcGame, PixelToScreenCoord(x), PixelToScreenCoord(y), &rcAir[1], SURFACE_ID_TEXT_BOX);
		
		//Draw number
		if (seconds & 0x1)
			PutNumber4(x + 32, y, seconds / 2, FALSE);
	}
}

void PutTimeCounter(int x, int y)
{
	RECT rcTime[3] = {
		{112, 104, 120, 112},
		{120, 104, 128, 112},
		{128, 104, 160, 112},
	};

	if (gMC.equip & 0x100)
	{
		// Draw clock and increase time
		if (g_GameFlags & 2)
		{
			if (time_count < 100 * 60 * 60)	// 100 minutes
				++time_count;

			if (time_count % 30 > 10)
				PutBitmap3(&grcGame, PixelToScreenCoord(x), PixelToScreenCoord(y), &rcTime[0], SURFACE_ID_TEXT_BOX);
			else
				PutBitmap3(&grcGame, PixelToScreenCoord(x), PixelToScreenCoord(y), &rcTime[1], SURFACE_ID_TEXT_BOX);
		}
		else
		{
			PutBitmap3(&grcGame, PixelToScreenCoord(x), PixelToScreenCoord(y), &rcTime[0], SURFACE_ID_TEXT_BOX);
		}

		// Draw time
		PutNumber4(x,		y, time_count / (60 * 60),	FALSE);
		PutNumber4(x + 20,	y, time_count / 60 % 60,	TRUE);
		PutNumber4(x + 32,	y, time_count / 6 % 10,		FALSE);
		PutBitmap3(&grcGame, PixelToScreenCoord(x + 30), PixelToScreenCoord(y), &rcTime[2], SURFACE_ID_TEXT_BOX);
	}
	else
	{
		time_count = 0;
	}
}

BOOL SaveTimeCounter()
{
	unsigned char *p;
	int i;

	REC rec;

	// Quit if player doesn't have the Nikumaru Counter
	if (!(gMC.equip & 0x100))
		return TRUE;

	// Get last time
	char path[MAX_PATH];
	sprintf(path, "%s/290.rec", gModulePath);

	FILE *fp = fopen(path, "rb");
	if (fp)
	{
		// Read data
		rec.counter[0] = File_ReadLE32(fp);
		rec.counter[1] = File_ReadLE32(fp);
		rec.counter[2] = File_ReadLE32(fp);
		rec.counter[3] = File_ReadLE32(fp);
		rec.random[0] = fgetc(fp);
		rec.random[1] = fgetc(fp);
		rec.random[2] = fgetc(fp);
		rec.random[3] = fgetc(fp);
		fclose(fp);

		p = (unsigned char*)&rec.counter[0];
		p[0] -= (SDL_BYTEORDER == SDL_LIL_ENDIAN) ? (rec.random[0]) : (rec.random[0] / 2);
		p[1] -= rec.random[0];
		p[2] -= rec.random[0];
		p[3] -= (SDL_BYTEORDER == SDL_LIL_ENDIAN) ? (rec.random[0] / 2) : (rec.random[0]);
		// If this is faster than our new time, quit
		if (rec.counter[0] < time_count)
			return TRUE;
	}

	// Save new time
	for (i = 0; i < 4; i++)
	{
		rec.counter[i] = time_count;
		rec.random[i] = Random(0, 250) + i;

		p = (unsigned char*)&rec.counter[i];
		p[0] += (SDL_BYTEORDER == SDL_LIL_ENDIAN) ? (rec.random[i]) : (rec.random[i] / 2);
		p[1] += rec.random[i];
		p[2] += rec.random[i];
		p[3] += (SDL_BYTEORDER == SDL_LIL_ENDIAN) ? (rec.random[i] / 2) : (rec.random[i]);
	}

	fp = fopen(path, "wb");
	if (fp == NULL)
		return FALSE;

	File_WriteLE32(rec.counter[0], fp);
	File_WriteLE32(rec.counter[1], fp);
	File_WriteLE32(rec.counter[2], fp);
	File_WriteLE32(rec.counter[3], fp);
	fputc(rec.random[0], fp);
	fputc(rec.random[1], fp);
	fputc(rec.random[2], fp);
	fputc(rec.random[3], fp);

	fclose(fp);
	return TRUE;
}

int LoadTimeCounter()
{
	unsigned char *p;
	int i;

	// Open file
	char path[MAX_PATH];
	sprintf(path, "%s/290.rec", gModulePath);

	FILE *fp = fopen(path, "rb");
	if (!fp)
		return 0;

	REC rec;

	// Read data
	rec.counter[0] = File_ReadLE32(fp);
	rec.counter[1] = File_ReadLE32(fp);
	rec.counter[2] = File_ReadLE32(fp);
	rec.counter[3] = File_ReadLE32(fp);
	rec.random[0] = fgetc(fp);
	rec.random[1] = fgetc(fp);
	rec.random[2] = fgetc(fp);
	rec.random[3] = fgetc(fp);
	fclose(fp);

	// Decode from checksum
	for (i = 0; i < 4; i++)
	{
		p = (unsigned char*)&rec.counter[i];
		p[0] -= (SDL_BYTEORDER == SDL_LIL_ENDIAN) ? (rec.random[i]) : (rec.random[i] / 2);
		p[1] -= rec.random[i];
		p[2] -= rec.random[i];
		p[3] -= (SDL_BYTEORDER == SDL_LIL_ENDIAN) ? (rec.random[i] / 2) : (rec.random[i]);
	}

	// Verify checksum's result
	if (rec.counter[0] != rec.counter[1] || rec.counter[0] != rec.counter[2])
	{
		time_count = 0;
		return 0;
	}
	else
	{
		time_count = rec.counter[0];
		return time_count;
	}
}
