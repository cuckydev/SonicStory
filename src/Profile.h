#pragma once

#include "WindowsWrapper.h"

#include "ArmsItem.h"
#include "SelStage.h"
#include "Stage.h"

struct PROFILE
{
	char code[8];
	int stage;
	MusicID music;
	int x;
	int y;
	int direct;
	unsigned int rings;
	int select_item;
	int equip;
	int unit;
	int counter;
	ITEM items[32];
	PERMIT_STAGE permitstage[8];
	signed char permit_mapping[0x80];
	char FLAG[4];
	unsigned char flags[1000];
};

BOOL IsProfile(void);
BOOL SaveProfile(const char *name);
BOOL LoadProfile(const char *name);
BOOL InitializeGame(void);
