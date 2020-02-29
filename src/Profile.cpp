#include "Profile.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "WindowsWrapper.h"

#include "ArmsItem.h"
#include "BossLife.h"
#include "Fade.h"
#include "File.h"
#include "Flags.h"
#include "Frame.h"
#include "Game.h"
#include "MiniMap.h"
#include "MyChar.h"
#include "NpChar.h"
#include "SelStage.h"
#include "Stage.h"
#include "Tags.h"
#include "ValueView.h"

const char *gDefaultName = "Profile.dat";
const char *gProfileCode = "Do041221";

BOOL IsProfile()
{
	char path[MAX_PATH];
	sprintf(path, "%s/%s", gModulePath, gDefaultName);

	FILE *fp = fopen(path, "rb");
	if (fp == NULL)
		return FALSE;

	fclose(fp);
	return TRUE;
}

BOOL SaveProfile(const char *name)
{
	PROFILE profile;
	FILE *fp;
	const char *FLAG = "FLAG";
	char path[MAX_PATH];

	// Get path
	if (name)
		sprintf(path, "%s/%s", gModulePath, name);
	else
		sprintf(path, "%s/%s", gModulePath, gDefaultName);

	// Open file
	fp = fopen(path, "wb");
	if (fp == NULL)
		return FALSE;

	// Set up profile
	memset(&profile, 0, sizeof(PROFILE));
	memcpy(profile.code, gProfileCode, sizeof(profile.code));
	memcpy(profile.FLAG, FLAG, sizeof(profile.FLAG));
	profile.stage = gStageNo;
	profile.music = gMusicNo;
	profile.x = gMC.x;
	profile.y = gMC.y;
	profile.direct = gMC.direct;
	profile.rings = gMC.rings;
	profile.select_item = gSelectedItem;
	profile.equip = gMC.equip;
	profile.unit = gMC.unit;
	profile.counter = gCounter;
	memcpy(profile.items, gItemData, sizeof(profile.items));
	memcpy(profile.permitstage, gPermitStage, sizeof(profile.permitstage));
	memcpy(profile.permit_mapping, gMapping, sizeof(profile.permit_mapping));
	memcpy(profile.flags, gFlagNPC, sizeof(profile.flags));

	// Write to file
	fwrite(profile.code, 8, 1, fp);
	File_WriteLE32(profile.stage, fp);
	File_WriteLE32(profile.music, fp);
	File_WriteLE32(profile.x, fp);
	File_WriteLE32(profile.y, fp);
	File_WriteLE32(profile.direct, fp);
	File_WriteLE32(profile.rings, fp);
	File_WriteLE32(profile.select_item, fp);
	File_WriteLE32(profile.equip, fp);
	File_WriteLE32(profile.unit, fp);
	File_WriteLE32(profile.counter, fp);
	for (int item = 0; item < 32; item++)
		File_WriteLE32(profile.items[item].code, fp);
	for (int stage = 0; stage < 8; stage++)
	{
		File_WriteLE32(profile.permitstage[stage].index, fp);
		File_WriteLE32(profile.permitstage[stage].event, fp);
	}
	fwrite(profile.permit_mapping, 0x80, 1, fp);
	fwrite(FLAG, 4, 1, fp);
	fwrite(profile.flags, 1000, 1, fp);

	fclose(fp);
	return TRUE;
}

BOOL LoadProfile(const char *name)
{
	// Get path
	char path[MAX_PATH];

	if (name)
		sprintf(path, "%s", name);
	else
		sprintf(path, "%s/%s", gModulePath, gDefaultName);

	// Open file
	PROFILE profile;

	FILE *fp = fopen(path, "rb");
	if (fp == NULL)
		return FALSE;

	// Check header code
	fread(profile.code, 8, 1, fp);
	if (memcmp(profile.code, gProfileCode, 8))
	{
#ifdef FIX_BUGS
		fclose(fp);	// The original game forgets to close the file
#endif
		return FALSE;
	}

	// Read data
	fseek(fp, 0, SEEK_SET);
	memset(&profile, 0, sizeof(PROFILE));
	fread(profile.code, 8, 1, fp);
	profile.stage = File_ReadLE32(fp);
	profile.music = (MusicID)File_ReadLE32(fp);
	profile.x = File_ReadLE32(fp);
	profile.y = File_ReadLE32(fp);
	profile.direct = File_ReadLE32(fp);
	profile.rings = File_ReadLE32(fp);
	profile.select_item = File_ReadLE32(fp);
	profile.equip = File_ReadLE32(fp);
	profile.unit = File_ReadLE32(fp);
	profile.counter = File_ReadLE32(fp);
	for (int item = 0; item < 32; item++)
		profile.items[item].code = File_ReadLE32(fp);
	for (int stage = 0; stage < 8; stage++)
	{
		profile.permitstage[stage].index = File_ReadLE32(fp);
		profile.permitstage[stage].event = File_ReadLE32(fp);
	}
	fread(profile.permit_mapping, 0x80, 1, fp);
	fread(profile.FLAG, 4, 1, fp);
	fread(profile.flags, 1000, 1, fp);
	fclose(fp);

	// Set things
	gSelectedItem = profile.select_item;
	gCounter = profile.counter;

	memcpy(gItemData, profile.items, sizeof(gItemData));
	memcpy(gPermitStage, profile.permitstage, sizeof(gPermitStage));
	memcpy(gMapping, profile.permit_mapping, sizeof(gMapping));
	memcpy(gFlagNPC, profile.flags, sizeof(gFlagNPC));

	// Load stage
	ChangeMusic(profile.music);
	InitMyChar();
	if (!TransferStage(profile.stage, 0, 0, 1))
		return FALSE;

	// Set character properties
	gMC.equip = profile.equip;
	gMC.unit = profile.unit;
	gMC.direct = profile.direct;
	gMC.rings = profile.rings;
	gMC.cond = 0x80;
	gMC.air = 30*60;
	gMC.x = profile.x;
	gMC.y = profile.y;

	// Reset stuff
	ClearFade();
	SetFrameMyChar();
	SetFrameTargetMyChar(16);
	InitBossLife();
	CutNoise();
	ClearValueView();
	gCurlyShoot_wait = 0;
	return TRUE;
}

BOOL InitializeGame(void)
{
	InitMyChar();
	gSelectedItem = 0;
	gCounter = 0;
	ClearItemData();
	ClearPermitStage();
	StartMapping();
	InitFlags();
	if (!TransferStage(13, 200, 10, 8))
	{
#ifdef JAPANESE
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "エラー", "ステージの読み込みに失敗", NULL);
#else
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Failed to load stage", NULL);
#endif

		return FALSE;
	}

	ClearFade();
	SetFrameMyChar();
	SetFrameTargetMyChar(16);
	InitBossLife();
	CutNoise();
	ClearValueView();
	gCurlyShoot_wait = 0;
	SetFadeMask();
	SetFrameTargetMyChar(16);
	return TRUE;
}
