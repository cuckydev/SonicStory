#pragma once
#include <stdint.h>
#include "NpChar.h"
#include "WindowsWrapper.h"

//Enums and constants
enum MYCHAR_ROUTINE
{
	ROUTINE_CONTROL,
	ROUTINE_HURT,
	ROUTINE_DEATH,
};

enum MYCHAR_ANIM
{
	ANIM_WALK,
	ANIM_RUN,
	ANIM_ROLL,
	ANIM_IDLE,
	ANIM_BALANCE,
	ANIM_INTERACT,
	ANIM_DUCK,
	ANIM_SPINDASH,
	ANIM_SKID,
	ANIM_DEAD,
	ANIM_HURT,
};

//Player struct
struct MYCHAR
{
	unsigned char cond;
	
	struct
	{
		bool inAir : 1;
		bool inBall : 1;
		bool wallLeft : 1;
		bool wallRight : 1;
		bool wallUp : 1;
		bool shouldNotFall : 1;
		bool rollJumping : 1;
		bool underwater : 1;
		bool jumping : 1;
	} status;
	
	NPCHAR *interact;
	
	int direct;
	int unit;
	MYCHAR_ROUTINE routine;
	int equip;
	int x, y;
	int tgt_x, tgt_y;
	int index_x, index_y;
	int xm, ym, gm;
	uint8_t angle, floorAngle1, floorAngle2, lastFloorAngle1, lastFloorAngle2;
	MYCHAR_ANIM anim, prev_anim;
	int anim_wait;
	int anim_frame;
	
	int xRadius, yRadius;
	struct
	{
		int front;
		int top;
		int back;
		int bottom;
	} view;
	RECT rect;
	
	unsigned char shock;
	unsigned char bubble;
	unsigned int rings;
	int air;
	signed char sprash;
	signed char ques;
	signed char boost_sw;
	int boost_cnt;
	
	long key, keyTrg;
	
	int acceleration, deceleration, top;
	
	bool spindashing;
	unsigned int spindashCounter;
};

extern MYCHAR gMC;

void InitMyChar();
void ShowMyChar(BOOL bShow);
void PutMyChar(int fx, int fy);
void LandOnFloor_ExitBall();
void ActMyChar(BOOL bKey);
void GetMyCharPosition(int *x, int *y);
void SetMyCharPosition(int x, int y);
void MoveMyChar(int x, int y);
void ZeroMyCharXMove();
int GetUnitMyChar();
void SetMyCharDirect(unsigned char dir);
void ChangeMyUnit(unsigned char a);
void PitMyChar();
void EquipItem(int flag, BOOL b);
void ResetCheck();
void SetNoise(int no, int freq);
void CutNoise();
void ResetNoise();
void SleepNoise();
