#include "ArmsItem.h"

#include <string.h>

#include "WindowsWrapper.h"

#include "CommonDefines.h"
#include "Draw.h"
#include "Escape.h"
#include "Game.h"
#include "KeyControl.h"
#include "Main.h"
#include "Sound.h"
#include "TextScr.h"

int gSelectedItem;
ITEM gItemData[ITEM_MAX];

/// True if we're in the items section of the inventory (not in the weapons section) (only relevant when the inventory is open)
static int gCampTitleY;

void ClearItemData()
{
	memset(gItemData, 0, sizeof(gItemData));
}

BOOL AddItemData(long code)
{
	// Search for code
	int i = 0;
	while (i < ITEM_MAX)
	{
		if (gItemData[i].code == code)
			break;	// Found identical

		if (gItemData[i].code == 0)
			break;	// Found free slot

		++i;
	}

	if (i == ITEM_MAX)
		return FALSE;	// Not found

	gItemData[i].code = code;

	return TRUE;
}

BOOL SubItemData(long code)
{
	// Search for code
	int i;
	for (i = 0; i < ITEM_MAX; ++i)
		if (gItemData[i].code == code)
			break;	// Found

	if (i == ITEM_MAX)
		return FALSE;	// Not found

	// Shift all items from the right to the left
	for (++i; i < ITEM_MAX; ++i)
		gItemData[i - 1] = gItemData[i];

	gItemData[i - 1].code = 0;
	gSelectedItem = 0;

	return TRUE;
}

/// Update the inventory cursor
void MoveCampCursor()
{
	// Compute the current amount of weapons and items
	int item_num = 0;
	while (gItemData[item_num].code != 0)
		++item_num;

	if (item_num == 0)
		return;	// Empty inventory

	/// True if we're currently changing cursor position
	BOOL bChange = FALSE;
	
	// Handle selected item
	if (gKeyTrg & gKeyLeft)
	{
		if (gSelectedItem % 6 == 0)
			gSelectedItem += 5;
		else
			--gSelectedItem;

		bChange = TRUE;
	}

	if (gKeyTrg & gKeyRight)
	{
		if (gSelectedItem == item_num - 1)
			gSelectedItem = 6 * (gSelectedItem / 6);	// Round down to multiple of 6
		else if (gSelectedItem % 6 == 5)
			gSelectedItem -= 5;	// Loop around row
		else
			++gSelectedItem;

		bChange = TRUE;
	}

	if (gKeyTrg & gKeyUp)
	{
		if (!(gSelectedItem / 6 == 0))
			gSelectedItem -= 6;
		bChange = TRUE;
	}

	if (gKeyTrg & gKeyDown)
	{
		if (!(gSelectedItem / 6 == (item_num - 1) / 6))
			gSelectedItem += 6;
		bChange = TRUE;
	}

	if (gSelectedItem >= item_num)
		gSelectedItem = item_num - 1;	// Don't allow selecting a non-existing item

	if (gKeyTrg & gKeyOk)
		StartTextScript(gItemData[gSelectedItem].code + 6000);

	if (bChange)
	{
		// Switch to an item
		PlaySoundObject(SND_YES_NO_CHANGE_CHOICE, 1);

		if (item_num)
			StartTextScript(gItemData[gSelectedItem].code + 5000);
		else
			StartTextScript(5000);
	}
}

/// Draw the inventory
void PutCampObject()
{
	int i;

	/// Rect for the current item
	RECT rcItem;

	/// Final rect drawn on the screen
	RECT rcView = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};

	/// Cursor rect array for items, element [1] being for when the cursor is flashing
	RECT rcCur2[2] = {{80, 88, 112, 104}, {80, 104, 112, 120}};

	RECT rcTitle = {80, 56, 144, 64};
	RECT rcBoxTop = {0, 0, 244, 8};
	RECT rcBoxBody = {0, 8, 244, 16};
	RECT rcBoxBottom = {0, 16, 244, 24};

	// Draw box
	PutBitmap3(&rcView, PixelToScreenCoord((WINDOW_WIDTH - 244) / 2), PixelToScreenCoord((WINDOW_HEIGHT - 224) / 2), &rcBoxTop, SURFACE_ID_TEXT_BOX);
	for (i = 1; i < 18; ++i)
		PutBitmap3(&rcView, PixelToScreenCoord((WINDOW_WIDTH - 244) / 2), PixelToScreenCoord(((WINDOW_HEIGHT - 240) / 2) + (8 * (i + 1))), &rcBoxBody, SURFACE_ID_TEXT_BOX);
	PutBitmap3(&rcView, PixelToScreenCoord((WINDOW_WIDTH - 244) / 2), PixelToScreenCoord(((WINDOW_HEIGHT - 240) / 2) + (8 * (i + 1))), &rcBoxBottom, SURFACE_ID_TEXT_BOX);

	// Move titles
	if (gCampTitleY > (WINDOW_HEIGHT - 208) / 2)
		--gCampTitleY;

	// Draw title
	PutBitmap3(&rcView, PixelToScreenCoord((WINDOW_WIDTH - 224) / 2), PixelToScreenCoord(gCampTitleY), &rcTitle, SURFACE_ID_TEXT_BOX);

	// Update cursor flashing
	static unsigned int flash;
	++flash;
	
	// Draw items cursor
	PutBitmap3(&rcView, PixelToScreenCoord(32 * (gSelectedItem % 6) + (WINDOW_WIDTH - 224) / 2), PixelToScreenCoord(16 * (gSelectedItem / 6) + (WINDOW_HEIGHT - 180) / 2), &rcCur2[flash & 1], SURFACE_ID_TEXT_BOX);

	for (i = 0; i < ITEM_MAX; ++i)
	{
		if (gItemData[i].code == 0)
			break;	// Invalid item

		// Get rect for next item
		rcItem.left = 32 * (gItemData[i].code % 8);
		rcItem.right = rcItem.left + 32;
		rcItem.top = 16 * (gItemData[i].code / 8);
		rcItem.bottom = rcItem.top + 16;

		PutBitmap3(&rcView, PixelToScreenCoord(32 * (i % 6) + (WINDOW_WIDTH - 224) / 2), PixelToScreenCoord(16 * (i / 6) + (WINDOW_HEIGHT - 180) / 2), &rcItem, SURFACE_ID_ITEM_IMAGE);
	}
}

int CampLoop()
{
	int arms_num;
	char old_script_path[MAX_PATH];

	RECT rcView = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};

	// Save the current script path (to restore it when we get out of the inventory)
	GetTextScriptPath(old_script_path);

	// Load the inventory script
	LoadTextScript2("ArmsItem.tsc");

	gCampTitleY = (WINDOW_HEIGHT - 192) / 2;

	// Put the cursor on the first item
	gSelectedItem = 0;
	StartTextScript(gItemData[gSelectedItem].code + 5000);

	for (;;)
	{
		GetTrg();

		// Handle ESC
		if (gKeyTrg & KEY_ESCAPE)
		{
			switch (Call_Escape())
			{
				case 0:
					return 0;	// Quit game
				case 2:
					return 2;	// Go to game intro
			}
		}

		if (g_GameFlags & GAME_FLAG_IS_CONTROL_ENABLED)
			MoveCampCursor();

		switch (TextScriptProc())
		{
			case 0:
				return 0;	// Quit game
			case 2:
				return 2;	// Go to game intro
		}

		// Get currently displayed image
		PutBitmap4(&rcView, 0, 0, &rcView, SURFACE_ID_SCREEN_GRAB);
		PutCampObject();
		PutTextScript();
		PutFramePerSecound();

		// Check whether we're getting out of the loop
		if (g_GameFlags & GAME_FLAG_IS_CONTROL_ENABLED && gKeyTrg & (gKeyCancel | gKeyItem))
		{
			StopTextScript();
			break;
		}

		if (!Flip_SystemTask())
			return 0;	// Quit game
	}

	// Resume original script
	LoadTextScript_Stage(old_script_path);
	return 1;	// Go to game
}

BOOL CheckItem(long a)
{
	for (int i = 0; i < ITEM_MAX; ++i)
		if (gItemData[i].code == a)
			return TRUE;	// Found

	return FALSE;	// Not found
}
