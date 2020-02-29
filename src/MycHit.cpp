#include "MycHit.h"

#include "WindowsWrapper.h"

#include "Back.h"
#include "Boss.h"
#include "Caret.h"
#include "Game.h"
#include "KeyControl.h"
#include "Map.h"
#include "MyChar.h"
#include "MycParam.h"
#include "NpChar.h"
#include "NpcHit.h"
#include "Sound.h"
#include "TextScr.h"

//Stage collision
static const int heightBlock[16 * 2] = { 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
										 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10};

static const int heightTrigA[16 * 2] = {-0x10,-0x10,-0x0F,-0x0F,-0x0E,-0x0E,-0x0D,-0x0D,-0x0C,-0x0C,-0x0B,-0x0B,-0x0A,-0x0A,-0x09,-0x09,
										 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static const int heightTrigB[16 * 2] = {-0x08,-0x08,-0x07,-0x07,-0x06,-0x06,-0x05,-0x05,-0x04,-0x04,-0x03,-0x03,-0x02,-0x02,-0x01,-0x01,
										-0x10,-0x0E,-0x0C,-0x0A,-0x08,-0x06,-0x04,-0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static const int heightTrigC[16 * 2] = {-0x01,-0x01,-0x02,-0x02,-0x03,-0x03,-0x04,-0x04,-0x05,-0x05,-0x06,-0x06,-0x07,-0x07,-0x08,-0x08,
										 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static const int heightTrigD[16 * 2] = {-0x09,-0x09,-0x0A,-0x0A,-0x0B,-0x0B,-0x0C,-0x0C,-0x0D,-0x0D,-0x0E,-0x0E,-0x0F,-0x0F,-0x10,-0x10,
										 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static const int heightTrigE[16 * 2] = { 0x10, 0x10, 0x0F, 0x0F, 0x0E, 0x0E, 0x0D, 0x0D, 0x0C, 0x0C, 0x0B, 0x0B, 0x0A, 0x0A, 0x09, 0x09,
										 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static const int heightTrigF[16 * 2] = { 0x08, 0x08, 0x07, 0x07, 0x06, 0x06, 0x05, 0x05, 0x04, 0x04, 0x03, 0x03, 0x02, 0x02, 0x01, 0x01,
										 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static const int heightTrigG[16 * 2] = { 0x01, 0x01, 0x02, 0x02, 0x03, 0x03, 0x04, 0x04, 0x05, 0x05, 0x06, 0x06, 0x07, 0x07, 0x08, 0x08,
										 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static const int heightTrigH[16 * 2] = { 0x09, 0x09, 0x0A, 0x0A, 0x0B, 0x0B, 0x0C, 0x0C, 0x0D, 0x0D, 0x0E, 0x0E, 0x0F, 0x0F, 0x10, 0x10,
										 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

struct COLTILE
{
	const int *height;
	uint8_t angle;
} tilemap[0x100] = {
	{nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {heightBlock,	0xFF}, {nullptr,		0x00}, {nullptr,		0x00},
	{nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00},
	{nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00},
	{nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00},
	{nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00},
	{nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00},
	{nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00},
	{nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00},
	{nullptr,		0x00}, {heightBlock,	0xFF}, {nullptr,		0x00}, {heightBlock,	0xFF}, {nullptr,		0x00}, {nullptr,		0x00}, {heightBlock,	0xFF}, {nullptr,		0x00},
	{nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00},
	{heightTrigA,	0x70}, {heightTrigB,	0x70}, {heightTrigC,	0x90}, {heightTrigD,	0x90}, {heightTrigE,	0x10}, {heightTrigF,	0x10}, {heightTrigG,	0xF0}, {heightTrigH,	0xF0},
	{nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00},
	{nullptr,		0x00}, {heightBlock,	0xFF}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00},
	{nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00},
	{heightTrigA,	0x70}, {heightTrigB,	0x70}, {heightTrigC,	0x90}, {heightTrigD,	0x90}, {heightTrigE,	0x10}, {heightTrigF,	0x10}, {heightTrigG,	0xF0}, {heightTrigH,	0xF0},
	{nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00},
	{nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00},
	{nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00},
	{nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00},
	{nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00},
	{nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00},
	{nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00},
	{nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00},
	{nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00},
	{nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00},
	{nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00},
	{nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00},
	{nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00},
	{nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00},
	{nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00},
	{nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00},
	{nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00}, {nullptr,		0x00},
};

//Horizontal stage collision
int GetCollisionH_Tile2(int x, int y, bool flipped, uint8_t *angle)
{
	COLTILE tile = tilemap[GetAttribute(x / 0x2000, y / 0x2000)];
	if (tile.height != nullptr)
	{
		//Return tile angle
		if (angle != nullptr)
			*angle = tile.angle;
		
		//Get our height in the heightmap
		int height = tile.height[0x10 + ((y / 0x200) & 0xF)] * 0x200;
		if (flipped)
			height = -height;
		
		//Either return this surface or check the tile above
		if (height > 0)
		{
			return 0x2000 - (height + (x & 0x1FFF));
		}
		else if (height < 0)
		{
			int distance = x & 0x1FFF;
			if (height + distance < 0)
				return ~distance;
		}
	}
	
	return 0x2000 - (x & 0x1FFF);
}

int GetCollisionH(int x, int y, bool flipped, uint8_t *angle)
{
	//Get our tile
	COLTILE tile = tilemap[GetAttribute((x += 0x1000) / 0x2000, (y += 0x1000) / 0x2000)];
	if (tile.height != nullptr)
	{
		//Flip x-position if flipped
		if (flipped)
			x ^= 0x1FFF;
		
		//Return tile angle
		if (angle != nullptr)
			*angle = tile.angle;
		
		//Get our height in the heightmap
		int height = tile.height[0x10 + ((y / 0x200) & 0xF)] * 0x200;
		if (flipped)
			height = -height;
		
		//Either return this surface or check the tile above
		if (height > 0)
		{
			if (height < 0x2000)
				return 0x2000 - (height + (x & 0x1FFF));
			else
				return GetCollisionH_Tile2(x - (flipped ? -0x2000 : 0x2000), y, flipped, angle) - 0x2000;
		}
		else if (height < 0)
		{
			if (height + (x & 0x1FFF) < 0)
				return GetCollisionH_Tile2(x - (flipped ? -0x2000 : 0x2000), y, flipped, angle) - 0x2000;
		}
	}
	
	return GetCollisionH_Tile2(x + (flipped ? -0x2000 : 0x2000), y, flipped, angle) + 0x2000;
}

//Vertical stage collision
int GetCollisionV_Tile2(int x, int y, bool flipped, uint8_t *angle)
{
	COLTILE tile = tilemap[GetAttribute(x / 0x2000, y / 0x2000)];
	if (tile.height != nullptr)
	{
		//Return tile angle
		if (angle != nullptr)
			*angle = tile.angle;
		
		//Get our height in the heightmap
		int height = tile.height[(x / 0x200) & 0xF] * 0x200;
		if (flipped)
			height = -height;
		
		//Either return this surface or check the tile above
		if (height > 0)
		{
			return 0x2000 - (height + (y & 0x1FFF));
		}
		else if (height < 0)
		{
			int distance = y & 0x1FFF;
			if (height + distance < 0)
				return ~distance;
		}
	}
	
	return 0x2000 - (y & 0x1FFF);
}

int GetCollisionV(int x, int y, bool flipped, uint8_t *angle)
{
	COLTILE tile = tilemap[GetAttribute((x += 0x1000) / 0x2000, (y += 0x1000) / 0x2000)];
	if (tile.height != nullptr)
	{
		//Flip y-position if flipped
		if (flipped)
			y ^= 0x1FFF;
		
		//Return tile angle
		if (angle != nullptr)
			*angle = tile.angle;
		
		//Get our height in the heightmap
		int height = tile.height[(x / 0x200) & 0xF] * 0x200;
		if (flipped)
			height = -height;
		
		//Either return this surface or check the tile above
		if (height > 0)
		{
			if (height < 0x2000)
				return 0x2000 - (height + (y & 0x1FFF));
			else
				return GetCollisionV_Tile2(x, y - (flipped ? -0x2000 : 0x2000), flipped, angle) - 0x2000;
		}
		else if (height < 0)
		{
			if (height + (y & 0x1FFF) < 0)
				return GetCollisionV_Tile2(x, y - (flipped ? -0x2000 : 0x2000), flipped, angle) - 0x2000;
		}
	}
	
	return GetCollisionV_Tile2(x, y + (flipped ? -0x2000 : 0x2000), flipped, angle) + 0x2000;
}

//NPC collision
#define DB 5
#define DI 7

void HitMyCharNpChar_ClearStanding(NPCHAR *npc)
{
	if (gMC.interact == npc && npc != nullptr)
	{
		npc->mychar_standing = FALSE;
		gMC.status.shouldNotFall = false;
		gMC.interact = nullptr;
	}
}

void HitMyCharNpChar_AttachPlayer(NPCHAR *npc)
{
	if (gMC.interact != nullptr && gMC.interact != npc)
		HitMyCharNpChar_ClearStanding(gMC.interact);
	gMC.interact = npc;
	gMC.angle = 0;
	gMC.ym = 0;
	gMC.gm = gMC.xm;
	gMC.status.shouldNotFall = true;
	npc->mychar_standing = TRUE;
	if (gMC.status.inAir == true)
	{
		gMC.status.inAir = false;
		LandOnFloor_ExitBall();
	}
}

int HitMyCharNpChar_NonSolid(NPCHAR *npc)
{
	//Use player's hitbox
	int playerWidth = 0x400; //radius
	int playerLeft = gMC.x - playerWidth;
	playerWidth *= 2; //diameter
	
	int playerHeight = 0x700; //radius
	int playerTop = gMC.y - playerHeight + 0x500;
	playerHeight *= 2; //diameter
	
	//Give us a smaller hitbox while ducking
	if (gMC.anim == ANIM_DUCK)
	{
		playerTop += 0x600;
	}
	
	//Get NPC's hitbox
	int npcLeft = npc->x - (npc->direct ? npc->hit.back : npc->hit.front);
	int npcWidth = npc->hit.back + npc->hit.front;
	int npcTop = npc->y - npc->hit.top;
	int npcHeight = npc->hit.top + npc->hit.bottom;
	
	//Check for contact
	int horizontalCheck = playerLeft - npcLeft;
	int verticalCheck = playerTop - npcTop;
	if (horizontalCheck >= -playerWidth && horizontalCheck <= npcWidth && verticalCheck >= -playerHeight && verticalCheck <= npcHeight)
		return 1;
	
	return 0;
}

int HitMyCharNpChar_SoftSolid(NPCHAR *npc)
{
	int hit = 0;
	
	//Get object sides depending on direction and our orientation
	int npcLeft = (npc->direct ? npc->hit.back : npc->hit.front);
	int npcRight = (npc->direct ? npc->hit.front : npc->hit.back);
	int npcWidth = npcLeft + npcRight;
	
	int npcExtLeft = npcLeft + 0xA00;
	int npcExtRight = npcRight + 0xA00;
	int npcExtWidth = npcExtLeft + npcExtRight;
	
	//Standing on object
	int lastXPos = npc->x - npc->xm;
	
	if (npc->mychar_standing == TRUE)
	{
		//Check if we're to exit the top of the object
		int xDiff = (gMC.x - lastXPos) + npcExtLeft;
		if (gMC.status.inAir || xDiff < 0 || xDiff >= npcExtWidth)
		{
			//Exit top as platform
			HitMyCharNpChar_ClearStanding(npc);
			return hit;
		}
		else
		{
			//Move with object
			gMC.x += npc->xm;
			gMC.y = npc->y - npc->hit.top - gMC.yRadius;
			
			//Bounce
			if (npc->bits & NPC_BOUNCY)
			{
				HitMyCharNpChar_ClearStanding(npc);
				gMC.ym = npc->ym - 0x200;
			}
			
			hit |= 8;
			return hit;
		}
	}
	else
	{
		//Check if we're within horizontal range
		int xDiff = (gMC.x - npc->x) + npcExtLeft; //d0
		if (xDiff >= 0 && xDiff <= npcExtWidth)
		{
			//Check if we're within vertical range
			int yDiff; //d3
			int heightHalf = npc->hit.top + gMC.yRadius;
			yDiff = ((gMC.y - npc->y) + 0x400) + heightHalf;
			
			//Get our complete height
			int height = heightHalf + npc->hit.bottom + gMC.yRadius;
			
			if (yDiff >= 0 && yDiff <= height)
			{
				//Get our clip differences
				int xClip = xDiff;
				if (xDiff >= npcExtLeft)
				{
					xDiff -= npcExtWidth;
					xClip = -xDiff;
				}
				
				int yClip = yDiff;
				if (yDiff >= heightHalf)
				{
					yDiff -= (0x400 + height);
					yClip = -yDiff;
				}
				
				//Check if we're above / below or to the sides of the object
				if (xClip >= yClip || yClip <= 0x800)
				{
					if (yDiff < 0) //If colliding from below
					{
						if (gMC.status.inAir == false && gMC.ym == 0)
						{
							hit |= 2;
							//Fallthrough into horizontal check
						}
						else
						{
							if (gMC.ym < 0 && yDiff < 0) //Why is yDiff checked to be negative?
							{
								//Clear ground move if in mid-air
								if (gMC.status.inAir)
									gMC.gm = 0;
								gMC.ym = 0;
							}
							
							//Set bottom touch flag
							hit |= 2;
							return hit;
						}
					}
					else
					{
						//Check our vertical difference
						if (yDiff < 0x1000)
						{
							//Subtract 0x400 from yDiff to undo the offset done before
							yDiff -= 0x400;
							
							//Check our horizontal range
							int16_t xDiff2 = (gMC.x - lastXPos) + npcLeft;
							
							if (xDiff2 >= 0 && xDiff2 < npcWidth && gMC.ym >= 0)
							{
								//Land on the object
								gMC.y -= (yDiff + 0x100);
								if (gMC.anim != ANIM_ROLL || (npc->bits & NPC_INTERACTABLE) != 0 || (npc->damage == 0 && (npc->bits & NPC_REAR_AND_TOP_DONT_HURT) == 0))
								{
									if (npc->bits & NPC_BOUNCY)
									{
										//Bounce
										gMC.ym = npc->ym - 0x200;
									}
									else
									{
										//Stand
										HitMyCharNpChar_AttachPlayer(npc);
									}
								}
								
								//Set top touch flag
								hit |= 8;
							}
							
							return hit;
						}
						
						return hit;
					}
				}
				
				//Bounce from sides
				if (xDiff > 0)
				{
					if (gMC.xm > -0x200)
						gMC.xm -= 0x200;
					gMC.gm = gMC.xm;
					hit |= 4;
				}
				else if (xDiff < 0)
				{
					if (gMC.xm < 0x200)
						gMC.xm += 0x200;
					gMC.gm = gMC.xm;
					hit |= 1;
				}
				
				return hit;
			}
		}
	}
	
	return hit;
}

int HitMyCharNpChar_HardSolid(NPCHAR *npc)
{
	int hit = 0;
	
	//Get object sides depending on direction and our orientation
	int npcLeft = (npc->direct ? npc->hit.back : npc->hit.front);
	int npcRight = (npc->direct ? npc->hit.front : npc->hit.back);
	int npcWidth = npcLeft + npcRight;
	
	int npcExtLeft = npcLeft + 0xA00;
	int npcExtRight = npcRight + 0xA00;
	int npcExtWidth = npcExtLeft + npcExtRight;
	
	//Standing on object
	int lastXPos = npc->x - npc->xm;
	
	if (npc->mychar_standing == TRUE)
	{
		//Check if we're to exit the top of the object
		int xDiff = (gMC.x - lastXPos) + npcExtLeft;
		if (gMC.status.inAir || xDiff < 0 || xDiff >= npcExtWidth)
		{
			//Exit top as platform
			HitMyCharNpChar_ClearStanding(npc);
			return hit;
		}
		else
		{
			//Move with object
			gMC.x += npc->xm;
			gMC.y = npc->y - npc->hit.top - gMC.yRadius;
			
			//Bounce
			if (npc->bits & NPC_BOUNCY)
			{
				HitMyCharNpChar_ClearStanding(npc);
				gMC.ym = npc->ym - 0x200;
			}
			
			hit |= 8;
			return hit;
		}
	}
	else
	{
		//Check if we're within horizontal range
		int xDiff = (gMC.x - npc->x) + npcExtLeft; //d0
		if (xDiff >= 0 && xDiff <= npcExtWidth)
		{
			//Check if we're within vertical range
			int yDiff; //d3
			int heightHalf = npc->hit.top + gMC.yRadius;
			yDiff = ((gMC.y - npc->y) + 0x400) + heightHalf;
			
			//Get our complete height
			int height = heightHalf + npc->hit.bottom + gMC.yRadius;
			
			if (yDiff >= 0 && yDiff <= height)
			{
				//Get our clip differences
				int xClip = xDiff;
				if (xDiff >= npcExtLeft)
				{
					xDiff -= npcExtWidth;
					xClip = -xDiff;
				}
				
				int yClip = yDiff;
				if (yDiff >= heightHalf)
				{
					yDiff -= (0x400 + height);
					yClip = -yDiff;
				}
				
				//Check if we're above / below or to the sides of the object
				if (xClip >= yClip || yClip <= 0x800)
				{
					if (yDiff < 0) //If colliding from below
					{
						if (gMC.ym < 0 && yDiff < 0) //Why is yDiff checked to be negative?
						{
							//Clear ground move if in mid-air
							if (gMC.status.inAir)
								gMC.gm = 0;
						
							//Clip us out of the bottom
							gMC.y -= yDiff;
							gMC.ym = 0;
						}
						
						//Set bottom touch flag
						hit |= 2;
						return hit;
					}
					else
					{
						//Check our vertical difference
						if (yDiff < 0x1000)
						{
							//Subtract 0x400 from yDiff to undo the offset done before
							yDiff -= 0x400;
							
							//Check our horizontal range
							int16_t xDiff2 = (gMC.x - lastXPos) + npcLeft;
							
							if (xDiff2 >= 0 && xDiff2 < npcWidth && gMC.ym >= 0)
							{
								//Land on the object
								gMC.y -= (yDiff + 0x100);
								if (npc->bits & NPC_BOUNCY)
								{
									//Bounce
									LandOnFloor_ExitBall();
									gMC.ym = npc->ym - 0x200;
								}
								else
								{
									//Stand
									HitMyCharNpChar_AttachPlayer(npc);
								}
								
								//Set top touch flag
								hit |= 8;
							}
							
							return hit;
						}
						
						return hit;
					}
				}
				
				//Hault our velocity if running into sides
				if (xDiff > 0)
				{
					if (gMC.xm >= 0)
					{
						gMC.gm = 0;
						gMC.xm = 0;
					}
					hit |= 4;
				}
				else if (xDiff < 0)
				{
					if (gMC.xm < 0)
					{
						gMC.gm = 0;
						gMC.xm = 0;
					}
					hit |= 1;
				}
				
				//Clip out of side
				gMC.x -= xDiff;
				return hit;
			}
		}
	}
	
	return hit;
}

void HitMyCharNpChar()
{
	//Don't run if disabled
	if ((gMC.cond & 0x80) == 0 || (gMC.cond & 2))
		return;

	//Check for NPC collisions
	static int ringOffset = 0;
	
	for (int i = 0; i < NPC_MAX; i++)
	{
		if ((gNPC[i].cond & 0x80) == 0)
		{
			HitMyCharNpChar_ClearStanding(&gNPC[i]);
			continue;
		}
		
		//Check if we're touching
		int hit = 0;
		if (gNPC[i].bits & NPC_SOLID_SOFT)
		{
			//Check for soft solid contact
			hit = HitMyCharNpChar_SoftSolid(&gNPC[i]);
		}
		else if (gNPC[i].bits & NPC_SOLID_HARD)
		{
			//Check for hard solid contact
			hit = HitMyCharNpChar_HardSolid(&gNPC[i]);
		}
		else
		{
			//Release if was standing on
			if (gMC.interact == &gNPC[i])
				HitMyCharNpChar_ClearStanding(&gNPC[i]);
			
			//Check for non-solid contact
			hit = HitMyCharNpChar_NonSolid(&gNPC[i]);
		}
		
		//Pickup if ring
		if (hit && gNPC[i].code_char == 1 && gNPC[i].count1 >= 30 && gMC.shock < 90)
		{
			PlaySoundObject((int)SND_RING_LEFT + (ringOffset ^= 1), 1);
			gMC.rings++;
			gNPC[i].cond = 0;
		}
		
		//Run event on contact
		if ((g_GameFlags & 4) == 0 && hit && (gNPC[i].bits & NPC_EVENT_WHEN_TOUCHED))
			StartTextScript(gNPC[i].code_event);
		
		//NPC damage and killing
		if ((g_GameFlags & 2) && (g_GameFlags & 4) == 0 && (gNPC[i].bits & NPC_INTERACTABLE) == 0 && hit)
		{
			//Damage if in ball form
			if ((gNPC[i].bits & NPC_SHOOTABLE) && (gMC.anim == ANIM_ROLL || gMC.anim == ANIM_SPINDASH))
			{
				if (gNPC[i].shock == 0)
				{
					//Do damage
					DamageNpChar(&gNPC[i], gMC.x, gMC.y, DB + gMC.rings / DI);
					
					if (gNPC[i].cond & 8)
					{
						//Killed, bounce like a normal enemy
						if (gMC.ym >= 0)
						{
							if (gMC.y < gNPC[i].y)
								gMC.ym = -gMC.ym; //If above us, reverse player velocity
							else
								gMC.ym -= 0x100; //If below us, slow down a bit
						}
						else
						{
							gMC.ym += 0x100; //If moving upwards, slow down a bit
						}
					}
					else
					{
						//Bigger enemy, bounce away
						gMC.xm = -gMC.xm;
						gMC.ym = -gMC.ym;
						gMC.gm = -gMC.gm;
					}
				}
			}
			else if (gNPC[i].shock == 0 && (gNPC[i].damage || (gNPC[i].bits & NPC_REAR_AND_TOP_DONT_HURT)))
			{
				//Check for damage
				if (gNPC[i].bits & NPC_REAR_AND_TOP_DONT_HURT)
				{
					//Hit if we're on the appropriate side
					if (hit & 4 && gNPC[i].xm < 0)
						HurtMyChar(&gNPC[i]);
					if (hit & 1 && gNPC[i].xm > 0)
						HurtMyChar(&gNPC[i]);
					if (hit & 8 && gNPC[i].ym < 0)
						HurtMyChar(&gNPC[i]);
					if (hit & 2 && gNPC[i].ym > 0)
						HurtMyChar(&gNPC[i]);
				}
				else
				{
					//Hurt regardless of direction
					HurtMyChar(&gNPC[i]);
				}
			}
		}
		
		//Run script if interactable
		if ((g_GameFlags & 4) == 0 && hit && (gMC.cond & 1) && (gNPC[i].bits & NPC_INTERACTABLE))
		{
			StartTextScript(gNPC[i].code_event);
			gMC.xm = 0;
			gMC.gm = 0;
			gMC.ques = FALSE;
		}
	}
	
	//Interaction question mark
	if (gMC.ques == TRUE)
	{
		SetCaret(gMC.x, gMC.y, 9, 0);
		gMC.ques = FALSE;
	}
}

void HitMyCharBoss()
{
	//Don't run if disabled
	if ((gMC.cond & 0x80) == 0 || (gMC.cond & 2))
		return;

	//Check for NPC collisions
	for (int b = 0; b < BOSS_MAX; b++)
	{
		if ((gBoss[b].cond & 0x80) == 0)
		{
			HitMyCharNpChar_ClearStanding(&gBoss[b]);
			continue;
		}
		
		//Check if we're touching
		int hit = 0;
		if (gBoss[b].bits & NPC_SOLID_SOFT)
		{
			//Check for soft solid contact
			hit = HitMyCharNpChar_SoftSolid(&gBoss[b]);
		}
		else if (gBoss[b].bits & NPC_SOLID_HARD)
		{
			//Check for hard solid contact
			hit = HitMyCharNpChar_HardSolid(&gBoss[b]);
		}
		else
		{
			//Release if was standing on
			if (gMC.interact == &gBoss[b])
				HitMyCharNpChar_ClearStanding(&gBoss[b]);
			
			//Check for non-solid contact
			hit = HitMyCharNpChar_NonSolid(&gBoss[b]);
		}
		
		//Run event on contact
		if ((g_GameFlags & 4) == 0 && hit && (gBoss[b].bits & NPC_EVENT_WHEN_TOUCHED))
			StartTextScript(gBoss[b].code_event);
		
		//NPC damage and killing
		if ((g_GameFlags & 2) && (g_GameFlags & 4) == 0 && (gBoss[b].bits & NPC_INTERACTABLE) == 0 && hit)
		{
			//Damage if in ball form
			if ((gBoss[b].bits & NPC_SHOOTABLE) && (gMC.anim == ANIM_ROLL || gMC.anim == ANIM_SPINDASH))
			{
				if (gBoss[b].shock == 0)
				{
					//Do damage
					int b_ = DamageBoss(b, gMC.x, gMC.y, DB + gMC.rings / DI);
					gBoss[b].shock = 40;
					gBoss[b_].shock = 40;
					
					if (gBoss[b].cond & 8)
					{
						//Killed, bounce like a normal enemy
						if (gMC.ym >= 0)
						{
							if (gMC.y < gBoss[b].y)
								gMC.ym = -gMC.ym; //If above us, reverse player velocity
							else
								gMC.ym -= 0x100; //If below us, slow down a bit
						}
						else
						{
							gMC.ym += 0x100; //If moving upwards, slow down a bit
						}
					}
					else
					{
						//Bigger enemy, bounce away
						gMC.xm = -gMC.xm;
						gMC.ym = -gMC.ym;
						gMC.gm = -gMC.gm;
					}
				}
			}
			else if (gBoss[b].shock == 0 && (gBoss[b].damage || (gBoss[b].bits & NPC_REAR_AND_TOP_DONT_HURT)))
			{
				//Check for damage
				if (gBoss[b].bits & NPC_REAR_AND_TOP_DONT_HURT)
				{
					//Hit if we're on the appropriate side
					if (hit & 4 && gBoss[b].xm < 0)
						HurtMyChar(&gBoss[b]);
					if (hit & 1 && gBoss[b].xm > 0)
						HurtMyChar(&gBoss[b]);
					if (hit & 8 && gBoss[b].ym < 0)
						HurtMyChar(&gBoss[b]);
					if (hit & 2 && gBoss[b].ym > 0)
						HurtMyChar(&gBoss[b]);
				}
				else
				{
					//Hurt regardless of direction
					HurtMyChar(&gBoss[b]);
				}
			}
		}
		
		//Run script if interactable
		if ((g_GameFlags & 4) == 0 && hit && (gMC.cond & 1) && (gBoss[b].bits & NPC_INTERACTABLE))
		{
			StartTextScript(gBoss[b].code_event);
			gMC.xm = 0;
			gMC.gm = 0;
			gMC.ques = FALSE;
		}
	}
}
