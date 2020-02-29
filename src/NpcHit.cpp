#include "NpcHit.h"

#include "WindowsWrapper.h"

#include "Back.h"
#include "Bullet.h"
#include "Caret.h"
#include "Flags.h"
#include "Game.h"
#include "Map.h"
#include "MyChar.h"
#include "NpChar.h"
#include "Sound.h"
#include "TextScr.h"
#include "ValueView.h"

void JadgeHitNpCharBlock(NPCHAR *npc, int x, int y)
{
	int hit = 0;

	if (npc->y - npc->hit.top < (y * 0x10 + 5) * 0x200
		&& npc->y + npc->hit.bottom > (y * 0x10 - 5) * 0x200
		&& npc->x - npc->hit.back < (x * 0x10 + 8) * 0x200
		&& npc->x - npc->hit.back > x * 0x10 * 0x200)
	{
		npc->x = ((x * 0x10 + 8) * 0x200) + npc->hit.back;
		hit |= 1;
	}

	if (npc->y - npc->hit.top < (y * 0x10 + 5) * 0x200
		&& npc->y + npc->hit.bottom > (y * 0x10 - 5) * 0x200
		&& npc->x + npc->hit.back > (x * 0x10 - 8) * 0x200
		&& npc->x + npc->hit.back < x * 0x10 * 0x200)
	{
		npc->x = ((x * 0x10 - 8) * 0x200) - npc->hit.back;
		hit |= 4;
	}

	if (npc->x - npc->hit.back < (x * 0x10 + 5) * 0x200
		&& npc->x + npc->hit.back > (x * 0x10 - 5) * 0x200
		&& npc->y - npc->hit.top < (y * 0x10 + 8) * 0x200
		&& npc->y - npc->hit.top > y * 0x10 * 0x200)
	{
		npc->y = ((y * 0x10 + 8) * 0x200) + npc->hit.top;
		npc->ym = 0;
		hit |= 2;
	}

	if (npc->x - npc->hit.back < (x * 0x10 + 5) * 0x200
		&& npc->x + npc->hit.back > (x * 0x10 - 5) * 0x200
		&& npc->y + npc->hit.bottom > (y * 0x10 - 8) * 0x200
		&& npc->y + npc->hit.bottom < y * 0x10 * 0x200)
	{
		npc->y = ((y * 0x10 - 8) * 0x200) - npc->hit.bottom;
		npc->ym = 0;
		hit |= 8;
	}

	npc->flag |= hit;
}

void JudgeHitNpCharTriangleA(NPCHAR *npc, int x, int y)
{
	int hit = 0;

	if (npc->x < (x * 0x10 + 8) * 0x200
		&& npc->x > (x * 0x10 - 8) * 0x200
		&& npc->y - npc->hit.top < (y * 0x10 * 0x200) - ((npc->x - (x * 0x10 * 0x200)) / 2) + 0x800
		&& npc->y + npc->hit.bottom > (y * 0x10 - 8) * 0x200)
	{
		// Clip
		npc->y = (y * 0x10 * 0x200) - ((npc->x - (x * 0x10 * 0x200)) / 2) + 0x800 + npc->hit.top;

		// Halt momentum
		if (npc->ym < 0)
			npc->ym = 0;

		// Set that hit a ceiling
		hit |= 2;
	}

	npc->flag |= hit;
}

void JudgeHitNpCharTriangleB(NPCHAR *npc, int x, int y)
{
	int hit = 0;

	if (npc->x < (x * 0x10 + 8) * 0x200
		&& npc->x > (x * 0x10 - 8) * 0x200
		&& npc->y - npc->hit.top < (y * 0x10 * 0x200) - ((npc->x - (x * 0x10 * 0x200)) / 2) - 0x800
		&& npc->y + npc->hit.bottom > (y * 0x10 - 8) * 0x200)
	{
		// Clip
		npc->y = (y * 0x10 * 0x200) - ((npc->x - (x * 0x10 * 0x200)) / 2) - 0x800 + npc->hit.top;

		// Halt momentum
		if (npc->ym < 0)
			npc->ym = 0;

		// Set that hit a ceiling
		hit |= 2;
	}

	npc->flag |= hit;
}

void JudgeHitNpCharTriangleC(NPCHAR *npc, int x, int y)
{
	int hit = 0;

	if (npc->x < (x * 0x10 + 8) * 0x200
		&& npc->x > (x * 0x10 - 8) * 0x200
		&& npc->y - npc->hit.top < (y * 0x10 * 0x200) + ((npc->x - (x * 0x10 * 0x200)) / 2) - 0x800
		&& npc->y + npc->hit.bottom > (y * 0x10 - 8) * 0x200)
	{
		// Clip
		npc->y = (y * 0x10 * 0x200) + ((npc->x - (x * 0x10 * 0x200)) / 2) - 0x800 + npc->hit.top;

		// Halt momentum
		if (npc->ym < 0)
			npc->ym = 0;

		// Set that hit a ceiling
		hit |= 2;
	}

	npc->flag |= hit;
}

void JudgeHitNpCharTriangleD(NPCHAR *npc, int x, int y)
{
	int hit = 0;

	if (npc->x < (x * 0x10 + 8) * 0x200
		&& npc->x > (x * 0x10 - 8) * 0x200
		&& npc->y - npc->hit.top < (y * 0x10 * 0x200) + ((npc->x - (x * 0x10 * 0x200)) / 2) + 0x800
		&& npc->y + npc->hit.bottom > (y * 0x10 - 8) * 0x200)
	{
		// Clip
		npc->y = (y * 0x10 * 0x200) + ((npc->x - (x * 0x10 * 0x200)) / 2) + 0x800 + npc->hit.top;

		// Halt momentum
		if (npc->ym < 0)
			npc->ym = 0;

		// Set that hit a ceiling
		hit |= 2;
	}

	npc->flag |= hit;
}

void JudgeHitNpCharTriangleE(NPCHAR *npc, int x, int y)
{
	int hit = 0;

	hit |= 0x10000;

	if (npc->x < (x * 0x10 + 8) * 0x200
		&& npc->x > (x * 0x10 - 8) * 0x200
		&& npc->y + npc->hit.bottom > (y * 0x10 * 0x200) + ((npc->x - (x * 0x10 * 0x200)) / 2) - 0x800
		&& npc->y - npc->hit.top < (y * 0x10 + 8) * 0x200)
	{
		// Clip
		npc->y = (y * 0x10 * 0x200) + ((npc->x - (x * 0x10 * 0x200)) / 2) - 0x800 - npc->hit.bottom;

		// Halt momentum
		if (npc->ym > 0)
			npc->ym = 0;

		// Set that hit this slope
		hit |= 0x28;
	}

	npc->flag |= hit;
}

void JudgeHitNpCharTriangleF(NPCHAR *npc, int x, int y)
{
	int hit = 0;

	hit |= 0x20000;

	if (npc->x < (x * 0x10 + 8) * 0x200
		&& npc->x >= (x * 0x10 - 8) * 0x200	// Note that this function uses '>='. I'm not sure if this is a bug.
		&& npc->y + npc->hit.bottom > (y * 0x10 * 0x200) + ((npc->x - (x * 0x10 * 0x200)) / 2) + 0x800
		&& npc->y - npc->hit.top < (y * 0x10 + 8) * 0x200)
	{
		// Clip
		npc->y = (y * 0x10 * 0x200) + ((npc->x - (x * 0x10 * 0x200)) / 2) + 0x800 - npc->hit.bottom;

		// Halt momentum
		if (npc->ym > 0)
			npc->ym = 0;

		// Set that hit this slope
		hit |= 0x28;
	}

	npc->flag |= hit;
}

void JudgeHitNpCharTriangleG(NPCHAR *npc, int x, int y)
{
	int hit = 0;

	hit |= 0x40000;

	if (npc->x < (x * 0x10 + 8) * 0x200
		&& npc->x > (x * 0x10 - 8) * 0x200
		&& npc->y + npc->hit.bottom > (y * 0x10 * 0x200) - ((npc->x - (x * 0x10 * 0x200)) / 2) + 0x800
		&& npc->y - npc->hit.top < (y * 0x10 + 8) * 0x200)
	{
		// Clip
		npc->y = (y * 0x10 * 0x200) - ((npc->x - (x * 0x10 * 0x200)) / 2) + 0x800 - npc->hit.bottom;

		// Halt momentum
		if (npc->ym > 0)
			npc->ym = 0;

		// Set that hit this slope
		hit |= 0x18;
	}

	npc->flag |= hit;
}

void JudgeHitNpCharTriangleH(NPCHAR *npc, int x, int y)
{
	int hit = 0;

	hit |= 0x80000;

	if (npc->x < (x * 0x10 + 8) * 0x200
		&& npc->x > (x * 0x10 - 8) * 0x200
		&& npc->y + npc->hit.bottom > (y * 0x10 * 0x200) - ((npc->x - (x * 0x10 * 0x200)) / 2) - 0x800
		&& npc->y - npc->hit.top < (y * 0x10 + 8) * 0x200)
	{
		// Clip
		npc->y = (y * 0x10 * 0x200) - ((npc->x - (x * 0x10 * 0x200)) / 2) - 0x800 - npc->hit.bottom;

		// Halt momentum
		if (npc->ym > 0)
			npc->ym = 0;

		// Set that hit this slope
		hit |= 0x18;
	}

	npc->flag |= hit;
}

void JudgeHitNpCharWater(NPCHAR *npc, int x, int y)
{
	int hit = 0;

	if (npc->x - npc->hit.back < (x * 0x10 + 6) * 0x200
		&& npc->x + npc->hit.back > (x * 0x10 - 6) * 0x200
		&& npc->y - npc->hit.top < (y * 0x10 + 6) * 0x200
		&& npc->y + npc->hit.bottom > (y * 0x10 - 6) * 0x200)
		hit |= 0x100;

	npc->flag |= hit;
}

void DamageNpChar(NPCHAR *npc, int srcX, int srcY, int damage)
{
	npc->life -= damage;

	if (npc->life < 1)
	{
		npc->life = 0;

		if (npc->bits & NPC_SHOW_DAMAGE)
			npc->damage_view -= damage;

		if ((gMC.cond & 0x80) && npc->bits & NPC_EVENT_WHEN_KILLED)
			StartTextScript(npc->code_event);
		else
			npc->cond |= 8;
	}
	else
	{
		if (npc->shock < 14)
		{
			SetCaret((srcX + npc->x) / 2, (srcY + npc->y) / 2, 11, 0);
			SetCaret((srcX + npc->x) / 2, (srcY + npc->y) / 2, 11, 0);
			SetCaret((srcX + npc->x) / 2, (srcY + npc->y) / 2, 11, 0);
			PlaySoundObject(npc->hit_voice, 1);
			npc->shock = 16;
		}

		if (npc->bits & NPC_SHOW_DAMAGE)
			npc->damage_view -= damage;
	}
}

void HitNpCharMap()
{
	int x, y;

	int i;
	for (i = 0; i < NPC_MAX; i++)
	{
		if ((gNPC[i].cond & 0x80) == 0)
			continue;

		if (gNPC[i].bits & NPC_IGNORE_SOLIDITY)
			continue;

		gNPC[i].flag = 0;

		for (y = (gNPC[i].y - gNPC[i].hit.top + 0x1000) / 0x2000; y <= (gNPC[i].y + gNPC[i].hit.bottom + 0x1000) / 0x2000; y++)
		{
			for (x = (gNPC[i].x - gNPC[i].hit.back + 0x1000) / 0x2000; x <= (gNPC[i].x + gNPC[i].hit.back + 0x1000) / 0x2000; x++)
			{
				switch (GetAttribute(x, y))
				{
					// No NPC block
					case 0x44:
						if (gNPC[i].bits & NPC_IGNORE_TILE_44)
							break;
						// Fallthrough
					// Block
					case 0x03:
					case 0x05:
					case 0x41:
					case 0x43:
						JadgeHitNpCharBlock(&gNPC[i], x, y);
						break;

					// Slopes
					case 0x50:
						JudgeHitNpCharTriangleA(&gNPC[i], x, y);
						break;

					case 0x51:
						JudgeHitNpCharTriangleB(&gNPC[i], x, y);
						break;

					case 0x52:
						JudgeHitNpCharTriangleC(&gNPC[i], x, y);
						break;

					case 0x53:
						JudgeHitNpCharTriangleD(&gNPC[i], x, y);
						break;

					case 0x54:
						JudgeHitNpCharTriangleE(&gNPC[i], x, y);
						break;

					case 0x55:
						JudgeHitNpCharTriangleF(&gNPC[i], x, y);
						break;

					case 0x56:
						JudgeHitNpCharTriangleG(&gNPC[i], x, y);
						break;

					case 0x57:
						JudgeHitNpCharTriangleH(&gNPC[i], x, y);
						break;

					// Water
					case 0x02:
					case 0x60:
					case 0x62:
						JudgeHitNpCharWater(&gNPC[i], x, y);
						break;

					// Water block
					case 0x04:
					case 0x61:
					case 0x64:
						JadgeHitNpCharBlock(&gNPC[i], x, y);
						JudgeHitNpCharWater(&gNPC[i], x, y);
						break;

					// Water slopes
					case 0x70:
						JudgeHitNpCharTriangleA(&gNPC[i], x, y);
						JudgeHitNpCharWater(&gNPC[i], x, y);
						break;

					case 0x71:
						JudgeHitNpCharTriangleB(&gNPC[i], x, y);
						JudgeHitNpCharWater(&gNPC[i], x, y);
						break;

					case 0x72:
						JudgeHitNpCharTriangleC(&gNPC[i], x, y);
						JudgeHitNpCharWater(&gNPC[i], x, y);
						break;

					case 0x73:
						JudgeHitNpCharTriangleD(&gNPC[i], x, y);
						JudgeHitNpCharWater(&gNPC[i], x, y);
						break;

					case 0x74:
						JudgeHitNpCharTriangleE(&gNPC[i], x, y);
						JudgeHitNpCharWater(&gNPC[i], x, y);
						break;

					case 0x75:
						JudgeHitNpCharTriangleF(&gNPC[i], x, y);
						JudgeHitNpCharWater(&gNPC[i], x, y);
						break;

					case 0x76:
						JudgeHitNpCharTriangleG(&gNPC[i], x, y);
						JudgeHitNpCharWater(&gNPC[i], x, y);
						break;

					case 0x77:
						JudgeHitNpCharTriangleH(&gNPC[i], x, y);
						JudgeHitNpCharWater(&gNPC[i], x, y);
						break;

					case 0xA0:
						gNPC[i].flag |= 0x100;
						// Fallthrough
					case 0x80:
						gNPC[i].flag |= 0x1000;
						break;

					case 0xA1:
						gNPC[i].flag |= 0x100;
						// Fallthrough
					case 0x81:
						gNPC[i].flag |= 0x2000;
						break;

					case 0xA2:
						gNPC[i].flag |= 0x100;
						// Fallthrough
					case 0x82:
						gNPC[i].flag |= 0x4000;
						break;

					case 0xA3:
						gNPC[i].flag |= 0x100;
						// Fallthrough
					case 0x83:
						gNPC[i].flag |= 0x8000;
						break;
				}
			}
		}
		
		if (gNPC[i].y > gWaterY + 0x800)
			gNPC[i].flag |= 0x100;
	}
}

void LoseNpChar(NPCHAR *npc, BOOL bVanish)
{
	// Play death sound
	PlaySoundObject(npc->destroy_voice, 1);

	// Create smoke
	switch (npc->size)
	{
		case 1:
			SetDestroyNpChar(npc->x, npc->y, npc->view.back, 3);
			break;

		case 2:
			SetDestroyNpChar(npc->x, npc->y, npc->view.back, 7);
			break;

		case 3:
			SetDestroyNpChar(npc->x, npc->y, npc->view.back, 12);
			break;
	}

	// Create drop
	if (npc->exp)
	{
		int val;
		SetExpObjects(npc->x, npc->y, npc->exp);
	}

	// Set flag
	SetNPCFlag(npc->code_flag);

	// Create value view
	if (npc->bits & NPC_SHOW_DAMAGE)
	{
		if ((npc->bits & NPC_SHOW_DAMAGE) && npc->damage_view)
			SetValueView(&npc->x, &npc->y, npc->damage_view);
		if (bVanish)
			VanishNpChar(npc);
	}
	else
	{
		npc->cond = 0;
	}
}

void HitNpCharBullet()
{
	int n;
	int b;
	BOOL bHit;

	for (n = 0; n < NPC_MAX; n++)
	{
		if ((gNPC[n].cond & 0x80) == 0)
			continue;

		if (gNPC[n].bits & NPC_SHOOTABLE && gNPC[n].bits & NPC_INTERACTABLE)
			continue;

		for (b = 0; b < BULLET_MAX; b++)
		{
			if ((gBul[b].cond & 0x80) == 0)
				continue;

			if (gBul[b].damage == -1)
				continue;

			// Check if bullet touches npc
			bHit = FALSE;
			if (gNPC[n].bits & NPC_SHOOTABLE
				&& gNPC[n].x - gNPC[n].hit.back < gBul[b].x + gBul[b].enemyXL
				&& gNPC[n].x + gNPC[n].hit.back > gBul[b].x - gBul[b].enemyXL
				&& gNPC[n].y - gNPC[n].hit.top < gBul[b].y + gBul[b].enemyYL
				&& gNPC[n].y + gNPC[n].hit.bottom > gBul[b].y - gBul[b].enemyYL)
				bHit = TRUE;
			else if (gNPC[n].bits & NPC_INVULNERABLE
				&& gNPC[n].x - gNPC[n].hit.back < gBul[b].x + gBul[b].blockXL
				&& gNPC[n].x + gNPC[n].hit.back > gBul[b].x - gBul[b].blockXL
				&& gNPC[n].y - gNPC[n].hit.top < gBul[b].y + gBul[b].blockYL
				&& gNPC[n].y + gNPC[n].hit.bottom > gBul[b].y - gBul[b].blockYL)
				bHit = TRUE;

			if (bHit)
			{
				// Damage NPC
				if (gNPC[n].bits & NPC_SHOOTABLE)
				{
					gNPC[n].life -= gBul[b].damage;

					if (gNPC[n].life < 1)
					{
						gNPC[n].life = 0;

						if (gNPC[n].bits & NPC_SHOW_DAMAGE)
							gNPC[n].damage_view -= gBul[b].damage;

						if ((gMC.cond & 0x80) && gNPC[n].bits & NPC_EVENT_WHEN_KILLED)
							StartTextScript(gNPC[n].code_event);
						else
							gNPC[n].cond |= 8;
					}
					else
					{
						if (gNPC[n].shock < 14)
						{
							SetCaret((gBul[b].x + gNPC[n].x) / 2, (gBul[b].y + gNPC[n].y) / 2, 11, 0);
							SetCaret((gBul[b].x + gNPC[n].x) / 2, (gBul[b].y + gNPC[n].y) / 2, 11, 0);
							SetCaret((gBul[b].x + gNPC[n].x) / 2, (gBul[b].y + gNPC[n].y) / 2, 11, 0);
							PlaySoundObject(gNPC[n].hit_voice, 1);
							gNPC[n].shock = 16;
						}

						if (gNPC[n].bits & NPC_SHOW_DAMAGE)
							gNPC[n].damage_view -= gBul[b].damage;
					}
				}
				else if (gBul[b].code_bullet == 13
					|| gBul[b].code_bullet == 14
					|| gBul[b].code_bullet == 15
					|| gBul[b].code_bullet == 28
					|| gBul[b].code_bullet == 29
					|| gBul[b].code_bullet == 30)
				{
					// Strange empty case that's needed for accurate assembly
				}
				else if ((gBul[b].bbits & 0x10) == 0)
				{
					// Hit invulnerable NPC
					SetCaret((gBul[b].x + gNPC[n].x) / 2, (gBul[b].y + gNPC[n].y) / 2, 2, 2);
					PlaySoundObject(31, 1);
					gBul[b].life = 0;
					continue;
				}

				--gBul[b].life;
			}
		}

		if (gNPC[n].cond & 8)
			LoseNpChar(&gNPC[n], TRUE);
	}
}

