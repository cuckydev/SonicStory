#include "MyChar.h"

#include <string.h>

#include "WindowsWrapper.h"

#include "ArmsItem.h"
#include "Caret.h"
#include "Draw.h"
#include "Back.h"
#include "Flags.h"
#include "Game.h"
#include "KeyControl.h"
#include "MycParam.h"
#include "NpChar.h"
#include "Sound.h"
#include "TextScr.h"
#include "Triangle.h"
#include "ValueView.h"
#include "MycHit.h"
#include "Frame.h"
#include "Map.h"
#include "Stage.h"

MYCHAR gMC;

int noise_no;
unsigned int noise_freq;

//Collision size constants
#define XRAD_DEF	0x0900
#define YRAD_DEF	0x1300

#define XRAD_ROL	0x0700
#define YRAD_ROL	0x0E00

//Player initialization
void InitMyChar()
{
	//State and other stuff
	memset(&gMC, 0, sizeof(MYCHAR));
	gMC.cond = 0x80;
	gMC.direct = 2;

	//Collision
	gMC.xRadius = XRAD_DEF;
	gMC.yRadius = YRAD_DEF;
	
	//Speeds
	gMC.acceleration = 0x00C;
	gMC.deceleration = 0x080;
	gMC.top = 0x600;

	//Other variables
	gMC.rings = 0;
	gMC.unit = 0;
	gMC.air = 30 * 60;
	gMC.boost_cnt = 50;
}

//Player mappings
struct
{
	RECT rect;
	struct
	{
		int x, y;
	} view;
} mcMap[] = {
	{{  0,  0,  16, 24}, {0x1000, 0x1C00}},
	{{ 16,  0,  32, 24}, {0x1000, 0x1C00}},
	{{ 32,  0,  48, 24}, {0x1000, 0x1C00}},
	{{ 48,  0,  64, 24}, {0x1000, 0x1C00}},
	{{ 64,  0,  80, 24}, {0x1000, 0x1C00}},
	{{ 80,  0,  96, 24}, {0x1000, 0x1C00}},
	{{ 96,  0, 112, 24}, {0x1000, 0x1C00}},
	{{112,  0, 128, 24}, {0x1000, 0x1C00}},
	
	{{  0, 24,  16, 48}, {0x1000, 0x1C00}},
	{{ 16, 24,  32, 48}, {0x1000, 0x1C00}},
	{{ 32, 24,  48, 48}, {0x1000, 0x1C00}},
	{{ 48, 24,  64, 48}, {0x1000, 0x1C00}},
	{{ 64, 24,  80, 48}, {0x1000, 0x1C00}},
	{{ 80, 24,  96, 48}, {0x1000, 0x1C00}},
	{{ 96, 24, 112, 48}, {0x1000, 0x1C00}},
	{{112, 24, 144, 48}, {0x2000, 0x1C00}},
	
	{{128, 0, 144, 24}, {0x1000, 0x1C00}},
	{{144, 0, 160, 24}, {0x1000, 0x1C00}},
	{{160, 0, 176, 24}, {0x1000, 0x1C00}},
	{{176, 0, 192, 24}, {0x1000, 0x1C00}},
	{{192, 0, 208, 24}, {0x1000, 0x1C00}},
};

//Animation scripts
enum ANIMATION_COMMAND
{
	ANICOMMAND_RESTART = 0xFF,
	ANICOMMAND_GO_BACK_FRAMES = 0xFE,
	ANICOMMAND_SET_ANIMATION = 0xFD,
	ANICOMMAND_MIN = 0xFC,
};

static const uint8_t animationWalk[] =		{0xFF,0x02,0x03,0x02,0x01,ANICOMMAND_RESTART};
static const uint8_t animationRun[] =		{0xFF,0x0A,0x0B,ANICOMMAND_RESTART,ANICOMMAND_RESTART};
static const uint8_t animationRoll[] =		{0xFE,0x05,0x06,0x05,0x07,ANICOMMAND_RESTART};
static const uint8_t animationIdle[] =		{0x1D,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x09,ANICOMMAND_GO_BACK_FRAMES,0x02};
static const uint8_t animationBalance[] =	{0x1D,0x0D,0x0E,ANICOMMAND_RESTART};
static const uint8_t animationInteract[] =	{0x1F,0x10,ANICOMMAND_RESTART};
static const uint8_t animationDuck[] =		{0x1F,0x04,ANICOMMAND_RESTART};
static const uint8_t animationSpindash[] =	{0x00,0x11,0x12,0x11,0x13,0x11,0x14,ANICOMMAND_RESTART};
static const uint8_t animationSkid[] =		{0x1F,0x0C,ANICOMMAND_RESTART};
static const uint8_t animationDead[] =		{0x1F,0x0C,ANICOMMAND_RESTART};
static const uint8_t animationHurt[] =		{0x1F,0x0F,ANICOMMAND_RESTART};

static const uint8_t *animationList[] = {
	animationWalk,
	animationRun,
	animationRoll,
	animationIdle,
	animationBalance,
	animationInteract,
	animationDuck,
	animationSpindash,
	animationSkid,
	animationDead,
	animationHurt,
};

//Player animation
void SetMappingFrame(const uint8_t frame)
{
	//Set our rect
	gMC.rect = mcMap[frame].rect;
	
	//Set our origin point
	gMC.view.front = mcMap[frame].view.x;
	gMC.view.top = mcMap[frame].view.y;
	gMC.view.back = mcMap[frame].view.x;
	gMC.view.bottom = mcMap[frame].view.y;
}

void FrameCommand(const uint8_t *animation)
{
	switch (animation[1 + gMC.anim_frame])
	{
		case ANICOMMAND_RESTART: //Restart animation
			gMC.anim_frame = 0;
			break;
		case ANICOMMAND_GO_BACK_FRAMES: //Go back X amount of frames
			gMC.anim_frame -= animation[2 + gMC.anim_frame];
			break;
		case ANICOMMAND_SET_ANIMATION: //Switch to X animation
			gMC.anim = (MYCHAR_ANIM)animation[2 + gMC.anim_frame];
			return;
		default:
			return;
	}
	
	//Advance frame
	SetMappingFrame(animation[1 + gMC.anim_frame]);
	gMC.anim_frame++;
}

void AdvanceFrame(const uint8_t *animation)
{
	//Handle commands
	if (animation[1 + gMC.anim_frame] >= ANICOMMAND_MIN)
	{
		FrameCommand(animation);
		return;
	}
	
	//Advance frame otherwise
	SetMappingFrame(animation[1 + gMC.anim_frame]);
	gMC.anim_frame++;
}

void Animate()
{
	//Handle animation reset if changed
	if (gMC.anim != gMC.prev_anim)
	{
		gMC.prev_anim = gMC.anim;
		gMC.anim_frame = 0;
		gMC.anim_wait = 0;
	}
	
	//Get the animation to reference
	const uint8_t *animation = animationList[gMC.anim];
	
	if (animation[0] < 0x80) //Generic animation
	{
		//Wait for next frame
		if (--gMC.anim_wait >= 0)
			return;
		gMC.anim_wait = animation[0];
		
		AdvanceFrame(animation);
		return;
	}
	else if (animation[0] == 0xFF) //Is a walking variant
	{
		//Wait for next frame
		if (--gMC.anim_wait >= 0)
			return;
		
		//Get our speed factor and animation
		int speedFactor = (gMC.gm < 0 ? -gMC.gm : gMC.gm);
		
		if (speedFactor >= 0x600)
			animation = animationList[ANIM_RUN];
		else
			animation = animationList[ANIM_WALK];
		
		//Get our next frame
		if (animation[1 + gMC.anim_frame] == ANICOMMAND_RESTART)
			gMC.anim_frame = 0;
		SetMappingFrame(animation[1 + gMC.anim_frame]);
		
		if (!gMC.status.inAir && ((gMC.anim_frame & 0x1) == 0 || animation == animationList[ANIM_RUN]))
			PlaySoundObject(24, 1);
		
		//Set our frame duration
		speedFactor = -speedFactor + 0x800;
		if (speedFactor < 0)
			speedFactor = 0;
		gMC.anim_wait = speedFactor >> 7;
		
		//Increment frame
		gMC.anim_frame++;
	}
	else //Rolling animation
	{
		//Wait for next frame
		if (--gMC.anim_wait >= 0)
			return;
		
		//Get our speed factor and set our frame duration
		int speedFactor = (gMC.gm < 0 ? -gMC.gm : gMC.gm);
		speedFactor = -speedFactor + 0x400;
		if (speedFactor < 0)
			speedFactor = 0;
		gMC.anim_wait = speedFactor >> 7;
		
		//Advance frame
		AdvanceFrame(animation);
	}
}

void ShowMyChar(BOOL bShow)
{
	if (bShow)
		gMC.cond &= ~2;
	else
		gMC.cond |= 2;
}

void PutMyChar(int fx, int fy)
{
	if ((gMC.cond & 0x80) == 0 || (gMC.cond & 2))
		return;

	//Draw player
	RECT rect = gMC.rect;
	
	//Facing right offset
	if (gMC.direct)
	{
		rect.top += 48;
		rect.bottom += 48;
	}
	
	//Mimiga mask offset
	if (gMC.equip & 0x40)
	{
		rect.top += 96;
		rect.bottom += 96;
	}
	
	if (gMC.routine != ROUTINE_CONTROL || (gMC.shock & 0x4) == 0)
		PutBitmap3(&grcGame, SubpixelToScreenCoord(gMC.x - gMC.view.front) - SubpixelToScreenCoord(fx), SubpixelToScreenCoord(gMC.y - gMC.view.top) - SubpixelToScreenCoord(fy), &rect, SURFACE_ID_MY_CHAR);

	//Draw air tank
	RECT rcBubble[2] = {
		{56, 96, 80, 120},
		{80, 96, 104, 120},
	};

	++gMC.bubble;
	if (gMC.equip & 0x10 && gMC.status.underwater)
		PutBitmap3(&grcGame, SubpixelToScreenCoord(gMC.x) - PixelToScreenCoord(12) - SubpixelToScreenCoord(fx), SubpixelToScreenCoord(gMC.y) - PixelToScreenCoord(12) - SubpixelToScreenCoord(fy), &rcBubble[(gMC.bubble / 2) % 2], SURFACE_ID_CARET);
	else if (gMC.unit == 1)
		PutBitmap3(&grcGame, SubpixelToScreenCoord(gMC.x) - PixelToScreenCoord(12) - SubpixelToScreenCoord(fx), SubpixelToScreenCoord(gMC.y) - PixelToScreenCoord(12) - SubpixelToScreenCoord(fy), &rcBubble[(gMC.bubble / 2) % 2], SURFACE_ID_CARET);
}

//Functions for landing on the ground
void LandOnFloor_SetState()
{
	//Exit airborne state
	gMC.status.inAir = false;
	gMC.status.rollJumping = false;
	gMC.status.jumping = false;
	
	gMC.boost_sw = 0;
	gMC.boost_cnt = 50;
}

void LandOnFloor_ExitBall()
{
	//Keep track of our previous height
	int oldYRadius = gMC.yRadius;
	gMC.xRadius = XRAD_DEF;
	gMC.yRadius = YRAD_DEF;
	
	if (gMC.status.inBall)
	{
		//Exit ball form
		gMC.status.inBall = false;
		gMC.anim = ANIM_WALK;
		gMC.y -= (gMC.yRadius - oldYRadius);
	}

	LandOnFloor_SetState();
}

void LandOnFloor()
{
	gMC.anim = ANIM_WALK;
	LandOnFloor_ExitBall();
}

//1-point collision checks
int CheckCollisionDown_1Point(int xPos, int yPos, uint8_t *outAngle)
{
	int distance = GetCollisionV(xPos, yPos, false, outAngle);
	if (outAngle != nullptr)
		*outAngle = ((*outAngle) & 1) ? 0x00 : (*outAngle);
	return distance;
}

int CheckCollisionUp_1Point(int xPos, int yPos, uint8_t *outAngle)
{
	int distance = GetCollisionV(xPos, yPos, true, outAngle);
	if (outAngle != nullptr)
		*outAngle = ((*outAngle) & 1) ? 0x80 : (*outAngle);
	return distance;
}

int CheckCollisionLeft_1Point(int xPos, int yPos, uint8_t *outAngle)
{
	int distance = GetCollisionH(xPos, yPos, true, outAngle);
	if (outAngle != nullptr)
		*outAngle = ((*outAngle) & 1) ? 0x40 : (*outAngle);
	return distance;
}

int CheckCollisionRight_1Point(int xPos, int yPos, uint8_t *outAngle)
{
	int distance = GetCollisionH(xPos, yPos, false, outAngle);
	if (outAngle != nullptr)
		*outAngle = ((*outAngle) & 1) ? 0xC0 : (*outAngle);
	return distance;
}

//2-point collision checks
uint8_t GetCloserFloor_General(uint8_t angleSide, int *distance, int *distance2)
{
	//Get the closer floor and use that
	uint8_t outAngle = gMC.floorAngle2;
	if (*distance2 > *distance)
	{
		outAngle = gMC.floorAngle1;

		int temp = *distance; //Keep a copy because we're swapping the distances
		*distance = *distance2;
		*distance2 = temp;
	}

	//If the angle is a multi-side angled block, use our given angle side
	if (outAngle & 1)
		outAngle = angleSide;
	return outAngle;
}

void CheckCollisionDown_2Point(int xPos, int yPos, int *distance, int *distance2, uint8_t *outAngle)
{
	int retDistance = GetCollisionV(xPos + gMC.xRadius, yPos, false, &gMC.floorAngle1);
	int retDistance2 = GetCollisionV(xPos - gMC.xRadius, yPos, false, &gMC.floorAngle2);

	uint8_t retAngle = GetCloserFloor_General(0x00, &retDistance, &retDistance2);
	if (distance != nullptr)
		*distance = retDistance;
	if (distance2 != nullptr)
		*distance2 = retDistance2;
	if (outAngle != nullptr)
		*outAngle = retAngle;
}

void CheckCollisionUp_2Point(int xPos, int yPos, int *distance, int *distance2, uint8_t *outAngle)
{
	int retDistance = GetCollisionV(xPos + gMC.xRadius, yPos, true, &gMC.floorAngle1);
	int retDistance2 = GetCollisionV(xPos - gMC.xRadius, yPos, true, &gMC.floorAngle2);

	uint8_t retAngle = GetCloserFloor_General(0x80, &retDistance, &retDistance2);
	if (distance != nullptr)
		*distance = retDistance;
	if (distance2 != nullptr)
		*distance2 = retDistance2;
	if (outAngle != nullptr)
		*outAngle = retAngle;
}

void CheckCollisionLeft_2Point(int xPos, int yPos, int *distance, int *distance2, uint8_t *outAngle)
{
	int retDistance = GetCollisionH(xPos, yPos - gMC.xRadius, true, &gMC.floorAngle1);
	int retDistance2 = GetCollisionH(xPos, yPos + gMC.xRadius, true, &gMC.floorAngle2);

	uint8_t retAngle = GetCloserFloor_General(0x40, &retDistance, &retDistance2);
	if (distance != nullptr)
		*distance = retDistance;
	if (distance2 != nullptr)
		*distance2 = retDistance2;
	if (outAngle != nullptr)
		*outAngle = retAngle;
}

void CheckCollisionRight_2Point(int xPos, int yPos, int *distance, int *distance2, uint8_t *outAngle)
{
	int retDistance = GetCollisionH(xPos, yPos - gMC.xRadius, false, &gMC.floorAngle1);
	int retDistance2 = GetCollisionH(xPos, yPos + gMC.xRadius, false, &gMC.floorAngle2);

	uint8_t retAngle = GetCloserFloor_General(0xC0, &retDistance, &retDistance2);
	if (distance != nullptr)
		*distance = retDistance;
	if (distance2 != nullptr)
		*distance2 = retDistance2;
	if (outAngle != nullptr)
		*outAngle = retAngle;
}

//General collision functions
int GetWallDistance(uint8_t inAngle)
{
	int xPos = gMC.x + gMC.xm;
	int yPos = gMC.y + gMC.ym;

	gMC.floorAngle1 = inAngle;
	gMC.floorAngle2 = inAngle;
	
	uint8_t offAngle = inAngle;
	if (offAngle + 0x20 >= 0x80)
	{
		if (offAngle >= 0x80)
			--offAngle;
		offAngle += 0x20;
	}
	else
	{
		if (offAngle >= 0x80)
			++offAngle;
		offAngle += 0x1F;
	}

	switch (offAngle & 0xC0)
	{
		case 0x00: //Downwards
			return CheckCollisionDown_1Point(xPos, yPos + 0xA00, nullptr);
		case 0x80: //Upwards
			return CheckCollisionUp_1Point(xPos, yPos - 0xA00, nullptr);
		case 0x40: //Left
			//If at a shallow angle, offset the position down 8 pixels (keep us from clipping up small steps)
			if ((gMC.angle & 0x38) == 0)
				yPos += 0x800;
			return CheckCollisionLeft_1Point(xPos - 0xA00, yPos, nullptr);
		case 0xC0: //Right
			//If at a shallow angle, offset the position down 8 pixels (keep us from clipping up small steps)
			if ((gMC.angle & 0x38) == 0)
				yPos += 0x800;
			return CheckCollisionRight_1Point(xPos + 0xA00, yPos, nullptr);
	}
	return 0;
}

//Ground collision
void GroundWallCollision()
{
	int gm = (gMC.interact != nullptr) ? (gMC.interact->xm + gMC.gm) : gMC.gm;
	if (((gMC.angle & 0x3F) == 0 || ((gMC.angle + 0x40) & 0xFF) < 0x80) && gm != 0)
	{
		//Get wall distance ahead of us
		uint8_t faceAngle = gMC.angle + (gm < 0 ? 0x40 : -0x40);
		int distance = GetWallDistance(faceAngle);
		
		//Get coordinate ahead of us
		uint8_t offAngle = faceAngle;
		if (offAngle + 0x20 >= 0x80)
		{
			if (offAngle >= 0x80)
				--offAngle;
			offAngle += 0x20;
		}
		else
		{
			if (offAngle >= 0x80)
				++offAngle;
			offAngle += 0x1F;
		}
		
		int frontX = 0, frontY = 0;
		switch (offAngle & 0xC0)
		{
			case 0x00:
				frontY = 0xA00;
				break;
			case 0x40:
				frontX = -0xA00;
				break;
			case 0x80:
				frontY = -0xA00;
				break;
			case 0xC0:
				frontX = 0xA00;
				break;
		}
		
		//Get tile type ahead of us
		int frontTX = (gMC.x + gMC.xm + frontX + 0x1000) / 0x2000;
		int frontTY = (gMC.y + gMC.ym + frontY + 0x1000) / 0x2000;
		uint8_t type = GetAttribute(frontTX, frontTY);
		
		if (distance < 0)
		{
			switch ((faceAngle + 0x20) & 0xC0)
			{
				case 0x00:
				{
					gMC.ym += distance;
					break;
				}
				case 0x40:
				{
					gMC.xm -= distance;
					
					if (type != 0x43 || gMC.status.inBall == false || gMC.gm > -0x400)
					{
						//Wall, stop
						gMC.gm = 0;
					}
					else
					{
						//Breakable block, smash
						ChangeMapParts(frontTX, frontTY, 0);
						PlaySoundObject(12, 1);
						gMC.x += 0x800;
					}
					break;
				}
				case 0x80:
				{
					gMC.ym -= distance;
					break;
				}
				case 0xC0:
				{
					gMC.xm += distance;
					if (type != 0x43 || gMC.status.inBall == false || gMC.gm < 0x400)
					{
						//Wall, stop
						gMC.gm = 0;
					}
					else
					{
						//Breakable block, smash
						ChangeMapParts(frontTX, frontTY, 0);
						PlaySoundObject(12, 1);
						gMC.x -= 0x800;
					}
					break;
				}
			}
		}
	}
}

int GetCloserFloor_Ground(int distance, int distance2)
{
	//Get which distance is closer and use that for our calculation purposes
	int outDistance = distance2;
	uint8_t thisAngle = gMC.floorAngle2;
	
	if (distance2 > distance)
	{
		thisAngle = distance2 > distance ? gMC.floorAngle1 : gMC.floorAngle2;
		outDistance = distance;
	}

	//Get our angle difference
	uint8_t angleDiff = thisAngle - gMC.angle;
	if (angleDiff >= 0x80)
		angleDiff = -angleDiff;
	
	//If the angle's least significant bit is set (a tile that has different angles for each side), or the angle difference is greater than 45 degrees, use our cardinal floor angle
	if (thisAngle & 1 || angleDiff >= 0x20)
		thisAngle = (gMC.angle + 0x20) & 0xC0;

	gMC.angle = thisAngle;
	return outDistance;
}

void GroundFloorCollision()
{
	if (gMC.status.shouldNotFall)
	{
		//Default to just standing on flat ground if we're standing on an object or something
		gMC.floorAngle1 = 0;
		gMC.floorAngle2 = 0;
	}
	else
	{
		//Set flags that we're not on a floor
		gMC.floorAngle1 = 3;
		gMC.floorAngle2 = 3;
		
		//Get the angle to use for determining our ground orientation (floor, wall, or ceiling)
		uint8_t offAngle = gMC.angle;
		if (((gMC.angle + 0x20) & 0xFF) >= 0x80)
		{
			if (gMC.angle >= 0x80)
				--offAngle;
			offAngle += 0x20;
		}
		else
		{
			if (gMC.angle >= 0x80)
				++offAngle;
			offAngle += 0x1F;
		}
		
		//Handle our individual surface collisions
		switch (offAngle & 0xC0)
		{
			case 0x00: //Floor
			{
				int distance1 = GetCollisionV(gMC.x + gMC.xRadius, gMC.y + gMC.yRadius, false, &gMC.floorAngle1);
				int distance2 = GetCollisionV(gMC.x - gMC.xRadius, gMC.y + gMC.yRadius, false, &gMC.floorAngle2);
				int nearestDifference = GetCloserFloor_Ground(distance1, distance2);
				
				if (nearestDifference < 0)
				{
					//Clip up
					if (nearestDifference >= -0x1400)
						gMC.y += nearestDifference;
				}
				else if (nearestDifference > 0)
				{
					//Get how far we can clip down to the floor
					int clipLength = abs(gMC.xm) + 0x400;
					if (clipLength > 0xE00)
						clipLength = 0xE00;

					if (nearestDifference > clipLength)
					{
						//If we're running off of a ledge, enter air state
						gMC.status.inAir = true;
						gMC.prev_anim = ANIM_RUN;
					}
					else
					{
						//Move down to floor surface
						gMC.y += nearestDifference;
					}
				}
				break;
			}
		}
	}
}

//Airborne collision
void AirCollision()
{
	//Get the primary gMC.angle we're moving in
	uint8_t moveAngle = GetArktan(-gMC.xm, -gMC.ym);
	
	switch ((moveAngle - 0x20) & 0xC0)
	{
		case 0x00: //Moving downwards
		{
			int distance, distance2;
			
			//Check for wall collisions
			distance = CheckCollisionLeft_1Point(gMC.x - 0xA00, gMC.y, nullptr);
			if (distance < 0)
			{
				//Clip out and stop our velocity
				gMC.x -= distance;
				gMC.xm = 0;
			}
			
			distance = CheckCollisionRight_1Point(gMC.x + 0xA00, gMC.y, nullptr);
			if (distance < 0)
			{
				//Clip out and stop our velocity
				gMC.x += distance;
				gMC.xm = 0;
			}
			
			//Check for collision with the floor
			uint8_t floorAngle;
			CheckCollisionDown_2Point(gMC.x, gMC.y + gMC.yRadius, &distance, &distance2, &floorAngle);
			
			//Are we touching the floor (and within clip length)
			int clipLength = -(gMC.ym + 0x800);
			if (distance2 < 0 && (distance2 >= clipLength || distance >= clipLength))
			{
				//Check if we should break the block
				int belowTX = (gMC.x + 0x1000) / 0x2000;
				int belowTY = (gMC.y + gMC.yRadius + 0x1000) / 0x2000;
				uint8_t type = GetAttribute(belowTX, belowTY);
				
				if (type == 0x43 && gMC.status.inBall == true && (gMC.key & gKeyDown) != 0)
				{
					//Destroy, clip, and bounce
					ChangeMapParts(belowTX, belowTY, 0);
					PlaySoundObject(12, 1);
					gMC.y += distance2;
					gMC.ym = -0x400;
					gMC.status.jumping = false;
				}
				else
				{
					//Clip onto floor
					gMC.angle = floorAngle;
					gMC.y += distance2;
					
					//Get our gMC.gm from our global speeds
					if ((gMC.angle + 0x20) & 0x40)
					{
						//If floor is greater than 45 degrees, use our full vertical velocity (capped at 0xFC0)
						gMC.xm = 0;
						if (gMC.ym > 0xFC0)
							gMC.ym = 0xFC0;

						gMC.gm = (gMC.angle >= 0x80) ? -gMC.ym : gMC.ym;
					}
					else if ((gMC.angle + 0x10) & 0x20)
					{
						//If floor is greater than 22.5 degrees, use our halved vertical velocity
						gMC.ym /= 2;
						gMC.gm = (gMC.angle >= 0x80) ? -gMC.ym : gMC.ym;
					}
					else
					{
						//If floor is less than 22.5 degrees, use our horizontal velocity
						gMC.ym = 0;
						gMC.gm = gMC.xm;
					}
					
					//Land on floor
					LandOnFloor();
				}
			}
			break;
		}
		
		case 0x40: //Moving to the left
		{
			//Collide with walls to the left of us
			int distance = CheckCollisionLeft_1Point(gMC.x - 0xA00, gMC.y, nullptr);

			if (distance < 0)
			{
				//Clip out of the wall
				gMC.x -= distance;
				
				//Stop our velocity
				gMC.xm = 0;
				gMC.gm = gMC.ym; //This affects walk / run animations to make them usually appear slower
			}
			else
			{
				//Collide with ceilings
				CheckCollisionUp_2Point(gMC.x, gMC.y - gMC.yRadius, nullptr, &distance, nullptr);
				
				if (distance < 0)
				{
					if (distance > -0xE00)
					{
						//Clip out of ceiling
						gMC.y -= distance;
						if (gMC.ym < 0)
							gMC.ym = 0;
					}
					else
					{
						//Collide with walls to the right?
						int distance = CheckCollisionRight_1Point(gMC.x + 0xA00, gMC.y, nullptr);
						
						if (distance < 0)
						{
							gMC.x += distance;
							gMC.xm = 0;
						}
					}
				}
				else if (gMC.ym >= 0)
				{
					//Collide with the floor
					uint8_t floorAngle;
					CheckCollisionDown_2Point(gMC.x, gMC.y + gMC.yRadius, nullptr, &distance, &floorAngle);
					
					if (distance < 0)
					{
						//Clip onto floor
						gMC.y += distance;
						gMC.angle = floorAngle;
						
						//Inherit horizontal velocity
						gMC.ym = 0;
						gMC.gm = gMC.xm;
						
						//Land on floor
						LandOnFloor();
					}
				}
			}
			break;
		}
		
		case 0x80: //Moving upwards
		{
			//Check for wall collisions
			int distance;
			distance = CheckCollisionLeft_1Point(gMC.x - 0xA00, gMC.y, nullptr);
			if (distance < 0)
			{
				//Clip out of the wall and stop our velocity
				gMC.x -= distance;
				gMC.xm = 0;
			}
			
			distance = CheckCollisionRight_1Point(gMC.x + 0xA00, gMC.y, nullptr);
			if (distance < 0)
			{
				//Clip out of the wall and stop our velocity
				gMC.x += distance;
				gMC.xm = 0;
			}
			
			//Check for collision with ceilings
			uint8_t ceilingAngle;
			CheckCollisionUp_2Point(gMC.x, gMC.y - gMC.yRadius, nullptr, &distance, &ceilingAngle);
			
			if (distance < 0)
			{
				//Check if we should break the block
				int aboveTX = (gMC.x + 0x1000) / 0x2000;
				int aboveTY = (gMC.y - gMC.yRadius + 0x1000) / 0x2000;
				uint8_t type = GetAttribute(aboveTX, aboveTY);
				
				if (type == 0x43 && gMC.status.inBall == true && (gMC.key & gKeyUp) != 0)
				{
					//Destroy, clip, and bounce
					ChangeMapParts(aboveTX, aboveTY, 0);
					PlaySoundObject(12, 1);
					gMC.y += distance;
					gMC.ym = 0x100;
				}
				else
				{
					//Clip out of ceiling
					gMC.y -= distance;
					
					//If ceiling is less than 135 degrees, land on it, otherwise be stopped by it
					if (((ceilingAngle + 0x20) & 0x40) == 0)
					{
						//Stop our vertical velocity
						gMC.ym = 0;
					}
					else
					{
						//Land on ceiling
						gMC.angle = ceilingAngle;
						LandOnFloor();
						gMC.gm = (ceilingAngle >= 0x80) ? -gMC.ym : gMC.ym;
					}
				}
			}
			break;
		}
		
		case 0xC0: //Moving to the right
		{
			//Collide with walls
			int distance = CheckCollisionRight_1Point(gMC.x + 0xA00, gMC.y, nullptr);

			if (distance < 0)
			{
				//Clip out of the wall
				gMC.x += distance;
				
				//Stop our velocity
				gMC.xm = 0;
				gMC.gm = gMC.ym; //This affects walk / run animations to make them usually appear slower
			}
			else
			{
				//Collide with ceilings
				CheckCollisionUp_2Point(gMC.x, gMC.y - gMC.yRadius, nullptr, &distance, nullptr);
				
				if (distance < 0)
				{
					//Clip out of ceiling (NOTE: There's no "> -0xE00" check here, unlike moving left)
					gMC.y -= distance;
					if (gMC.ym < 0)
						gMC.ym = 0;
				}
				else if (gMC.ym >= 0)
				{
					//Collide with the floor
					uint8_t floorAngle;
					CheckCollisionDown_2Point(gMC.x, gMC.y + gMC.yRadius, nullptr, &distance, &floorAngle);
					
					if (distance < 0)
					{
						//Clip onto floor
						gMC.y += distance;
						gMC.angle = floorAngle;
						
						//Inherit horizontal velocity
						gMC.ym = 0;
						gMC.gm = gMC.xm;
						
						//Land on floor
						LandOnFloor();
					}
				}
			}
			break;
		}
	}
}

//Jump / air functions
void JumpHeight()
{
	if (gMC.status.jumping)
	{
		//Slow us down if jump is released when jumping
		int jumpRelease = gMC.status.underwater ? -0x200 : -0x400;
		if (jumpRelease > gMC.ym && (gMC.key & gKeyJump) == 0)
		{
			//We're not holding jump, slow us down
			gMC.ym = jumpRelease;
		}
	}
	else
	{
		//Cap our upwards velocity
		if (gMC.ym < -0xFC0)
			gMC.ym = -0xFC0;
	}
}

void AirMovement()
{
	//Move left and right
	if (!gMC.status.rollJumping)
	{
		int16_t newVelocity = gMC.xm;
		int16_t jumpAcceleration = gMC.acceleration << 1;
		
		//Move left if left is held
		if (gMC.key & gKeyLeft)
		{
			//Accelerate left
			gMC.direct = 0;
			newVelocity -= jumpAcceleration;
			
			//Don't accelerate past the top speed
			if (newVelocity <= -gMC.top)
			{
				newVelocity += jumpAcceleration;
				if (newVelocity >= -gMC.top)
					newVelocity = -gMC.top;
			}
		}
		
		//Move right if right is held
		if (gMC.key & gKeyRight)
		{
			//Accelerate right
			gMC.direct = 2;
			newVelocity += jumpAcceleration;
			
			//Don't accelerate past the top speed
			if (newVelocity >= gMC.top)
			{
				newVelocity -= jumpAcceleration;
				if (newVelocity <= gMC.top)
					newVelocity = gMC.top;
			}
		}
		
		//Copy our new velocity to our velocity
		gMC.xm = newVelocity;
	}
	
	//Air drag
	if (gMC.ym >= -0x400 && gMC.ym < 0)
	{
		int16_t drag = gMC.xm >> 5;
		
		if (drag > 0)
		{
			gMC.xm -= drag;
			if (gMC.xm < 0)
				gMC.xm = 0;
		}
		else if (drag < 0)
		{
			gMC.xm -= drag;
			if (gMC.xm >= 0)
				gMC.xm = 0;
		}
	}
}

bool Jump()
{
	if (gMC.keyTrg & gKeyJump)
	{
		//Apply our jump velocity
		int16_t jumpForce = 0x680;
		if (gMC.status.underwater)
			jumpForce = 0x380;
		
		gMC.xm += (GetCos(gMC.angle - 0x40) * jumpForce) >> 9;
		gMC.ym += (GetSin(gMC.angle - 0x40) * jumpForce) >> 9;
		
		//Put us in the jump state
		gMC.status.inAir = true;
		gMC.status.jumping = true;
		PlaySoundObject(SND_JUMP, 1);
		
		//Handle our collision and roll state
		if (!gMC.status.inBall)
		{
			//Go into ball form
			gMC.xRadius = XRAD_ROL;
			gMC.yRadius = YRAD_ROL;
			gMC.anim = ANIM_ROLL;
			gMC.status.inBall = true;
			gMC.y -= (gMC.yRadius - YRAD_DEF);
		}
		
		return true;
	}
	
	return false;
}

void UpdateAngleInAir()
{
	//Bring our angle down back upwards
	if (gMC.angle != 0)
	{
		if (gMC.angle >= 0x80)
		{
			gMC.angle += 2;
			if (gMC.angle < 0x80)
				gMC.angle = 0;
		}
		else
		{
			gMC.angle -= 2;
			if (gMC.angle >= 0x80)
				gMC.angle = 0;
		}
	}
}

//Rolling functions
void PerformRoll()
{
	if (!gMC.status.inBall)
	{
		//Enter ball state
		gMC.status.inBall = true;
		gMC.xRadius = XRAD_ROL;
		gMC.yRadius = YRAD_ROL;
		gMC.anim = ANIM_ROLL;
		gMC.y += (YRAD_DEF - YRAD_ROL);
		
		PlaySoundObject(SND_ROLL, 1);
	}
}

void Roll()
{
	if ((gMC.key & (gKeyLeft | gKeyRight)) == 0 && !gMC.status.inBall)
	{
		if (gMC.key & gKeyDown)
		{
			if (abs(gMC.gm) >= 0x100)
				PerformRoll();
			else
				gMC.anim = ANIM_DUCK;
		}
		else if (gMC.anim == ANIM_DUCK)
		{
			//Revert to walk animation if was ducking
			gMC.anim = ANIM_WALK;
		}
	}
}

void RollMoveLeft()
{
	if (gMC.gm <= 0)
	{
		gMC.direct = 0;
		gMC.anim = ANIM_ROLL;
	}
	else
	{
		gMC.gm -= 0x20;
		if (gMC.gm < 0)
			gMC.gm = -0x80;
	}
}
	
void RollMoveRight()
{
	if (gMC.gm >= 0)
	{
		gMC.direct = 2;
		gMC.anim = ANIM_ROLL;
	}
	else
	{
		gMC.gm += 0x20;
		if (gMC.gm >= 0)
			gMC.gm = 0x80;
	}
}

void RollMovement()
{
	//Decelerate if pulling back
	if (gMC.key & gKeyLeft)
		RollMoveLeft();
	if (gMC.key & gKeyRight)
		RollMoveRight();
	
	//Friction
	int rollDeceleration = gMC.acceleration >> 1;
	
	if (gMC.gm > 0)
	{
		gMC.gm -= rollDeceleration;
		if (gMC.gm < 0)
			gMC.gm = 0;
	}
	else if (gMC.gm < 0)
	{
		gMC.gm += rollDeceleration;
		if (gMC.gm >= 0)
			gMC.gm = 0;
	}
	
	//Stop if slowed down
	if (abs(gMC.gm) < 0x80)
	{
		//Exit ball form
		gMC.status.inBall = false;
		gMC.xRadius = XRAD_DEF;
		gMC.yRadius = YRAD_DEF;
		gMC.anim = ANIM_IDLE;
		gMC.y -= (YRAD_DEF - YRAD_ROL);
	}
	
	//Convert our inertia into global speeds
	gMC.xm = (GetCos(gMC.angle) * gMC.gm) >> 9;
	gMC.ym = (GetSin(gMC.angle) * gMC.gm) >> 9;
	
	//Cap our global horizontal speed
	if (gMC.xm <= -0x1000)
		gMC.xm = -0x1000;
	if (gMC.xm >= 0x1000)
		gMC.xm = 0x1000;
	
	//Collide with walls
	GroundWallCollision();
}

//Ground movement functions
void GroundMoveLeft()
{
	int newInertia = gMC.gm;
	
	if (newInertia <= 0)
	{
		//Flip if not already turned around
		if (gMC.direct)
		{
			gMC.direct = 0;
			gMC.prev_anim = ANIM_RUN;
		}
		
		//Accelerate
		newInertia -= gMC.acceleration;
		
		//Don't accelerate past the top speed
		if (newInertia <= -gMC.top)
		{
			newInertia += gMC.acceleration;
			if (newInertia >= -gMC.top)
				newInertia = -gMC.top;
		}
		
		//Set inertia and do walk animation
		gMC.gm = newInertia;
		gMC.anim = ANIM_WALK;
	}
	else
	{
		//Decelerate
		newInertia -= gMC.deceleration;
		if (newInertia < 0)
			newInertia = -0x80;
		gMC.gm = newInertia;
		
		//Skid if going fast enough
		if (gMC.gm >= 0x400)
		{
			PlaySoundObject(SND_SKID, 1);
			gMC.anim = ANIM_SKID;
			gMC.direct = 2;
		}
	}
}

void GroundMoveRight()
{
	int newInertia = gMC.gm;
	
	if (newInertia >= 0)
	{
		//Flip if not already turned around
		if (!gMC.direct)
		{
			gMC.direct = 2;
			gMC.prev_anim = ANIM_RUN;
		}
		
		//Accelerate
		newInertia += gMC.acceleration;
		
		//Don't accelerate past the top speed
		if (newInertia >= gMC.top)
		{
			newInertia -= gMC.acceleration;
			if (newInertia <= gMC.top)
				newInertia = gMC.top;
		}
		
		//Set inertia and do walk animation
		gMC.gm = newInertia;
		gMC.anim = ANIM_WALK;
	}
	else
	{
		//Decelerate
		newInertia += gMC.deceleration;
		if (newInertia >= 0)
			newInertia = 0x80;
		gMC.gm = newInertia;
		
		//Skid if going fast enough
		if (gMC.gm <= -0x400)
		{
			PlaySoundObject(SND_SKID, 1);
			gMC.anim = ANIM_SKID;
			gMC.direct = 0;
		}
	}
}

void GroundMovement()
{
	//Move left and right
	if (gMC.key & gKeyLeft)
		GroundMoveLeft();
	if (gMC.key & gKeyRight)
		GroundMoveRight();
	
	if (((gMC.angle + 0x20) & 0xC0) == 0 && gMC.gm == 0)
	{
		//Use idle animation
		gMC.anim = ANIM_IDLE;
		
		//Check for balancing
		if (gMC.status.shouldNotFall)
		{
			//unimplemented
		}
		else
		{
			//If Sonic's middle bottom point is 12 pixels away from the floor, start balancing
			if ((gMC.floorAngle1 = 0) == 0 && CheckCollisionDown_1Point(gMC.x, gMC.y + gMC.yRadius, &gMC.floorAngle1) >= 0xC00)
			{
				if (gMC.lastFloorAngle1 == 3) //If there's no floor to the right of us
				{
					gMC.anim = ANIM_BALANCE;
					gMC.direct = 2;
				}
				else if (gMC.lastFloorAngle2 == 3) //If there's no floor to the left of us
				{
					gMC.anim = ANIM_BALANCE;
					gMC.direct = 0;
				}
			}
		}
	}
	
	//Handle ducking and disabling waiting in cutscenes
	if (gMC.anim == ANIM_IDLE)
	{
		if ((g_GameFlags & 2) == 0)
		{
			//Lock to first frame of animation
			gMC.prev_anim = ANIM_RUN;
		}
		else
		{
			//Duck
			if (gMC.key & gKeyDown)
				gMC.anim = ANIM_DUCK;
		}
	}
	
	//Friction
	if ((gMC.key & (gKeyLeft | gKeyRight)) == 0 && gMC.gm != 0)
	{
		if (gMC.gm > 0)
		{
			gMC.gm -= gMC.acceleration;
			if (gMC.gm < 0)
				gMC.gm = 0;
		}
		else
		{
			gMC.gm += gMC.acceleration;
			if (gMC.gm >= 0)
				gMC.gm = 0;
		}
	}
	
	//Convert our ground speed into global speed then check for walls in front of us
	gMC.xm = (GetCos(gMC.angle) * gMC.gm) >> 9;
	gMC.ym = (GetSin(gMC.angle) * gMC.gm) >> 9;
	GroundWallCollision();
}

//Slope gravity functions
void RegularSlopeGravity()
{
	if (((gMC.angle + 0x60) & 0xFF) < 0xC0)
	{
		//Get our slope gravity
		int force = (GetSin(gMC.angle) * 0x20) >> 9;
		
		//Apply our slope gravity (if our inertia is non-zero, always apply, if it is 0, apply if the force is at least 0xD units per frame)
		if (gMC.gm != 0)
		{
			if (gMC.gm < 0)
				gMC.gm += force;
			else if (force != 0)
				gMC.gm += force;
		}
		else
		{
			//Apply force only if it's greater than $D units per frame (so we can stand on shallow floors)
			if (abs(force) >= 0xD)
				gMC.gm += force;
		}
	}
}

void RollingSlopeGravity()
{
	if (((gMC.angle + 0x60) & 0xFF) < 0xC0)
	{
		//Get the sin of the floor and force
		int16_t force = (GetSin(gMC.angle) * 0x50) >> 9;
		
		if (gMC.gm >= 0)
		{
			if (force < 0)
				force >>= 2;
			gMC.gm += force;
		}
		else
		{
			if (force >= 0)
				force >>= 2;
			gMC.gm += force;
		}
	}
}

//Spindash routine
const unsigned int spindashSpeed[9] =		{0x800, 0x880, 0x900, 0x980, 0xA00, 0xA80, 0xB00, 0xB80, 0xC00};

bool Spindash()
{
	if (gMC.spindashing == false)
	{
		//We must be ducking in order to spindash
		if (gMC.anim != ANIM_DUCK)
			return false;
		
		//Initiate spindash
		if (gMC.keyTrg & gKeyJump)
		{
			//Play animation and sound
			gMC.anim = ANIM_SPINDASH;
			ChangeSoundFrequency(9, 2195);
			PlaySoundObject(SND_SPINDASH_REV, 1);
			
			//Set spindash variables
			gMC.spindashing = true;
			gMC.spindashCounter = 0;
		}
		else
		{
			return false;
		}
	}
	//Release spindash
	else if ((gMC.key & gKeyDown) == 0)
	{
		//Begin rolling
		gMC.xRadius = XRAD_ROL;
		gMC.yRadius = YRAD_ROL;
		gMC.anim = ANIM_ROLL;
		
		//Offset our position to line up with the ground
		gMC.y += (YRAD_DEF - YRAD_ROL);
		
		//Release spindash
		gMC.spindashing = false;
		
		//Set our speed
		gMC.gm = spindashSpeed[gMC.spindashCounter >> 8];
		
		//Revert if facing left
		if (gMC.direct == 0)
			gMC.gm = -gMC.gm;
		
		//Actually go into the roll routine
		gMC.status.inBall = true;
		PlaySoundObject(SND_SPINDASH_REV, 0);
		PlaySoundObject(SND_SPINDASH_RELEASE, 1);
	}
	//Charging spindash
	else
	{
		//Reduce our spindash counter
		if (gMC.spindashCounter != 0)
		{
			uint16_t nextCounter = (gMC.spindashCounter - (gMC.spindashCounter >> 5));
			
			//The original makes sure the spindash counter is 0 if it underflows (which seems to be impossible, to my knowledge)
			if (nextCounter <= gMC.spindashCounter)
				gMC.spindashCounter = nextCounter;
			else
				gMC.spindashCounter = 0;
		}
		
		//Rev our spindash
		if (gMC.keyTrg & gKeyJump)
		{
			//Restart the spindash animation and play the rev sound
			gMC.anim = ANIM_SPINDASH;
			gMC.prev_anim = ANIM_WALK;
			ChangeSoundFrequency(9, 2195 + gMC.spindashCounter / 0x10);
			PlaySoundObject(SND_SPINDASH_REV, 1);
			
			//Increase our spindash counter
			gMC.spindashCounter += 0x200;
			if (gMC.spindashCounter >= 0x800)
				gMC.spindashCounter = 0x800;
		}
	}
	
	//Friction
	if ((gMC.key & (gKeyLeft | gKeyRight)) == 0 && gMC.gm != 0)
	{
		if (gMC.gm > 0)
		{
			gMC.gm -= gMC.acceleration;
			if (gMC.gm < 0)
				gMC.gm = 0;
		}
		else
		{
			gMC.gm += gMC.acceleration;
			if (gMC.gm >= 0)
				gMC.gm = 0;
		}
	}
	
	//Move and collide with the level
	gMC.xm = (GetCos(gMC.angle) * gMC.gm) >> 9;
	gMC.ym = (GetSin(gMC.angle) * gMC.gm) >> 9;
	GroundWallCollision();
	gMC.x += gMC.xm;
	gMC.y += gMC.ym;
	GroundFloorCollision();
	return true;
}

//Water function
bool TypeIsWater(uint8_t type)
{
	switch (type)
	{
		case 0x62:
			return true;
		case 0x02:
			return true;
		case 0x60:
			return true;
		case 0x61:
			return true;
		case 0x70:
			return true;
		case 0x71:
			return true;
		case 0x72:
			return true;
		case 0x73:
			return true;
		case 0x74:
			return true;
		case 0x75:
			return true;
		case 0x76:
			return true;
		case 0x77:
			return true;
		case 0xA0:
			return true;
		case 0xA1:
			return true;
		case 0xA2:
			return true;
		case 0xA3:
			return true;
	}
	return false;
}

void MyCharWater()
{
	//Get if we are underwater
	gMC.status.underwater = false;
	if (gMC.y > gWaterY + 0x800)
		gMC.status.underwater = true;
	
	uint8_t bodyType = GetAttribute((gMC.x + 0x1000) / 0x2000, (gMC.y + 0x1000) / 0x2000);
	if (TypeIsWater(bodyType))
		gMC.status.underwater = true;
	
	//Handle entering / exiting water
	int dir;
	if (bodyType == 0x62)
		dir = 2; //Blood
	else
		dir = 0; //Water
	
	if (gMC.status.underwater && !gMC.sprash)
	{
		//Enter
		gMC.top = 0x300;
		gMC.acceleration = 6;
		gMC.deceleration = 0x40;
		gMC.xm >>= 1;
		gMC.ym >>= 2;
		
		//Splash
		if (gMC.ym)
		{
			PlaySoundObject(56, 1);
			for (int a = 0; a < 8; a++)
			{
				int x = gMC.x + (Random(-8, 8) * 0x200);
				SetNpChar(73, x, gMC.y, gMC.xm + Random(-0x200, 0x200), Random(-0x200, 0x80) - abs(gMC.ym) / 2, dir, 0, 0);
			}
		}
	}
	else if (gMC.sprash && !gMC.status.underwater)
	{
		//Exit
		gMC.top = 0x600;
		gMC.acceleration = 0xC;
		gMC.deceleration = 0x80;
		if (gMC.routine != ROUTINE_HURT)
			gMC.ym <<= 1;
		
		//Splash
		if (gMC.ym)
		{
			PlaySoundObject(56, 1);
			for (int a = 0; a < 8; a++)
			{
				int x = gMC.x + (Random(-8, 8) * 0x200);
				SetNpChar(73, x, gMC.y, gMC.xm + Random(-0x200, 0x200), Random(-0x200, 0x80) - abs(gMC.ym) / 2, dir, 0, 0);
			}
		}
	}
	
	gMC.sprash = gMC.status.underwater;
	
	//Handle air
	if (gMC.status.underwater && (gMC.equip & 0x10) == 0)
	{
		if (gMC.air)
		{
			if (--gMC.air <= 0)
			{
				if (GetNPCFlag(4000))
				{
					//Core cutscene
					StartTextScript(1100);
				}
				else
				{
					LandOnFloor_ExitBall();
					gMC.xm = 0;
					gMC.ym = 0;
					gMC.routine = ROUTINE_DEATH;
					gMC.anim = ANIM_DEAD;
					StartTextScript(40);
				}
			}
		}
	}
	else
		gMC.air = 30*60;
}

//Vector function
void MyCharVector()
{
	//Handle wind tiles
	uint8_t bodyType = GetAttribute((gMC.x + 0x1000) / 0x2000, (gMC.y + 0x1000) / 0x2000);
	if (bodyType == 0x80 || bodyType == 0xA0)
	{
		if (!(gMC.xm <= -0x5FF && gMC.gm <= -0x5FF))
		{
			gMC.xm -= 0x88;
			gMC.gm -= 0x88;
		}
	}
	if (bodyType == 0x81 || bodyType == 0xA1)
	{
		if (gMC.ym > -0x5FF)
			gMC.ym -= 0x80;
		gMC.status.inAir = true;
		gMC.status.jumping = false;
	}
	if (bodyType == 0x82 || bodyType == 0xA2)
	{
		if (!(gMC.xm >= 0x5FF && gMC.gm >= 0x5FF))
		{
			gMC.xm += 0x88;
			gMC.gm += 0x88;
		}
	}
	if (bodyType == 0x83 || bodyType == 0xA3)
	{
		if (gMC.ym < 0x5FF)
			gMC.ym += 0x80;
	}
	
	//Secondary wind hack
	if (gMC.cond & 0x20)
	{
		gMC.cond &= ~0x20;
		if (gMC.xm < -0x1000)
			gMC.xm = -0x1000;
		if (gMC.xm > 0x1000)
			gMC.xm = 0x1000;
	}
}

//Spikes function
void MyCharSpike()
{
	uint8_t bodyType = GetAttribute((gMC.x + 0x1000) / 0x2000, (gMC.y + 0x1000) / 0x2000);
	if (bodyType == 0x42 || bodyType == 0x62)
		HurtMyChar(nullptr);
}

//Booster function
bool MyCharBoost()
{
	//Booster 0.8
	if (gMC.equip & 0x1)
	{
		if (gMC.boost_sw == 0 && gMC.boost_cnt != 0 && (gMC.keyTrg & gKeyJump))
		{
			//Initiate booster
			gMC.boost_sw = 1;
			if (gMC.ym > 0x100)
				gMC.ym /= 2;
		}
		
		//Handle boosting each frame
		if (gMC.boost_cnt > 0)
		{
			if (gMC.boost_sw != 0 && (gMC.key & gKeyJump))
			{
				if (--gMC.boost_cnt >= 0)
				{
					if (gMC.ym > -0x400)
					{
						//Upwards force
						gMC.ym -= 0x20;

						if (!(gMC.boost_cnt % 3))
						{
							SetCaret(gMC.x, gMC.yRadius / 2 + gMC.y, 7, 3);
							PlaySoundObject(113, 1);
						}

						//Bounce off of ceiling
						if (gMC.status.wallUp == true)
							gMC.ym = 0x200;
					}
					else
					{
						//Gravity
						gMC.ym += gMC.status.underwater ? 0x10 : 0x20;
					}
					
					AirMovement();
					return true;
				}
			}
		}
		else
		{
			//End boosting and make sure our fuel is 0
			gMC.boost_cnt = 0;
		}
		
		//End boosting
		gMC.boost_sw = 0;
	}
	
	//Booster 2.0
	if (gMC.equip & 0x20)
	{
		//Check if we should start boosting
		if (gMC.boost_sw == 0 && gMC.boost_cnt != 0 && (gMC.keyTrg & gKeyJump))
		{
			int dirMask = gMC.key & (gKeyLeft | gKeyUp | gKeyRight | gKeyDown);
			
			if ((dirMask & gKeyUp) || dirMask == 0)
			{
				//Boost upwards
				gMC.boost_sw = 2;
				gMC.xm = 0;
				gMC.ym = -0x5FF;
			}
			else if (dirMask & gKeyLeft)
			{
				//Boost left
				gMC.boost_sw = 1;
				gMC.ym = 0;
				gMC.xm = -0x5FF;
			}
			else if (dirMask & gKeyRight)
			{
				//Boost right
				gMC.boost_sw = 1;
				gMC.ym = 0;
				gMC.xm = 0x5FF;
			}
			else if (dirMask & gKeyDown)
			{
				//Boost down
				gMC.boost_sw = 3;
				gMC.xm = 0;
				gMC.ym = 0x5FF;
			}
		}
		
		//Handle boosting each frame
		if (gMC.boost_cnt > 0)
		{
			if (gMC.boost_sw != 0 && (gMC.key & gKeyJump))
			{
				//Reduce fuel and move
				if (--gMC.boost_cnt >= 0)
				{
					int maxBoostSpd = gMC.status.underwater ? 0x2FF : 0x5FF;
					
					if (gMC.boost_sw == 1)
					{
						//Go up when going into a wall
						if (gMC.status.wallLeft || gMC.status.wallRight)
							gMC.ym = -0x100;
						
						//Turn around
						if (gMC.key & gKeyLeft)
							gMC.direct = 0;
						if (gMC.key & gKeyRight)
							gMC.direct = 2;

						//Move in direction facing
						if (gMC.direct == 0)
							gMC.xm -= 0x20;
						if (gMC.direct == 2)
							gMC.xm += 0x20;
						
						//Cap speed
						if (gMC.xm < -maxBoostSpd)
							gMC.xm = -maxBoostSpd;
						if (gMC.xm > maxBoostSpd)
							gMC.xm = maxBoostSpd;
						
						//Update inertia for animation
						gMC.gm = gMC.xm;

						//Boost particles (and sound)
						if ((gMC.keyTrg & gKeyJump) || gMC.boost_cnt % 3 == 1)
						{
							if (!gMC.direct)
								SetCaret(gMC.x + 0x400, gMC.y + 0x400, 7, 2);
							if (gMC.direct == 2)
								SetCaret(gMC.x - 0x400, gMC.y + 0x400, 7, 0);

							PlaySoundObject(113, 1);
						}
					}
					else if (gMC.boost_sw == 2)
					{
						//Move upwards
						gMC.ym -= 0x20;
						if (gMC.ym < -maxBoostSpd)
							gMC.ym = -maxBoostSpd;
						
						//Update inertia for animation
						gMC.gm = gMC.ym;
						
						//Move side to side
						AirMovement();

						//Boost particles (and sound)
						if ((gMC.keyTrg & gKeyJump) || gMC.boost_cnt % 3 == 1)
						{
							SetCaret(gMC.x, gMC.y + 0xC00, 7, 3);
							PlaySoundObject(113, 1);
						}
					}
					else if (gMC.boost_sw == 3 && ((gMC.keyTrg & gKeyJump) || gMC.boost_cnt % 3 == 1))
					{
						//Move downwards
						gMC.ym += 0x20;
						if (gMC.ym > maxBoostSpd)
							gMC.ym = maxBoostSpd;
						
						//Update inertia for animation
						gMC.gm = gMC.ym;
						
						//Move side to side
						AirMovement();
						
						//Boost particles (and sound)
						SetCaret(gMC.x, gMC.y - 0xC00, 7, 1);
						PlaySoundObject(113, 1);
					}
					return true;
				}
			}
		}
		else
		{
			//End boosting and make sure our fuel is 0
			gMC.boost_cnt = 0;
		}
		
		//End boosting
		if (gMC.boost_sw == 1)
			gMC.xm /= 2;
		else if (gMC.boost_sw == 2)
			gMC.ym /= 2;
		gMC.boost_sw = 0;
		return false;
	}
	
	return false;
}

//Control routine
void ActMyChar_Control()
{
	//Clear interaction if other keys are pressed
	bool interactFilter = (gMC.key & (gKeyLeft | gKeyRight | gKeyDown | gKeyJump)) || gMC.status.inAir || gMC.status.inBall;
	if (interactFilter)
		gMC.cond &= ~0x01;
		
	if (gMC.status.inAir == false)
	{
		if (gMC.status.inBall == false) //Normal ground movement
		{
			//Check for jumping and spindashing
			if (Spindash())
				return;
			if (Jump())
				return;
			
			//Handle our movement
			RegularSlopeGravity();
			GroundMovement();
			Roll();
			
			//Move from velocity, then check for collision with floors below us
			gMC.x += gMC.xm;
			gMC.y += gMC.ym;
			GroundFloorCollision();
			
			//Handle interaction
			if ((gMC.cond & 0x01) == 0 && (gMC.keyTrg & gKeyUp) && !interactFilter)
			{
				gMC.cond |= 0x01;
				gMC.ques = TRUE;
			}
			
			if (gMC.cond & 0x01)
				gMC.anim = ANIM_INTERACT;
		}
		else //Rolling
		{
			//Check for jumping
			if (Jump())
				return;
			
			//Handle our movement
			RollingSlopeGravity();
			RollMovement();
			
			//Move from velocity, then check for collision with floors below us
			gMC.x += gMC.xm;
			gMC.y += gMC.ym;
			GroundFloorCollision();
		}
	}
	else //In-air
	{
		//Cancel spindash
		if (gMC.spindashing)
		{
			gMC.spindashing = false;
			gMC.anim = ANIM_ROLL;
		}
		
		//Booster update
		bool boostRes = MyCharBoost();
		
		if (boostRes == false)
		{
			//Handle our movement
			JumpHeight();
			AirMovement();
		}
		
		//Move according to our velocity
		gMC.x += gMC.xm;
		gMC.y += gMC.ym;
		
		if (boostRes == false)
		{
			//Gravity (0x38 above water, 0x10 below water)
			gMC.ym += 0x38;
			if (gMC.status.underwater)
				gMC.ym -= 0x28;
			if (gStageNo == 80 && gMC.ym > 0x5FF)
				gMC.ym = 0x5FF;
		}
		
		//Update airborne angle and collide with stage
		UpdateAngleInAir();
		AirCollision();
	}
}

//Stream update
void ActMyChar_Stream(BOOL bKey)
{
	//Force into roll animation
	gMC.anim = ANIM_ROLL;
	gMC.status.underwater = true;
	
	//Handle movement
	if (bKey)
	{
		if (gKey & (gKeyRight | gKeyLeft))
		{
			if (gKey & gKeyLeft)
				gMC.xm -= 0x20;

			if (gKey & gKeyRight)
				gMC.xm += 0x20;
		}
		else if (gMC.xm < 0x20 && gMC.xm > -0x20)
		{
			gMC.xm = 0;
		}
		else if (gMC.xm > 0)
		{
			gMC.xm -= 0x20;
		}
		else if (gMC.xm < 0)
		{
			gMC.xm += 0x20;
		}

		if (gKey & (gKeyDown | gKeyUp))
		{
			if (gKey & gKeyUp)
				gMC.ym -= 0x20;

			if (gKey & gKeyDown)
				gMC.ym += 0x20;
		}
		else if (gMC.ym < 0x20 && gMC.ym > -0x20)
		{
			gMC.ym = 0;
		}
		else if (gMC.ym > 0)
		{
			gMC.ym -= 0x20;
		}
		else if (gMC.ym < 0)
		{
			gMC.ym += 0x20;
		}
	}
	else
	{
		if (gMC.xm < 0x20 && gMC.xm > -0x20)
			gMC.xm = 0;
		else if (gMC.xm > 0)
			gMC.xm -= 0x20;
		else if (gMC.xm < 0)
			gMC.xm += 0x20;

		if (gMC.ym < 0x20 && gMC.ym > -0x20)
			gMC.ym = 0;
		else if (gMC.ym > 0)
			gMC.ym -= 0x20;
		else if (gMC.ym < 0)
			gMC.ym += 0x20;
	}

	if (gMC.xm > 0x400)
		gMC.xm = 0x400;
	if (gMC.xm < -0x400)
		gMC.xm = -0x400;

	if (gMC.ym > 0x400)
		gMC.ym = 0x400;
	if (gMC.ym < -0x400)
		gMC.ym = -0x400;

	if ((gKey & (gKeyUp | gKeyLeft)) == (gKeyLeft | gKeyUp))
	{
		if (gMC.xm < -780)
			gMC.xm = -780;
		if (gMC.ym < -780)
			gMC.ym = -780;
	}

	if ((gKey & (gKeyUp | gKeyRight)) == (gKeyRight | gKeyUp))
	{
		if (gMC.xm > 780)
			gMC.xm = 780;
		if (gMC.ym < -780)
			gMC.ym = -780;
	}

	if ((gKey & (gKeyDown | gKeyLeft)) == (gKeyLeft | gKeyDown))
	{
		if (gMC.xm < -780)
			gMC.xm = -780;
		if (gMC.ym > 780)
			gMC.ym = 780;
	}

	if ((gKey & (gKeyDown | gKeyRight)) == (gKeyRight | gKeyDown))
	{
		if (gMC.xm > 780)
			gMC.xm = 780;
		if (gMC.ym > 780)
			gMC.ym = 780;
	}

	gMC.x += gMC.xm;
	gMC.y += gMC.ym;
	
	//Collision
	AirCollision();
}

//Update
void ActMyChar(BOOL bKey)
{
	//Don't update if disabled
	if ((gMC.cond & 0x80) == 0 || (gMC.cond & 0x2) != 0)
		return;
	
	//Update us
	switch (gMC.routine)
	{
		case ROUTINE_CONTROL:
			//Copy inputs
			if (bKey)
			{
				gMC.key = gKey;
				gMC.keyTrg = gKeyTrg;
			}
			else
			{
				gMC.key = 0;
				gMC.keyTrg = 0;
			}
			
			//Update
			if (!gMC.unit)
				ActMyChar_Control();
			else
				ActMyChar_Stream(bKey);
			MyCharWater();
			MyCharVector();
			MyCharSpike();
			
			//Copy floor angles
			gMC.lastFloorAngle1 = gMC.floorAngle1;
			gMC.lastFloorAngle2 = gMC.floorAngle2;
			
			//Animate and decrease shock
			Animate();
			if (gMC.shock)
				gMC.shock--;
			break;
		case ROUTINE_HURT:
			//Move and fall to gravity
			gMC.x += gMC.xm;
			gMC.y += gMC.ym;
			
			if (gMC.unit == 0)
			{
				gMC.ym += 0x30;
				if (gMC.status.underwater)
					gMC.ym -= 0x20;
			}
			else
			{
				gMC.xm -= 0x8;
				gMC.ym = 0;
			}
			
			//Collide with level and stop once touched floor
			AirCollision();
			
			if (!gMC.status.inAir || (gMC.unit && (gMC.shock -= 5) <= 0))
			{
				//Clear our speed
				gMC.xm = 0;
				gMC.ym = 0;
				gMC.gm = 0;
				
				//Reset animation and routine
				gMC.anim = ANIM_WALK;
				gMC.routine = ROUTINE_CONTROL;
				
				//Restart invulnerability timer
				gMC.shock = 120;
			}
			
			//Water check
			MyCharWater();
			
			//Animate
			Animate();
			break;
		case ROUTINE_DEATH:
			//Move and fall to gravity
			gMC.x += gMC.xm;
			gMC.y += gMC.ym;
			gMC.ym += 0x38;
			if (gMC.status.underwater)
				gMC.ym -= 0x28;
			
			//Animate
			Animate();
			break;
	}
	
	//Collision status update
	int dleft = CheckCollisionLeft_1Point(gMC.x - 0x1000, gMC.y, nullptr);
	int dup = CheckCollisionUp_1Point(gMC.x, gMC.y - gMC.yRadius, nullptr);
	int dright = CheckCollisionRight_1Point(gMC.x + 0x1000, gMC.y, nullptr);
	
	if (dleft <= 0)
		gMC.status.wallLeft = true;
	else
		gMC.status.wallLeft = false;
	if (dright <= 0)
		gMC.status.wallRight = true;
	else
		gMC.status.wallRight = false;
	if (dup <= 0)
		gMC.status.wallUp = true;
	else
		gMC.status.wallUp = false;
	
	//Camera update
	if (gMC.unit == 0)
	{
		if (gMC.routine == ROUTINE_DEATH)
		{
			//Lock camera to coordinates of death
			gMC.tgt_x = gMC.index_x;
			gMC.tgt_y = gMC.index_y;
		}
		else
		{
			//Update camera
			gMC.tgt_x = gMC.x + (gMC.xm * gFrame.wait * 5 / 4);
			gMC.tgt_y = gMC.y + (gMC.ym * gFrame.wait * 5 / 4) + (gMC.yRadius - YRAD_DEF);
			gMC.index_x = gMC.x;
			gMC.index_y = gMC.y;
		}
	}
}

void GetMyCharPosition(int *x, int *y)
{
	*x = gMC.x;
	*y = gMC.y;
}

void SetMyCharPosition(int x, int y)
{
	gMC.status.inBall = false;
	gMC.xRadius = XRAD_DEF;
	gMC.yRadius = YRAD_DEF;
	gMC.x = x;
	gMC.y = y + 0x1000 - gMC.yRadius;
	gMC.tgt_x = gMC.x;
	gMC.tgt_y = gMC.y;
	gMC.index_x = gMC.x;
	gMC.index_y = gMC.y;
	gMC.xm = 0;
	gMC.ym = 0;
	gMC.gm = 0;
	gMC.cond &= ~1;
}

void MoveMyChar(int x, int y)
{
	gMC.x = x;
	gMC.y = y + 0x1000 - gMC.yRadius;
}

void ZeroMyCharXMove()
{
	gMC.xm = 0;
	gMC.gm = 0;
}

int GetUnitMyChar()
{
	return gMC.unit;
}

void SetMyCharDirect(unsigned char dir)
{
	if (dir == 3)
	{
		gMC.cond |= 1;
	}
	else
	{
		gMC.cond &= ~1;

		if (dir < 10)
		{
			gMC.direct = dir;
		}
		else
		{
			int i;
			for (i = 0; i < NPC_MAX; i++)
				if (gNPC[i].code_event == dir)
					break;

			if (i == NPC_MAX)
				return;

			if (gMC.x > gNPC[i].x)
				gMC.direct = 0;
			else
				gMC.direct = 2;
		}
	}

	gMC.xm = 0;
	gMC.gm = 0;
}

void ChangeMyUnit(unsigned char a)
{
	gMC.unit = a;
}

void PitMyChar()
{
	gMC.y += 0x4000;
}

void EquipItem(int flag, BOOL b)
{
	if (b)
		gMC.equip |= flag;
	else
		gMC.equip &= ~flag;
}

void ResetCheck()
{
	gMC.cond &= ~1;
}

void SetNoise(int no, int freq)
{
	noise_freq = freq;
	noise_no = no;

	switch (noise_no)
	{
		case 1:
			ChangeSoundFrequency(40, noise_freq);
			ChangeSoundFrequency(41, noise_freq + 100);
			PlaySoundObject(40, -1);
			PlaySoundObject(41, -1);
			break;

		case 2:
			PlaySoundObject(58, -1);
			break;
	}
}

void CutNoise()
{
	noise_no = 0;
	PlaySoundObject(40, 0);
	PlaySoundObject(41, 0);
	PlaySoundObject(58, 0);
}

void ResetNoise()
{
	switch (noise_no)
	{
		case 1:
			ChangeSoundFrequency(40, noise_freq);
			ChangeSoundFrequency(41, noise_freq + 100);
			PlaySoundObject(40, -1);
			PlaySoundObject(41, -1);
			break;

		case 2:
			PlaySoundObject(58, -1);
			break;
	}
}

void SleepNoise()
{
	PlaySoundObject(40, 0);
	PlaySoundObject(41, 0);
	PlaySoundObject(58, 0);
}
