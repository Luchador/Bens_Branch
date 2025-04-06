#include <ultra64.h>
#include <math.h>
#include "constants.h"
#include "game/race.h"
#include "bss.h"
#include "lib/anim.h"
#include "data.h"
#include "types.h"

uint16_t raceGetAnimSumAngleAsInt(int16_t animnum, int frame, int endframe)
{
	int16_t inttranslate[3];
	uint16_t sumangle = 0;

	while (frame < endframe) {
		sumangle += animGetPosAngleAsInt(0, false, &g_SkelChr, animnum, frame, inttranslate, false);
		frame++;
	}

	return sumangle;
}

int raceGetAnimSumForwardAsInt(int16_t animnum, int frame, int endframe)
{
	int sumforward = 0;
	int16_t inttranslate[3];

	while (frame < endframe) {
		animGetPosAngleAsInt(0, false, &g_SkelChr, animnum, frame, inttranslate, false);
		sumforward += inttranslate[2];
		frame++;
	}

	return sumforward;
}

int raceInitAnimGroup(struct attackanimconfig *configs)
{
	int count = 0;
	struct attackanimconfig *config = configs;

	while (config->animnum != 0) {
		uint16_t angle = raceGetAnimSumAngleAsInt(config->animnum, 0, (int)floorf(config->unk04));

		if (config->unk04 > 0) {
			if (angle < 0x8000) {
				config->unk08 = angle * 0.00009585853695171f / config->unk04;
			} else {
				config->unk08 = (angle * 0.00009585853695171f - M_TAU) / config->unk04;
			}
		} else {
			config->unk08 = 0;
		}

		config++;
		count++;
	}

	return count;
}

void raceInitAnimGroups(struct attackanimgroup **groups)
{
	int i;

	for (i = 0; i < 32; i++) {
		if (groups[i]->len < 0) {
			groups[i]->len = raceInitAnimGroup(groups[i]->animcfg);
		}
	}
}

/**
 * Calculate and return the average forward movement speed for an animation,
 * normalized to world units (likely meters per frame). Also stores the raw
 * average forward delta in a lookup table for later use.
 */
int raceCountAnims(struct animtablerow *rows)
{
	int i;

	for (i = 0; rows[i].animnum > 0; i++);

	return i;
}

float raceCalculateAnimSpeed(int16_t animnum)
{
	float avgforward = raceGetAnimSumForwardAsInt(animnum, 0, animGetNumFrames(animnum) - 1) / (float) animGetNumFrames(animnum);

	g_AnimAvgForwardPerFrame[animnum] = avgforward;

	return avgforward * 0.1000000089407f;
}

void raceInitAnims(void)
{
	int race;
	int i;

	for (race = 0; race < ARRAYCOUNT(g_AnimTablesByRace); race++) {
		for (i = 0; g_AnimTablesByRace[race][i].hitpart != -1; i++) {
			if (g_AnimTablesByRace[race][i].deathanims) {
				g_AnimTablesByRace[race][i].deathanimcount = raceCountAnims(g_AnimTablesByRace[race][i].deathanims);
			} else {
				g_AnimTablesByRace[race][i].deathanimcount = 0;
			}

			if (g_AnimTablesByRace[race][i].injuryanims) {
				g_AnimTablesByRace[race][i].injuryanimcount = raceCountAnims(g_AnimTablesByRace[race][i].injuryanims);
			} else {
				g_AnimTablesByRace[race][i].injuryanimcount = 0;
			}
		}

		for (i = 0; g_MoveAnims[race][i].animnum >= 0; i++) {
			g_MoveAnims[race][i].value = raceCalculateAnimSpeed(g_MoveAnims[race][i].animnum);
		}
	}

	raceCountAnims(g_AnimTableHumanSlumped);

	for (race = 0; race < 2; race++) {
		raceInitAnimGroups(g_StandHeavyAttackAnims[race]);
		raceInitAnimGroups(g_StandLightAttackAnims[race]);
		raceInitAnimGroups(g_StandDualAttackAnims[race]);
		raceInitAnimGroups(g_KneelHeavyAttackAnims[race]);
		raceInitAnimGroups(g_KneelLightAttackAnims[race]);
		raceInitAnimGroups(g_KneelDualAttackAnims[race]);
	}

	raceInitAnimGroup(g_RollAttackAnims);
	raceInitAnimGroup(g_WalkAttackAnims);
}
