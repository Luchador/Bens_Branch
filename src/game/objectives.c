#include <ultra64.h>
#include "constants.h"
#include "game/chraction.h"
#include "game/debug.h"
#include "game/prop.h"
#include "game/setuputils.h"
#include "game/objectives.h"
#include "game/tex.h"
#include "game/camera.h"
#include "game/hudmsg.h"
#include "game/inv.h"
#include "game/playermgr.h"
#include "game/lv.h"
#include "game/training.h"
#include "game/lang.h"
#include "game/propobj.h"
#include "bss.h"
#include "lib/dma.h"
#include "lib/memp.h"
#include "lib/rng.h"
#include "string.h"
#include "lib/mtx.h"
#include "data.h"
#include "types.h"
#include "platform.h"

struct objective *g_Objectives[MAX_OBJECTIVES];
uint32_t g_ObjectiveStatuses[MAX_OBJECTIVES];
struct tag *g_TagsLinkedList;
struct briefingobj *g_BriefingObjs;
struct criteria_roomentered *g_RoomEnteredCriterias;
struct criteria_throwinroom *g_ThrowInRoomCriterias;
struct criteria_holograph *g_HolographCriterias;
int g_NumTags;
struct tag **g_TagPtrs;

int g_ObjectiveLastIndex = -1;
bool g_ObjectiveChecksDisabled = false;

void tagsReset(void)
{
	int index = 0;
	struct tag *tag = g_TagsLinkedList;

	while (tag) {
		if (tag->tagnum >= index) {
			index = tag->tagnum + 1;
		}

		tag = tag->next;
	}

	g_NumTags = index;

	if (g_NumTags) {
		uint32_t size = index * sizeof(uintptr_t);
		g_TagPtrs = mempAlloc(ALIGN16(size), MEMPOOL_STAGE);

		for (index = 0; index < g_NumTags; index++) {
			g_TagPtrs[index] = NULL;
		}
	}

	tag = g_TagsLinkedList;

	while (tag) {
		g_TagPtrs[tag->tagnum] = tag;
		tag = tag->next;
	}
}

struct tag *tagFindById(int tag_id)
{
	struct tag *tag = NULL;

	if (tag_id >= 0 && tag_id < g_NumTags) {
		tag = g_TagPtrs[tag_id];
	}

	return tag;
}

int objGetTagNum(struct defaultobj *obj)
{
	struct tag *tag = g_TagsLinkedList;

	if (obj && (obj->hidden & OBJHFLAG_TAGGED)) {
		while (tag) {
			if (obj == tag->obj) {
				return tag->tagnum;
			}

			tag = tag->next;
		}
	}

	return -1;
}

struct defaultobj *objFindByTagId(int tag_id)
{
	struct tag *tag = tagFindById(tag_id);
	struct defaultobj *obj = NULL;

	if (tag) {
		obj = tag->obj;
	}

	if (obj && (obj->hidden & OBJHFLAG_TAGGED) == 0) {
		obj = NULL;
	}

	return obj;
}

int objectiveGetCount(void)
{
	return g_ObjectiveLastIndex + 1;
}

char *objectiveGetText(int index)
{
	if (index < 10 && g_Objectives[index]) {
		return langGet(g_Objectives[index]->text);
	}

	return NULL;
}

uint32_t objectiveGetDifficultyBits(int index)
{
	if (index < 10 && g_Objectives[index]) {
		return g_Objectives[index]->difficulties;
	}

	return DIFFBIT_A | DIFFBIT_SA | DIFFBIT_PA | DIFFBIT_PD;
}

/**
 * Check if an objective is complete.
 *
 * It starts be setting the objective's status to complete, then iterates each
 * requirement in the objective to decide whether to change it to incomplete or
 * failed.
 */
int objectiveCheck(int index)
{
	int objstatus = OBJECTIVE_COMPLETE;

	if (index < ARRAYCOUNT(g_Objectives)) {
		if (g_Objectives[index] == NULL) {
			objstatus = g_ObjectiveStatuses[index];
		} else {
			// Note: This is setting the cmd pointer to the start of the
			// beginobjective macro in the stage's setup file. The first
			// iteration of the while loop below will skip past it.
			uint32_t *cmd = (uint32_t *)g_Objectives[index];

			while ((uint8_t)PD_BE32(cmd[0]) != OBJTYPE_ENDOBJECTIVE) {
				// The status of this requirement
				int reqstatus = OBJECTIVE_COMPLETE;

				switch ((uint8_t)PD_BE32(cmd[0])) {
				case OBJECTIVETYPE_DESTROYOBJ:
					{
						struct defaultobj *obj = objFindByTagId(cmd[1]);
						if (obj && obj->prop && objIsHealthy(obj)) {
							reqstatus = OBJECTIVE_INCOMPLETE;
						}
					}
					break;
				case OBJECTIVETYPE_COMPFLAGS:
					if (!chrHasStageFlag(NULL, cmd[1])) {
						reqstatus = OBJECTIVE_INCOMPLETE;
					}
					break;
				case OBJECTIVETYPE_FAILFLAGS:
					if (chrHasStageFlag(NULL, cmd[1])) {
						reqstatus = OBJECTIVE_FAILED;
					}
					break;
				case OBJECTIVETYPE_COLLECTOBJ:
					{
						struct defaultobj *obj = objFindByTagId(cmd[1]);
						int prevplayernum;
						int collected = false;
						int i;

						if (!obj || !obj->prop || !objIsHealthy(obj)) {
							reqstatus = OBJECTIVE_FAILED;
						} else {
							prevplayernum = g_Vars.currentplayernum;

							for (i = 0; i < PLAYERCOUNT(); i++) {
								if (g_Vars.players[i] == g_Vars.bond || g_Vars.players[i] == g_Vars.coop) {
									setCurrentPlayerNum(i);

									if (invHasProp(obj->prop)) {
										collected = true;
										break;
									}
								}
							}

							setCurrentPlayerNum(prevplayernum);

							if (!collected) {
								reqstatus = OBJECTIVE_INCOMPLETE;
							}
						}
					}
					break;
				case OBJECTIVETYPE_THROWOBJ:
					{
						struct defaultobj *obj = objFindByTagId(cmd[1]);

						if (obj && obj->prop) {
							int i;
							int prevplayernum = g_Vars.currentplayernum;

							for (i = 0; i < PLAYERCOUNT(); i++) {
								if (g_Vars.players[i] == g_Vars.bond || g_Vars.players[i] == g_Vars.coop) {
									setCurrentPlayerNum(i);

									if (invHasProp(obj->prop)) {
										reqstatus = OBJECTIVE_INCOMPLETE;
										break;
									}
								}
							}

							setCurrentPlayerNum(prevplayernum);
						}
					}
					break;
				case OBJECTIVETYPE_HOLOGRAPH:
					{
						struct defaultobj *obj = objFindByTagId(cmd[1]);

						if (cmd[2] == 0) {
							if (!obj || !obj->prop || !objIsHealthy(obj)) {
								reqstatus = OBJECTIVE_FAILED;
							} else {
								reqstatus = OBJECTIVE_INCOMPLETE;
							}
						}
					}
					break;
				case OBJECTIVETYPE_ENTERROOM:
					if (cmd[2] == 0) {
						reqstatus = OBJECTIVE_INCOMPLETE;
					}
					break;
				case OBJECTIVETYPE_THROWINROOM:
					if (cmd[3] == 0) {
						reqstatus = OBJECTIVE_INCOMPLETE;
					}
					break;
				case OBJTYPE_BEGINOBJECTIVE:
				case OBJTYPE_ENDOBJECTIVE:
					break;
				}

				if (objstatus == OBJECTIVE_COMPLETE) {
					if (reqstatus != OBJECTIVE_COMPLETE) {
						// This is the first requirement that is causing the
						// objective to not be complete, so apply it.
						objstatus = reqstatus;
					}
				} else if (objstatus == OBJECTIVE_INCOMPLETE) {
					if (reqstatus == OBJECTIVE_FAILED) {
						// An earlier requirement was incomplete,
						// and this requirement is failed.
						objstatus = reqstatus;
					}
				}

				cmd = cmd + setupGetCmdLength(cmd);
			}
		}
	}

	return objstatus;
}

bool objectiveIsAllComplete(void)
{
	int i;

	for (i = 0; i < objectiveGetCount(); i++) {
		uint32_t diffbits = objectiveGetDifficultyBits(i);

		if ((1 << lvGetDifficulty() & diffbits) &&
				objectiveCheck(i) != OBJECTIVE_COMPLETE) {
			return false;
		}
	}

	return true;
}

void objectivesDisableChecking(void)
{
	g_ObjectiveChecksDisabled = true;
}

void objectivesShowHudmsg(char *buffer, int hudmsgtype)
{
	int prevplayernum = g_Vars.currentplayernum;
	int i;

	for (i = 0; i < PLAYERCOUNT(); i++) {
		setCurrentPlayerNum(i);

		if (g_Vars.currentplayer == g_Vars.bond || g_Vars.currentplayer == g_Vars.coop) {
			hudmsgCreateWithFlags(buffer, hudmsgtype, HUDMSGFLAG_DELAY | HUDMSGFLAG_ALLOWDUPES);
		}
	}

	setCurrentPlayerNum(prevplayernum);
}

void objectivesCheckAll(void)
{
	int availableindex = 0;
	int i;
	char buffer[50] = "";

	if (!g_ObjectiveChecksDisabled) {
		for (i = 0; i <= g_ObjectiveLastIndex; i++) {
			int status = objectiveCheck(i);

			if (g_ObjectiveStatuses[i] != status) {
				g_ObjectiveStatuses[i] = status;

				if (objectiveGetDifficultyBits(i) & (1 << lvGetDifficulty())) {
					sprintf(buffer, "%s %d: ", langRemoveNewline(langGet(L_MISC_044)), availableindex + 1); // "Objective"

					// NTSC 1.0 and above shows objective messages to everyone,
					// while beta only shows them to the current player.
					if (status == OBJECTIVE_COMPLETE) {
						strcat(buffer, langGet(L_MISC_045)); // "Completed"
						objectivesShowHudmsg(buffer, HUDMSGTYPE_OBJECTIVECOMPLETE);
					} else if (status == OBJECTIVE_INCOMPLETE) {
						strcat(buffer, langGet(L_MISC_046)); // "Incomplete"
						objectivesShowHudmsg(buffer, HUDMSGTYPE_OBJECTIVECOMPLETE);
					} else if (status == OBJECTIVE_FAILED) {
						strcat(buffer, langGet(L_MISC_047)); // "Failed"
						objectivesShowHudmsg(buffer, HUDMSGTYPE_OBJECTIVEFAILED);
					}
				}
			}

			if (objectiveGetDifficultyBits(i) & (1 << lvGetDifficulty())) {
				availableindex++;
			}
		}
	}
}

void objectiveCheckRoomEntered(int currentroom)
{
	struct criteria_roomentered *criteria = g_RoomEnteredCriterias;

	while (criteria) {
		if (criteria->status == OBJECTIVE_INCOMPLETE) {
			int room = chrGetPadRoom(NULL, criteria->pad);

			if (room >= 0 && room == currentroom) {
				criteria->status = OBJECTIVE_COMPLETE;
			}
		}

		criteria = criteria->next;
	}
}

void objectiveCheckThrowInRoom(int arg0, RoomNum *inrooms)
{
	struct criteria_throwinroom *criteria = g_ThrowInRoomCriterias;

	while (criteria) {
		if (criteria->status == OBJECTIVE_INCOMPLETE && criteria->unk04 == arg0) {
			int room = chrGetPadRoom(NULL, criteria->pad);

			if (room >= 0) {
				RoomNum requirerooms[2];
				requirerooms[0] = room;
				requirerooms[1] = -1;

				if (arrayIntersects(requirerooms, inrooms)) {
					criteria->status = OBJECTIVE_COMPLETE;
				}
			}
		}

		criteria = criteria->next;
	}
}

void objectiveCheckHolograph(float maxdist)
{
	struct criteria_holograph *criteria = g_HolographCriterias;

	while (criteria) {
		if (g_Vars.stagenum == STAGE_CITRAINING) {
			criteria->status = OBJECTIVE_INCOMPLETE;
		}

		if (criteria->status == OBJECTIVE_INCOMPLETE) {
			struct defaultobj *obj = objFindByTagId(criteria->obj);

			if (obj && obj->prop
					&& (obj->prop->flags & PROPFLAG_ONTHISSCREENTHISTICK)
					&& obj->prop->z >= 0
					&& objIsHealthy(obj)) {
				struct coord sp9c;
				float sp94[2];
				float sp8c[2];
				float dist = -1;

				if (maxdist != 0.0f) {
					float xdiff = obj->prop->pos.x - g_Vars.currentplayer->cam_pos.x;
					float zdiff = obj->prop->pos.z - g_Vars.currentplayer->cam_pos.z;
					dist = xdiff * xdiff + zdiff * zdiff;
					maxdist = maxdist * maxdist;
				}

				if (dist < maxdist && func0f0899dc(obj->prop, &sp9c, sp94, sp8c)) {
					float sp78[2];
					float sp70[2];
					func0f06803c(&sp9c, sp94, sp8c, sp78, sp70);

					if (sp78[0] > camGetScreenLeft()
							&& sp78[0] < camGetScreenLeft() + camGetScreenWidth()
							&& sp70[0] > camGetScreenLeft()
							&& sp70[0] < camGetScreenLeft() + camGetScreenWidth()
							&& sp78[1] > camGetScreenTop()
							&& sp78[1] < camGetScreenTop() + camGetScreenHeight()
							&& sp70[1] > camGetScreenTop()
							&& sp70[1] < camGetScreenTop() + camGetScreenHeight()) {
						criteria->status = OBJECTIVE_COMPLETE;

						if (g_Vars.stagenum == STAGE_CITRAINING) {
							struct trainingdata *data = dtGetData();
							data->holographedpc = true;
						}
					}
				}
			}
		}

		criteria = criteria->next;
	}
}
