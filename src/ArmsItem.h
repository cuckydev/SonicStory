#pragma once

#include "WindowsWrapper.h"

// "Code" means "ID" here
// "Num" often means "ammo" here

/// Item struct
struct ITEM
{
	/// ID of the item
	int code;
};


// Limits for the amount of items
#define ITEM_MAX 0x20

// Currently selected item
extern int gSelectedItem;

/// Contains data for all the items the character currently has
extern ITEM gItemData[ITEM_MAX];

/// Clear the item array, reverting it to the default state (no items) (initialize items basically)
void ClearItemData();


/// Add code to the items. Fails if no space is left
BOOL AddItemData(long code);

/// Remove code from the items. Fails if code is not found
BOOL SubItemData(long code);


/// Inventory loop. Returns mode.
int CampLoop();


/// Search for a in the items. Returns whether a was found
BOOL CheckItem(long a);