#include <ultra64.h>
#include <math.h>
#include "constants.h"
#include "game/title.h"
#include "game/bondgun.h"
#include "game/modelmgr.h"
#include "game/tex.h"
#include "game/inv.h"
#include "game/playermgr.h"
#include "game/mtxutils.h"
#include "game/menuutils.h"
#include "game/gfxmemory.h"
#include "game/credits.h"
#include "game/bondview.h"
#include "game/textutils.h"
#include "game/file.h"
#include "game/lv.h"
#include "game/music.h"
#include "game/training.h"
#include "game/modeldef.h"
#include "game/lang.h"
#include "game/propobj.h"
#include "game/savebuffer.h"
#include "game/utils.h"
#include "bss.h"
#include "lib/joy.h"
#include "lib/vi.h"
#include "lib/main.h"
#include "lib/model.h"
#include "lib/snd.h"
#include "string.h"
#include "lib/lib_317f0.h"
#include "data.h"
#include "types.h"
#include "string.h"
#include "video.h"

#define TITLE_ASPECT (videoGetAspect())

int8_t *g_TitleModelBuffer;
Vtx *g_PdLogoVertices[NUM_FRAMEBUFFERS];
Col *g_PdLogoColours[NUM_FRAMEBUFFERS];
int g_PdLogoVtxColIndex;


int g_TitleViewHeight = 480;
bool g_IsTitleDemo = false;
bool g_TitleButtonPressed = false;
bool g_TitleFastForward = false;
unsigned int g_TitleIdleTime60 = 0;
int g_TitleMode = -1;
int g_TitleNextMode = -1;
unsigned int g_TitleDelayedTimer = 2;
int g_TitleDelayedMode = -1;
int g_TitleTimer = 0;
int g_TitleNextStage = -1; // appears to be used for more than just title
struct model *g_TitleModel = NULL;
struct model *g_TitleModelNLogo2 = NULL;
struct model *g_TitleModelPdTwo = NULL;
struct model *g_TitleModelPdThree = NULL;

Lights1 g_TitleLightPdLogoFront = gdSPDefLights1(0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
Lights1 g_TitleLightPdLogoNotFront = gdSPDefLights1(0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
Lights1 g_TitleLightPdLogoMain = gdSPDefLights1(0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x7f);
Lights1 g_TitleLightNintendoRare = gdSPDefLights1(0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
Lights1 g_TitleLightRareLogo = gdSPDefLights1(0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x7f);

void titleSetLight(Lights1 *light, int8_t r, int8_t g, int8_t b, float luminosity, struct coord *dir)
{
	light->a.l.col[0] = r * luminosity;
	light->a.l.col[1] = g * luminosity;
	light->a.l.col[2] = b * luminosity;

	light->a.l.colc[0] = r * luminosity;
	light->a.l.colc[1] = g * luminosity;
	light->a.l.colc[2] = b * luminosity;

	light->l[0].l.col[0] = r;
	light->l[0].l.col[1] = g;
	light->l[0].l.col[2] = b;

	light->l[0].l.colc[0] = r;
	light->l[0].l.colc[1] = g;
	light->l[0].l.colc[2] = b;

	light->l[0].l.dir[0] = dir->x * 127.0f;
	light->l[0].l.dir[1] = dir->y * 127.0f;
	light->l[0].l.dir[2] = dir->z * 127.0f;
}

void titleInitLegal(void)
{
	musicQueueStopAllEvent();
	g_TitleTimer = 0;
	g_TitleButtonPressed = false;
	g_TitleFastForward = false;
}

void titleTickLegal(void)
{
	viSetFovY(60);
	viSetAspect(videoGetAspect());
	viSetZRange(100, 10000);
	viSetUseZBuf(false);

	g_TitleTimer += g_Vars.lvupdate60;

	if (g_TitleTimer > TICKS(180)) {
		titleSetNextMode(TITLEMODE_RARELOGO);
	}
}

bool g_LegalEnabled;

#define LEGALELEMENTTYPE_BLUETEXTSM  0
#define LEGALELEMENTTYPE_BLUETEXTMD  1
#define LEGALELEMENTTYPE_BLUETEXTLG  2
#define LEGALELEMENTTYPE_LINE        3
#define LEGALELEMENTTYPE_DOLBYLOGO   4
#define LEGALELEMENTTYPE_WHITETEXTLG 5
#define LEGALELEMENTTYPE_WHITETEXTSM 6
#define LEGALELEMENTTYPE_RARELOGO    7

struct legalelement {
	int16_t x;
	int16_t y;
	int16_t type;
	uint16_t textid;
	const char *textptr;
};

struct legalelement g_LegalElements[] = {
	{ 49,  179, LEGALELEMENTTYPE_BLUETEXTLG,  L_OPTIONS_077 }, // "Nintendo 64 Product Identification"
	{ 49,  200, LEGALELEMENTTYPE_LINE,        0             },
	{ 69,  207, LEGALELEMENTTYPE_BLUETEXTLG,  L_OPTIONS_078 }, // "Product ID:"
	{ 69,  227, LEGALELEMENTTYPE_BLUETEXTLG,  L_OPTIONS_079 }, // "Product Code:"
	{ 69,  247, LEGALELEMENTTYPE_BLUETEXTLG,  L_OPTIONS_080 }, // "Variant:"
	{ 69,  267, LEGALELEMENTTYPE_BLUETEXTLG,  L_OPTIONS_081 }, // "Developer:"
	{ 249, 207, LEGALELEMENTTYPE_BLUETEXTLG,  L_OPTIONS_082 }, // "Perfect Dark"
	{ 249, 227, LEGALELEMENTTYPE_BLUETEXTLG,  L_OPTIONS_083 }, // "NUS-NPDE-USA"
	{ 249, 247, LEGALELEMENTTYPE_BLUETEXTLG,  L_OPTIONS_084 }, // "NTSC version 8.7 final"
	{ 249, 267, LEGALELEMENTTYPE_BLUETEXTLG,  L_OPTIONS_085 }, // "Rare Ltd. (twycross)"
	{ 69,  290, LEGALELEMENTTYPE_LINE,        0             },
	{ 69,  299, LEGALELEMENTTYPE_WHITETEXTLG, L_OPTIONS_076 }, // "N64 EXPANSION PAK"
	{ 266, 296, LEGALELEMENTTYPE_WHITETEXTSM, L_OPTIONS_075 }, // "tm"
	{ 286, 299, LEGALELEMENTTYPE_WHITETEXTLG, L_OPTIONS_074 }, // "NOT DETECTED"
	{ 69,  320, LEGALELEMENTTYPE_LINE,        0             },
	{ 69,  328, LEGALELEMENTTYPE_BLUETEXTMD,  L_OPTIONS_087 }, // "The Rarewere Logo and Perfect Dark are ..."
	{ 138, 343, LEGALELEMENTTYPE_BLUETEXTMD,  L_OPTIONS_088 }, // "Presented in Dolby Surround. Dolby and ..."
	{ 69,  372, LEGALELEMENTTYPE_BLUETEXTMD,  L_OPTIONS_089 }, // "Uses Miles Sound System ..."
	{ 69,  428, LEGALELEMENTTYPE_LINE,        0             },
	{ 69,  433, LEGALELEMENTTYPE_BLUETEXTSM,  L_OPTIONS_093 }, // "rare designs on the future <<<"
	{ 69,  344, LEGALELEMENTTYPE_DOLBYLOGO,   0             },
};

Gfx *titleRenderLegal(Gfx *gdl)
{
	struct legalelement *elem;
	struct legalelement *end;
	struct modelrenderdata renderdata = { NULL, true, 3 };
	int x;
	int y;
	struct fontchar *font1;
	struct font *font2;

	if (g_LegalEnabled) {
		gdl = titleClear(gdl);
		gdl = textConfigureGfxPipeline(gdl);

		gSPSetExtraGeometryModeEXT(gdl++, G_ASPECT_CENTER_EXT);

		elem = g_LegalElements;
		end = &g_LegalElements[ARRAYCOUNT(g_LegalElements)];

		for (; elem < end; elem++) {
			uint32_t colour = 0x7f7fffff;

			switch (elem->type) {
			case LEGALELEMENTTYPE_BLUETEXTSM:
				font1 = g_CharsHandelGothicSm;
				font2 = g_FontHandelGothicSm;
				break;
			case LEGALELEMENTTYPE_BLUETEXTMD:
				font1 = g_CharsHandelGothicMd;
				font2 = g_FontHandelGothicMd;
				break;
			case LEGALELEMENTTYPE_BLUETEXTLG:
				font1 = g_CharsHandelGothicLg;
				font2 = g_FontHandelGothicLg;
				break;
			case LEGALELEMENTTYPE_WHITETEXTLG:
				font1 = g_CharsHandelGothicLg;
				font2 = g_FontHandelGothicLg;
				colour = 0xffffffff;

				if (elem->textid == L_OPTIONS_074 || elem->textid == L_OPTIONS_073) {
					elem->textid = L_OPTIONS_073; // "Detected"
				}
				break;
			case LEGALELEMENTTYPE_WHITETEXTSM:
				font1 = g_CharsHandelGothicSm;
				font2 = g_FontHandelGothicSm;
				colour = 0xffffffff;
				break;
			default:
				font1 = g_CharsHandelGothicLg;
				font2 = g_FontHandelGothicLg;
				break;
			}

			if (elem->type == LEGALELEMENTTYPE_LINE) {
				gdl = text0f153780(gdl);
				gdl = text0f153a34(gdl, elem->x, elem->y, viGetWidth(), elem->y + 2, 0x7f7fff7f);
				gdl = textConfigureGfxPipeline(gdl);
			} else if (elem->type == LEGALELEMENTTYPE_DOLBYLOGO) {
				gdl = text0f153780(gdl);

				gDPSetTexturePersp(gdl++, G_TP_NONE);
				gDPSetAlphaCompare(gdl++, G_AC_NONE);
				gDPSetTextureLOD(gdl++, G_TL_TILE);
				gDPSetTextureConvert(gdl++, G_TC_FILT);

				texSelect(&gdl, &g_TexGeneralConfigs[47], 1, 0, 2, 1, 0);

				gDPSetCycleType(gdl++, G_CYC_1CYCLE);
				gDPSetCombineMode(gdl++, G_CC_DECALRGBA, G_CC_DECALRGBA);
				gDPSetTextureFilter(gdl++, G_TF_POINT);

				gSPTextureRectangle(gdl++,
						elem->x << 2,
						elem->y << 2,
						(elem->x + 64) << 2,
						(elem->y + 24) << 2,
						G_TX_RENDERTILE, 0, 0x0300, 0x0400, -0x0400);

				gdl = textConfigureGfxPipeline(gdl);
			} else if (elem->type == LEGALELEMENTTYPE_RARELOGO) {
				gdl = text0f153780(gdl);

				gDPSetTexturePersp(gdl++, G_TP_NONE);
				gDPSetAlphaCompare(gdl++, G_AC_NONE);
				gDPSetTextureLOD(gdl++, G_TL_TILE);
				gDPSetTextureConvert(gdl++, G_TC_FILT);

				texSelect(&gdl, &g_TexGeneralConfigs[49], 1, 0, 2, 1, 0);

				gDPSetCycleType(gdl++, G_CYC_1CYCLE);
				gDPSetCombineMode(gdl++, G_CC_DECALRGBA, G_CC_DECALRGBA);
				gDPSetTextureFilter(gdl++, G_TF_POINT);

				gSPTextureRectangle(gdl++,
						elem->x << 2,
						elem->y << 2,
						(elem->x + 32) << 2,
						(elem->y + 42) << 2,
						G_TX_RENDERTILE, 0, 0x0540, 0x0400, -0x0400);

				gdl = textConfigureGfxPipeline(gdl);
			} else {
#define ELEM_TEXT (char *)(elem->textptr ? elem->textptr : langGet(elem->textid))
				x = elem->x;
				y = elem->y;
				gdl = textRenderProjected(gdl, &x, &y, ELEM_TEXT, font1, font2, colour, viGetWidth(), viGetHeight(), 0, 0);
			}
		}

		gSPClearExtraGeometryModeEXT(gdl++, G_ASPECT_MODE_EXT);

		gdl = text0f153780(gdl);
	}

	return gdl;
}

bool g_LegalEnabled = true;
bool g_PdLogoIsFirstTick = true;
bool g_PdLogoTriggerExit = false;

void titleInitPdLogo(void)
{
	uint8_t *nextaddr = g_TitleModelBuffer;
	unsigned int remaining;
	unsigned int size;

	g_TitleTimer = 0;

	{
		struct coord coord = {0, 0, 0};
		g_ModelStates[MODEL_NLOGO].modeldef = modeldefLoad(g_ModelStates[MODEL_NLOGO].fileid, nextaddr, TITLE_ALLOCSIZE, 0);
		size = ALIGN64(fileGetLoadedSize(g_ModelStates[MODEL_NLOGO].fileid));
		nextaddr += size;
		remaining = TITLE_ALLOCSIZE - size;
		modelAllocateRwData(g_ModelStates[MODEL_NLOGO].modeldef);

		g_TitleModel = modelmgrInstantiateModelWithAnim(g_ModelStates[MODEL_NLOGO].modeldef);
		modelSetScale(g_TitleModel, 1);
		modelSetRootPosition(g_TitleModel, &coord);
	}

	{
		struct coord coord = {0, 0, 0};
		g_ModelStates[MODEL_NLOGO2].modeldef = modeldefLoad(g_ModelStates[MODEL_NLOGO2].fileid, nextaddr, remaining, 0);
		size = ALIGN64(fileGetLoadedSize(g_ModelStates[MODEL_NLOGO2].fileid));
		nextaddr += size;
		remaining -= size;
		modelAllocateRwData(g_ModelStates[MODEL_NLOGO2].modeldef);

		g_TitleModelNLogo2 = modelmgrInstantiateModelWithAnim(g_ModelStates[MODEL_NLOGO2].modeldef);
		modelSetScale(g_TitleModelNLogo2, 1);
		modelSetRootPosition(g_TitleModelNLogo2, &coord);
	}

	{
		struct coord coord = {0, 0, 0};
		g_ModelStates[MODEL_PDTWO].modeldef = modeldefLoad(g_ModelStates[MODEL_PDTWO].fileid, nextaddr, remaining, 0);
		size = ALIGN64(fileGetLoadedSize(g_ModelStates[MODEL_PDTWO].fileid));
		nextaddr += size;
		remaining -= size;
		modelAllocateRwData(g_ModelStates[MODEL_PDTWO].modeldef);

		g_TitleModelPdTwo = modelmgrInstantiateModelWithoutAnim(g_ModelStates[MODEL_PDTWO].modeldef);
		modelSetScale(g_TitleModelPdTwo, 1);
		modelSetRootPosition(g_TitleModelPdTwo, &coord);
	}

	{
		struct coord coord = {0, 0, 0};
		g_ModelStates[MODEL_PDTHREE].modeldef = modeldefLoad(g_ModelStates[MODEL_PDTHREE].fileid, nextaddr, remaining, 0);
		size = ALIGN64(fileGetLoadedSize(g_ModelStates[MODEL_PDTHREE].fileid));
		nextaddr += size;
		remaining -= size;
		modelAllocateRwData(g_ModelStates[MODEL_PDTHREE].modeldef);

		g_TitleModelPdThree = modelmgrInstantiateModelWithoutAnim(g_ModelStates[MODEL_PDTHREE].modeldef);
		modelSetScale(g_TitleModelPdThree, 1);
		modelSetRootPosition(g_TitleModelPdThree, &coord);
	}

	{
		struct modelrodata_dl *rodata = (struct modelrodata_dl *)modelGetPartRodata(g_ModelStates[MODEL_PDTWO].modeldef, MODELPART_LOGO_FRONTSIDE);

		size = ALIGN8(rodata->numvertices * sizeof(Vtx));

		g_PdLogoVertices[0] = (void *)nextaddr;

		nextaddr += size;
		remaining -= size;
		g_PdLogoVertices[1] = (void *)nextaddr;

		nextaddr += size;
		remaining -= size;
		size = ALIGN8(rodata->numcolours * sizeof(Col));
		g_PdLogoColours[0] = (void *)nextaddr;

		nextaddr += size;
		remaining -= size;
		g_PdLogoColours[1] = (void *)nextaddr;

		g_PdLogoVtxColIndex = 0;

		joySetAllowTitleInput(false);

		g_PdLogoIsFirstTick = true;
		g_PdLogoTriggerExit = false;

		if (g_TitleButtonPressed) {
			titleSkipToPdTitle();
		}
	}
}

void titleExitPdLogo(void)
{
	modelmgrFreeModel(g_TitleModel);
	modelmgrFreeModel(g_TitleModelNLogo2);
	modelmgrFreeModel(g_TitleModelPdTwo);
	modelmgrFreeModel(g_TitleModelPdThree);

	joySetAllowTitleInput(true);
}

void titleTickPdLogo(void)
{
	viSetFovY(46);
	viSetAspect(videoGetAspect());
	viSetZRange(100, 10000);
	viSetUseZBuf(false);

	g_TitleTimer += g_Vars.lvupdate60;
	g_PdLogoVtxColIndex = 1 - g_PdLogoVtxColIndex;

	if (g_PdLogoTriggerExit) {
		// Exiting due to player not pressing anything
		//if (g_AltTitleEnabled) {
		if(true) {
			g_TitleMode = TITLEMODE_SKIP;
			creditsRequestAltTitle();
			g_TitleNextStage = STAGE_CREDITS; // for alt title screen
			setNumPlayers(1);
			mainChangeToStage(g_TitleNextStage);

			g_Vars.bondplayernum = 0;
			g_Vars.coopplayernum = -1;
			g_Vars.antiplayernum = -1;

			lvSetDifficulty(DIFF_A);
		} else {
			titleSetNextMode(TITLEMODE_SKIP);
		}
	}

	if (g_TitleButtonPressed && g_TitleTimer > TICKS(666)) {
		titleSetNextMode(TITLEMODE_SKIP);
	}

	if (joyGetButtonsPressedThisFrame(0, 0xffffffff)) {
		g_TitleButtonPressed = g_TitleFastForward = true;

		if (g_TitleTimer < TICKS(549)) {
			titleSetNextMode(TITLEMODE_PDLOGO);
		}
	}
}

Gfx *titleRenderPdLogoModel(Gfx *gdl, struct model *model, bool arg2, float arg3, int arg4, float arg5, Mtxf *arg6, Vtx *vertices, Col *colours)
{
	struct modelrenderdata renderdata = {NULL, true, 3};
	int tmp2;
	int i;
	int j;
	Vtx *sp100;
	Col *spfc;
	union modelrwdata *tmp;
	struct modelrwdata_dl *rwdata;
	struct modelnode *node1;
	struct modelnode *node2;
	int s6;
	int k;
	struct modelrodata_dl *s5rodata;
	struct modelrodata_dl *s1rodata;
	int alpha1;
	int spcc[3];
	float spc0[3];
	Vtx *a3;
	int alpha2;
	Vtx *t0;
	Col *s1;
	Col *s2;
	Mtxf sp6c;

	tmp = modelGetNodeRwData(model, modelGetPart(model->definition, MODELPART_LOGO_0000));
	tmp->toggle.visible = arg2;

	tmp = modelGetNodeRwData(model, modelGetPart(model->definition, MODELPART_LOGO_0001));
	tmp->toggle.visible = !arg2;

	s6 = arg3 * 65536.0f;

	if (s6 < 0) {
		s6 = 0;
	} else if (s6 > 65536) {
		s6 = 65536;
	}

	alpha1 = s6 / 256;

	if (alpha1 > arg4) {
		alpha1 = arg4;
	}

	if (!arg2) {
		s6 = 65536 - s6;
		alpha1 = 256 - alpha1;
	}

	if (alpha1 < 0) {
		alpha1 = 0;
	} else if (alpha1 > 255) {
		alpha1 = 255;
	}

	alpha2 = arg5 * 256.0f;

	if (alpha2 < 0) {
		alpha2 = 0;
	} else if (alpha2 > 255) {
		alpha2 = 255;
	}

	sp100 = vertices;
	spfc = colours;

	for (i = 0; i < 4; i++) {
		if (i == 0) {
			node1 = modelGetPart(model->definition, MODELPART_LOGO_FRONTSIDE);
			node2 = modelGetPart(model->definition, MODELPART_LOGO_0003);
		} else if (i == 1) {
			node1 = modelGetPart(model->definition, MODELPART_LOGO_RIGHTSIDE);
			node2 = modelGetPart(model->definition, MODELPART_LOGO_0005);
		} else if (i == 2) {
			node1 = modelGetPart(model->definition, MODELPART_LOGO_BACKSIDE);
			node2 = modelGetPart(model->definition, MODELPART_LOGO_0007);
		} else {
			node1 = modelGetPart(model->definition, MODELPART_LOGO_LEFTSIDE);
			node2 = modelGetPart(model->definition, MODELPART_LOGO_0009);
		}

		if (node1 && node2) {
			if (arg2) {
				s5rodata = &node1->rodata->dl;
				s1rodata = &node2->rodata->dl;
				rwdata = modelGetNodeRwData(model, node1);
			} else {
				s5rodata = &node2->rodata->dl;
				s1rodata = &node1->rodata->dl;
				rwdata = modelGetNodeRwData(model, node2);
			}

			s1 = (Col *)ALIGN8(s5rodata->numvertices * sizeof(Vtx) + (uintptr_t)s5rodata->vertices);
			s2 = (Col *)ALIGN8(s1rodata->numvertices * sizeof(Vtx) + (uintptr_t)s1rodata->vertices);

			a3 = s5rodata->vertices;
			t0 = s1rodata->vertices;

			rwdata->vertices = sp100;
			rwdata->colours = spfc;

			for (j = 0; j < s5rodata->numvertices; j++) {
				sp100[j] = a3[j];

				tmp2 = (t0[j].x - a3[j].x) * s6;
				tmp2 = tmp2 / 65536;
				sp100[j].x += (int16_t) tmp2;

				tmp2 = (t0[j].y - a3[j].y) * s6;
				tmp2 = tmp2 / 65536;
				sp100[j].y += (int16_t) tmp2;

				tmp2 = (t0[j].z - a3[j].z) * s6;
				tmp2 = tmp2 / 65536;
				sp100[j].z += (int16_t) tmp2;
			}

			for (j = 0; j < s5rodata->numcolours; j++) {
				spcc[0] = ((int8_t) s2[j].r * s6 + (int8_t) s1[j].r * (65536 - s6)) / 65536;
				spcc[1] = ((int8_t) s2[j].g * s6 + (int8_t) s1[j].g * (65536 - s6)) / 65536;
				spcc[2] = ((int8_t) s2[j].b * s6 + (int8_t) s1[j].b * (65536 - s6)) / 65536;

				spc0[0] = spcc[0];
				spc0[1] = spcc[1];
				spc0[2] = spcc[2];

				if (spc0[0] != 0.0f || spc0[1] != 0.0f || spc0[2] != 0.0f) {
					utilsNormalizeF(&spc0[0], &spc0[1], &spc0[2]);
				}

				spfc[j].r = (int) (spc0[0] * 127.0f);
				spfc[j].g = (int) (spc0[1] * 127.0f);
				spfc[j].b = (int) (spc0[2] * 127.0f);
				spfc[j].a = alpha2;
			}

			sp100 = (void *)ALIGN8(s5rodata->numvertices * sizeof(Vtx) + (uintptr_t)sp100);
			spfc = (void *)ALIGN8(s5rodata->numcolours * sizeof(uint32_t) + (uintptr_t)spfc);
		}
	}

	gDPSetPrimColor(gdl++, 0, 0, 0x00, 0x00, 0x00, alpha1);

	renderdata.unk00 = arg6;
	renderdata.unk10 = gfxAllocate(model->definition->nummatrices * sizeof(Mtxf));

	mtx4Copy((Mtx*)arg6, (Mtx*)renderdata.unk10);

	model->matrices = renderdata.unk10;

	modelUpdateRelations(model);

	renderdata.flags = 3;
	renderdata.zbufferenabled = false;
	renderdata.gdl = gdl;

	modelRender(&renderdata, model);

	gdl = renderdata.gdl;

	return gdl;
}

float g_PdLogoYRotCur = 0;
float g_PdLogoYRotSpeed = 0;
float g_PdLogoXRotCur = 0;
float g_PdLogoXRotSpeed = 0;
float g_PdLogoScale = 1;
float g_PdLogoFrac = 0;
bool g_PdLogoUseCombinedModel = false;
float g_PdLogoEndYRot = 0;
float g_PdLogoAmbientLightFrac = 1;
int g_PdLogoBlackTimer = 0;
bool g_PdLogoYRotEnabled = false;
int g_PdLogoPreMorphTimer = 0;
bool g_PdLogoMorphing = false;
int g_PdLogoExitTimer = 0;
int g_PdLogoMorphEndTimer = 0;
bool g_PdLogoYRotStopping = false;
bool g_PdLogoDarkenEnabled = false;
bool g_PdLogoPointlessTimerEnabled = false;
int g_PdLogoPreTitleTimer = 0;
float g_PdLogoTitleStepFrac = 0;
int g_PdLogoTitleStep = 0;
bool g_PdLogoTitlePresenting = false;
int g_PdLogoPointlessTimer = 0;
bool g_PdLogoLightMoving = false;
float g_PdLogoLightDirFrac = 0;


/**
 * Skip immediately to the "PERFECT DARK" part of the PdLogo mode.
 *
 * Assumes the title mode is already PdLogo, but at an earlier point.
 */
void titleSkipToPdTitle(void)
{
	g_PdLogoYRotCur = 0;
	g_PdLogoYRotSpeed = 0;
	g_PdLogoXRotCur = 0;
	g_PdLogoXRotSpeed = 0;
	g_PdLogoScale = 0.35f;
	g_PdLogoFrac = 1;
	g_PdLogoTitleStepFrac = 0.63f;
	g_PdLogoLightDirFrac = 0.19975f;
	g_PdLogoEndYRot = 0;
	g_PdLogoAmbientLightFrac = 0;

	g_PdLogoUseCombinedModel = true;
	g_PdLogoBlackTimer = 0;
	g_PdLogoYRotEnabled = false;
	g_PdLogoPreMorphTimer = 0;
	g_PdLogoMorphing = false;
	g_PdLogoExitTimer = 0;
	g_PdLogoMorphEndTimer = 0;
	g_PdLogoYRotStopping = false;
	g_PdLogoDarkenEnabled = false;
	g_PdLogoPointlessTimerEnabled = false;
	g_PdLogoPreTitleTimer = 0;
	g_PdLogoTitleStep = 1;
	g_PdLogoTitlePresenting = true;
	g_PdLogoPointlessTimer = 0;
	g_PdLogoLightMoving = true;
	g_TitleTimer = TICKS(549);
	g_PdLogoIsFirstTick = false;

	musicStartTemporaryPrimary(MUSIC_TITLE2);
}

Gfx *titleRenderPdLogo(Gfx *gdl)
{
	struct modelrenderdata renderdata = {NULL, true, 3};
	Mtxf sp2b0;
	Mtxf sp270;
	Mtxf sp230;
	struct model *model;
	struct modelnode *node;
	Mtxf sp1e8;
	Mtxf sp1a8;

	float yrotmax = 4.240475f;
	float xrotmax = 0.47116387f;
	float xrotmin = 0.0f;
	float yrotaccel = 0.00018846555f;
	float xrotaccel = 0.00011307933f;
	float yrotmaxspeed = 0.018846555f;
	float xrotmaxspeed = 0.011307933f;

	int premorphduration = TICKS(80);
	float amblightinc = 0.0075f;
	float lightdirinc = 0.017f;
	float logoinc = 0.004f;
	float step0inc = 0.025f;
	float step1inc = 0.09f;
	float step2inc = 0.1f;

	int tmp;
	struct modelrodata_dl *rodata;
	struct modelrwdata_dl *rwdata;

	float sp13c;

	Gfx *tmpgdl;
	LookAt *lookat;
	Mtx spf0;

	sp13c = g_TitleTimer / TICKS(4500.0f) - 0.1f;

	if (g_PdLogoIsFirstTick) {
		g_PdLogoYRotCur = yrotmax;
		g_PdLogoYRotSpeed = yrotmaxspeed;
		g_PdLogoXRotCur = xrotmax;
		g_PdLogoXRotSpeed = 0.0f;
		g_PdLogoScale = 0.35f;
		g_PdLogoFrac = 0.0f;
		g_PdLogoUseCombinedModel = false;
		g_PdLogoAmbientLightFrac = 1.0f;
		g_PdLogoIsFirstTick = false;
		g_PdLogoBlackTimer = 1;
		g_PdLogoYRotEnabled = false;
		g_PdLogoPreMorphTimer = 0;
		g_PdLogoMorphing = false;
		g_PdLogoExitTimer = 0;
		g_PdLogoMorphEndTimer = 0;
		g_PdLogoYRotStopping = false;
		g_PdLogoDarkenEnabled = false;
		g_PdLogoPointlessTimerEnabled = false;
		g_PdLogoPreTitleTimer = 0;
		g_PdLogoTitleStepFrac = 0.0f;
		g_PdLogoTitlePresenting = false;
		g_PdLogoTitleStep = -1;
		g_PdLogoPointlessTimer = 0;
		g_PdLogoLightMoving = false;
		g_PdLogoLightDirFrac = 0.0f;
	}

	if (g_PdLogoBlackTimer != 0) {
		g_PdLogoBlackTimer++;

		if (g_PdLogoBlackTimer >= 4) {
			g_PdLogoBlackTimer = 0;
			g_PdLogoYRotEnabled = true;
			g_PdLogoPreMorphTimer = 1;
		}
	}

	if (g_PdLogoYRotStopping) {
		if (g_PdLogoYRotCur < g_PdLogoEndYRot) {
			applySpeed(&g_PdLogoYRotCur, g_PdLogoEndYRot, &g_PdLogoYRotSpeed, yrotaccel, yrotaccel, yrotmaxspeed);

			if (g_PdLogoYRotCur >= g_PdLogoEndYRot) {
				g_PdLogoYRotCur = g_PdLogoEndYRot;
				g_PdLogoYRotSpeed = 0.0f;
			}

			if (g_PdLogoYRotCur >= M_TAU) {
				g_PdLogoYRotCur -= M_TAU;
				g_PdLogoEndYRot -= M_TAU;
			} else if (g_PdLogoYRotCur < 0.0f) {
				g_PdLogoYRotCur += M_TAU;
				g_PdLogoEndYRot += M_TAU;
			}
		}

		if (g_PdLogoYRotCur >= g_PdLogoEndYRot) {
			g_PdLogoYRotStopping = false;
		}
	} else if (g_PdLogoYRotEnabled) {
		g_PdLogoYRotCur += g_PdLogoYRotSpeed * g_Vars.lvupdate60;
		if (g_PdLogoYRotCur >= M_TAU) {
			g_PdLogoYRotCur -= M_TAU;
		} else if (g_PdLogoYRotCur < 0.0f) {
			g_PdLogoYRotCur += M_TAU;
		}
	}

	if (g_PdLogoPreMorphTimer != 0) {
		g_PdLogoPreMorphTimer += g_Vars.lvupdate60;

		if (g_PdLogoPreMorphTimer > 0) {
			g_PdLogoFrac = (float) g_PdLogoPreMorphTimer / premorphduration;
		} else {
			g_PdLogoFrac = 0.0f;
		}

		if (g_PdLogoPreMorphTimer > premorphduration) {
			g_PdLogoPreMorphTimer = 0;
			g_PdLogoMorphing = true;
			g_PdLogoFrac = 0.0f;
			g_PdLogoUseCombinedModel = true;
		}
	}

	if (g_PdLogoMorphing) {
		g_PdLogoFrac += logoinc * g_Vars.lvupdate60freal;

		if (g_PdLogoFrac >= 0.8f) {
			if (g_PdLogoMorphEndTimer == 0) {
				g_PdLogoMorphEndTimer = 1;
			}
		}

		if (g_PdLogoFrac >= 1.0f) {
			g_PdLogoFrac = 1.0f;
			g_PdLogoMorphing = false;
		}
	}

	if (g_PdLogoMorphEndTimer != 0) {
		g_PdLogoMorphEndTimer += g_Vars.lvupdate60;

		if (g_PdLogoXRotCur > xrotmin) {
			// Implement the camera lowering effect, but it's actually
			// the model that rotates upwards to face the camera
			applyRotation(&g_PdLogoXRotCur, xrotmin, &g_PdLogoXRotSpeed, xrotaccel, xrotaccel, xrotmaxspeed);

			if (g_PdLogoXRotCur <= xrotmin) {
				g_PdLogoXRotCur = xrotmin;
				g_PdLogoXRotSpeed = 0.0f;
			}
		}

		if (g_PdLogoMorphEndTimer > TICKS(30) && g_PdLogoMorphEndTimer - g_Vars.lvupdate60 <= TICKS(30)) {
			// Start slowing the spinning rotation
			g_PdLogoYRotEnabled = false;
			g_PdLogoYRotStopping = true;

			tmp = g_PdLogoYRotCur * 4.0f / M_TAU;
			tmp += 2;

			g_PdLogoEndYRot = tmp * M_TAU / 4.0f;
		}

		if (g_PdLogoMorphEndTimer > TICKS(100) && g_PdLogoMorphEndTimer - g_Vars.lvupdate60 <= TICKS(100)) {
			g_PdLogoDarkenEnabled = true;
		}

		if (!g_PdLogoYRotStopping && g_PdLogoXRotCur <= 0.0f) {
			// Spinning has stopped and model is also facing camera vertically
			g_PdLogoMorphEndTimer = 0;
			g_PdLogoDarkenEnabled = true;
		}
	}

	if (g_PdLogoDarkenEnabled) {
		// Fading out the side and back faces of the logo. This is done by adjusting the ambient lighting.
		// The front face is excluded from this further below.
		g_PdLogoAmbientLightFrac -= amblightinc * g_Vars.lvupdate60freal;

		if (g_PdLogoAmbientLightFrac <= 0.0f) {
			g_PdLogoAmbientLightFrac = 0.0f;
			g_PdLogoDarkenEnabled = false;
			g_PdLogoPreTitleTimer = 1;
		}
	}

	if (g_PdLogoPreTitleTimer != 0) {
		g_PdLogoPreTitleTimer += g_Vars.lvupdate60;

		if (g_PdLogoPreTitleTimer > TICKS(20)) {
			g_PdLogoPreTitleTimer = 0;
			g_PdLogoPointlessTimerEnabled = true;
		}
	}

	if (g_PdLogoPointlessTimerEnabled) {
		g_PdLogoPointlessTimerEnabled = false;
		g_PdLogoPointlessTimer = 1;
	}

	if (g_PdLogoPointlessTimer != 0) {
		g_PdLogoPointlessTimer += g_Vars.lvupdate60;

		if (g_PdLogoPointlessTimer > 0) {
			g_PdLogoPointlessTimer = 0;
			g_PdLogoTitlePresenting = true;
			g_PdLogoTitleStep = 1;
			g_PdLogoLightMoving = true;
		}
	}

	if (g_PdLogoTitlePresenting) {

		if (g_PdLogoTitleStep == 0) {
			g_PdLogoTitleStepFrac += step0inc;
		} else if (g_PdLogoTitleStep == 1) {
			g_PdLogoTitleStepFrac += step1inc;
		} else {
			g_PdLogoTitleStepFrac += step2inc;
		}

		if (g_PdLogoTitleStepFrac >= 1.0f) {
			g_PdLogoTitleStepFrac = 0.0f;
			g_PdLogoTitleStep++;
			if (g_PdLogoTitleStep == 10)
			{
				g_PdLogoTitlePresenting = false;
				g_PdLogoExitTimer = 1;
			}
		}
	}

	if (g_PdLogoLightMoving) {
		g_PdLogoLightDirFrac += lightdirinc * g_Vars.lvupdate60freal;

		if (g_PdLogoLightDirFrac >= 1.0f) {
			g_PdLogoLightDirFrac = 1.0f;
			g_PdLogoLightMoving = false;
		}
	}

	if (g_PdLogoExitTimer != 0) {
		g_PdLogoExitTimer += g_Vars.lvupdate60;

		if (g_PdLogoExitTimer > TICKS(60)) {
			g_PdLogoExitTimer = 0;
			g_PdLogoTriggerExit = true;
		}
	}

	gdl = viSetFillColour(gdl, 0, 0, 0);
	gdl = viFillBuffer(gdl);

	if (g_PdLogoBlackTimer != 0) {
		return gdl;
	}

	lookat = gfxAllocateLookAt(2);
	mtxLookAtReflect(&spf0, lookat, 0.0f, 0.0f, 4000.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	gSPLookAt(gdl++, lookat);

	float angle1;
	float angle2;

	angle1 = (g_PdLogoLightDirFrac + -1.0f);
	angle2 = 0.0f - 0.15f * g_PdLogoLightDirFrac;
	angle1 = M_PI + angle1 * M_PI;
	angle2 = M_PI + angle2 * M_PI;

	g_TitleLightPdLogoMain.a.l.col[0] = g_TitleLightPdLogoMain.a.l.col[1] = g_TitleLightPdLogoMain.a.l.col[2] = g_TitleLightPdLogoMain.a.l.colc[0] = g_TitleLightPdLogoMain.a.l.colc[1] = g_TitleLightPdLogoMain.a.l.colc[2] = 0;
	g_TitleLightPdLogoMain.l[0].l.col[0] = g_TitleLightPdLogoMain.l[0].l.col[1] = g_TitleLightPdLogoMain.l[0].l.col[2] = g_TitleLightPdLogoMain.l[0].l.colc[0] = g_TitleLightPdLogoMain.l[0].l.colc[1] = g_TitleLightPdLogoMain.l[0].l.colc[2] = 0xff;
	g_TitleLightPdLogoMain.l[0].l.dir[0] = 127.0f * sinf(angle1) * cosf(angle2);
	g_TitleLightPdLogoMain.l[0].l.dir[1] = 127.0f * sinf(angle2);
	g_TitleLightPdLogoMain.l[0].l.dir[2] = 127.0f * cosf(angle1) * cosf(angle2);
	

	mtxBuildLookAtMatrixF((Mtx*)&sp2b0, 0.0f, 0.0f, 4000.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	model = g_PdLogoUseCombinedModel == true ? g_TitleModel : g_TitleModelNLogo2;

	mtx4LoadYRotation(g_PdLogoYRotCur, (Mtx*)&sp1e8);
	mtx4LoadXRotation(g_PdLogoXRotCur, (Mtx*)&sp1a8);
	mtx4MultMtx4InPlace((Mtx*)&sp1a8, (Mtx*)&sp1e8);
	mtx4MultMtx4((Mtx*)&sp2b0, (Mtx*)&sp1e8, (Mtx*)&sp270);
	mtxScaleRotationPart(g_PdLogoScale, (Mtx*)&sp270);

	g_TitleLightPdLogoNotFront.a.l.col[0] = g_TitleLightPdLogoNotFront.a.l.col[1] = g_TitleLightPdLogoNotFront.a.l.col[2] = g_TitleLightPdLogoNotFront.a.l.colc[0] = g_TitleLightPdLogoNotFront.a.l.colc[1] = g_TitleLightPdLogoNotFront.a.l.colc[2] = 255.0f * g_PdLogoAmbientLightFrac;

	// Inject a SetLights command into the displaylists for each of the four logo sides.
	// The front face gets a different light which makes it remain lit when the other sides go dark.
	
	int numvertices = 0;
	int numcolours = 0;

	node = modelGetPart(model->definition, MODELPART_LOGO_FRONTSIDE);

	if (node != NULL) {
		rodata = &node->rodata->dl;
		numvertices += rodata->numvertices + 1;
		numcolours += rodata->numcolours + 1;
		rwdata = modelGetNodeRwData(model, node);
		rwdata->gdl = tmpgdl = gfxAllocate(5 * sizeof(Gfx));

		gSPSetLights1(tmpgdl++, g_TitleLightPdLogoFront);
		gSPBranchList(tmpgdl++, rodata->opagdl);
	}

	node = modelGetPart(model->definition, MODELPART_LOGO_RIGHTSIDE);

	if (node != NULL) {
		rodata = &node->rodata->dl;
		numvertices += rodata->numvertices + 1;
		numcolours += rodata->numcolours + 1;
		rwdata = modelGetNodeRwData(model, node);
		rwdata->gdl = tmpgdl = gfxAllocate(5 * sizeof(Gfx));

		if (g_PdLogoAmbientLightFrac > 0.0f) {
			gSPSetLights1(tmpgdl++, g_TitleLightPdLogoNotFront);
			gSPBranchList(tmpgdl++, rodata->opagdl);
		} else {
			gSPEndDisplayList(tmpgdl++);
		}
	}

	node = modelGetPart(model->definition, MODELPART_LOGO_BACKSIDE);

	if (node != NULL) {
		rodata = &node->rodata->dl;
		numvertices += rodata->numvertices + 1;
		numcolours += rodata->numcolours + 1;
		rwdata = modelGetNodeRwData(model, node);
		rwdata->gdl = tmpgdl = gfxAllocate(5 * sizeof(Gfx));

		if (g_PdLogoAmbientLightFrac > 0.0f) {
			gSPSetLights1(tmpgdl++, g_TitleLightPdLogoNotFront);
			gSPBranchList(tmpgdl++, rodata->opagdl);
		} else {
			gSPEndDisplayList(tmpgdl++);
		}
	}

	node = modelGetPart(model->definition, MODELPART_LOGO_LEFTSIDE);

	if (node != NULL) {
		rodata = &node->rodata->dl;
		numvertices += rodata->numvertices + 1;
		numcolours += rodata->numcolours + 1;
		rwdata = modelGetNodeRwData(model, node);
		rwdata->gdl = tmpgdl = gfxAllocate(5 * sizeof(Gfx));

		if (g_PdLogoAmbientLightFrac > 0.0f) {
			gSPSetLights1(tmpgdl++, g_TitleLightPdLogoNotFront);
			gSPBranchList(tmpgdl++, rodata->opagdl);
		} else {
			gSPEndDisplayList(tmpgdl++);
		}
	}

	gdl = titleRenderPdLogoModel(gdl, model, true, g_PdLogoFrac, 240, 1.0f, &sp270, gfxAllocateVertices(numvertices), gfxAllocateColours(numcolours));

	gSPSetLights1(gdl++, g_TitleLightPdLogoMain);
	{
		struct coord sp64 = {0, 0, 1000};
		mtx4LoadTranslation(&sp64, (Mtx*)&sp1e8);
	}

	mtxScale3x4(1.0f + sp13c, (Mtx*)&sp1e8);
	mtx4MultMtx4((Mtx*)&sp2b0, (Mtx*)&sp1e8, (Mtx*)&sp230);
	mtxScaleRotationPart(0.308f, (Mtx*)&sp230);

	// Render the "PERFECT DARK" model
	if (g_PdLogoTitleStep >= 0) {
		if (g_PdLogoTitleStep == 0) {
			// empty
		} else if (g_PdLogoTitleStep == 1) {
			bool visible = g_PdLogoTitleStepFrac < 0.5f;
			model = g_TitleModelPdThree;
			gdl = titleRenderPdLogoModel(gdl, model, visible, g_PdLogoTitleStepFrac, 255, g_PdLogoTitleStepFrac, &sp230, g_PdLogoVertices[g_PdLogoVtxColIndex], g_PdLogoColours[g_PdLogoVtxColIndex]);
		} else if (g_PdLogoTitleStep == 2) {
			bool visible = g_PdLogoTitleStepFrac < 0.5f;
			model = g_TitleModelPdTwo;
			gdl = titleRenderPdLogoModel(gdl, model, visible, 1.0f - g_PdLogoTitleStepFrac, 255, 1.0f, &sp230, g_PdLogoVertices[g_PdLogoVtxColIndex], g_PdLogoColours[g_PdLogoVtxColIndex]);
		} else if (g_PdLogoTitleStep == 3) {
			bool visible = g_PdLogoTitleStepFrac < 0.5f;
			model = g_TitleModelPdTwo;
			gdl = titleRenderPdLogoModel(gdl, model, visible, g_PdLogoTitleStepFrac, 255, 1.0f, &sp230, g_PdLogoVertices[g_PdLogoVtxColIndex], g_PdLogoColours[g_PdLogoVtxColIndex]);
		} else {
			model = g_TitleModelPdTwo;
			gdl = titleRenderPdLogoModel(gdl, model, false, 1.0f, 255, 1.0f, &sp230, g_PdLogoVertices[g_PdLogoVtxColIndex], g_PdLogoColours[g_PdLogoVtxColIndex]);
		}
	}

	return gdl;
}

void titleInitNintendoLogo(void)
{
	int8_t *nextaddr = g_TitleModelBuffer;

	g_TitleFastForward = false;

	if (g_TitleButtonPressed) {
		g_TitleTimer = TICKS(180);
	} else {
		g_TitleTimer = 0;
	}

	{
		struct coord coord = {0, 0, 0};

		g_ModelStates[MODEL_NINTENDOLOGO].modeldef = modeldefLoad(g_ModelStates[MODEL_NINTENDOLOGO].fileid, nextaddr, TITLE_ALLOCSIZE, 0);

		modelAllocateRwData(g_ModelStates[MODEL_NINTENDOLOGO].modeldef);
		g_TitleModel = modelmgrInstantiateModelWithoutAnim(g_ModelStates[MODEL_NINTENDOLOGO].modeldef);
		modelSetScale(g_TitleModel, 1);
		modelSetRootPosition(g_TitleModel, &coord);
		joySetAllowTitleInput(false);
	}
}

void titleExitNintendoLogo(void)
{
	modelmgrFreeModel(g_TitleModel);
	joySetAllowTitleInput(true);
}

/**
 * If no button has been pressed during the Rare logo (so g_TitleButtonPressed
 * is false) and the player presses a button within 140 ticks of the Nintendo
 * logo, the Nintendo logo sequence will play at double speed until it exits at
 * the 140 tick mark.
 */
void titleTickNintendoLogo(void)
{
	viSetFovY(60);
	viSetAspect(videoGetAspect());
	viSetZRange(100, 10000);
	viSetUseZBuf(false);

	g_TitleTimer += g_Vars.lvupdate60;

	if (g_TitleFastForward) {
		g_TitleTimer += g_Vars.lvupdate60;
	}

	if (joyGetButtonsPressedThisFrame(0, 0xffffffff)) {
		if (0 == 1) { // "Warm" reset
			g_TitleButtonPressed = true;
			titleSetNextMode(TITLEMODE_PDLOGO);
		} else if (!g_TitleButtonPressed) {
			g_TitleFastForward = true;
		}
	}

	if (g_TitleFastForward && !g_TitleButtonPressed && g_TitleTimer > TICKS(140)) {
		g_TitleButtonPressed = true;
		g_TitleFastForward = false;
		titleSetNextMode(TITLEMODE_PDLOGO);
	}

	if (g_TitleTimer > TICKS(240))
	{
		titleSetNextMode(TITLEMODE_PDLOGO);
	}
}

Gfx *titleRenderNintendoLogo(Gfx *gdl)
{
	struct modelrenderdata renderdata = { NULL, true, 3 };
	int i;
	int j;
	Mtxf sp108;
	float fracdone = g_TitleTimer / (TICKS(240.0f));
	struct coord lightdir = {0, 0, 0};
	int v0;

	gdl = titleClear(gdl);

	gSPSetLights1(gdl++, g_TitleLightNintendoRare);

	lightdir.z = sinf((1 - fracdone) * 1.5f * M_PI);
	lightdir.x = cosf((1 - fracdone) * 1.5f * M_PI);

	utilsNormalizeF(&lightdir.x, &lightdir.y, &lightdir.z);

	v0 = 255;

	if (fracdone < 0.1f) {
		v0 = 255.0f * fracdone / 0.1f;
	}

	if (fracdone > 0.9f) {
		v0 = (1 - fracdone) * 255.0f / 0.1f;
	}

	if (v0 > 255) {
		v0 = 255;
	}

	if (v0 < 0) {
		v0 = 0;
	}

	titleSetLight(&g_TitleLightNintendoRare, v0, v0, v0, 0.0f, &lightdir);
	{
		Mtxf spa8;
		struct coord sp9c;
		Mtxf sp54;

		sp9c.x = 0.0f;

		if (fracdone < 0.4f) {
			sp9c.x = (-cosf((1.0f - fracdone / .4f) * M_PI) * 0.5f + 0.5f) * 1.5707963705063f;
		}

		sp9c.y = (-cosf((1.0f - (fracdone / 1)) * M_PI) * 0.5f + .5f) * 0.35f;
		sp9c.z = 0.0f;

		mtx4LoadRotation(&sp9c, (Mtx*)&spa8);
		mtxScale3x4(fracdone * 0.2f + 1.0f, (Mtx*)&spa8);

		mtxBuildLookAtMatrixF((Mtx*)&sp108,
				/* pos  */ 0.0f, 0.0f, 4000,
				/* look */ 0.0f, 0.0f, 0.0f,
				/* up   */ 0.0f, 1.0f, 0.0f);

		mtx4MultMtx4InPlace((Mtx*)&sp108, (Mtx*)&spa8);
		mtx4Copy((Mtx*)&spa8, (Mtx*)&sp108);
		renderdata.unk00 = &sp108;

		renderdata.unk10 = gfxAllocate(g_TitleModel->definition->nummatrices * sizeof(Mtxf));
		mtx4Copy((Mtx*)&sp108, (Mtx*)renderdata.unk10);
		g_TitleModel->matrices = renderdata.unk10;

		modelUpdateRelations(g_TitleModel);

		renderdata.flags = 3;
		renderdata.zbufferenabled = false;
		renderdata.gdl = gdl;

		modelRender(&renderdata, g_TitleModel);

		gdl = renderdata.gdl;
	}

	return gdl;
}

void titleInitRareLogo(void)
{
	int8_t *nextaddr = g_TitleModelBuffer;

	g_TitleTimer = -3;

	struct coord coord = {0, 0, 0};

	g_ModelStates[MODEL_RARELOGO].modeldef = modeldefLoad(g_ModelStates[MODEL_RARELOGO].fileid, nextaddr, TITLE_ALLOCSIZE, 0);

	modelAllocateRwData(g_ModelStates[MODEL_RARELOGO].modeldef);
	g_TitleModel = modelmgrInstantiateModelWithoutAnim(g_ModelStates[MODEL_RARELOGO].modeldef);
	modelSetScale(g_TitleModel, 1);
	modelSetRootPosition(g_TitleModel, &coord);
	musicQueueStopAllEvent();
	joySetAllowTitleInput(false);

	if (!g_IsTitleDemo) {
		g_IsTitleDemo = true;
	}
	
}

void titleExitRareLogo(void)
{
	modelmgrFreeModel(g_TitleModel);
	joySetAllowTitleInput(true);
}

/**
 * If a button is pressed between 0-59 ticks, set the timer to 100 and schedule
 * the next mode for 140 (ie. in 40 ticks time).
 *
 * If a button is pressed at 60+ ticks, set the next mode immediately.
 *
 * So the fastest way to skip through the Rare logo is to press a button as
 * early as possible, but if you press the button between 20-59 ticks it'll end
 * up taking longer than if you'd waited a second.
 */
void titleTickRareLogo(void)
{
	viSetFovY(60);
	viSetAspect(videoGetAspect());
	viSetZRange(100, 10000);
	viSetUseZBuf(false);

	if (g_TitleTimer < 0) {
		g_TitleTimer++;
	} else {
		if (g_TitleTimer == 0) {
			musicQueueStartEvent(TRACKTYPE_PRIMARY, MUSIC_TITLE1, 0, 0x7fff);
		}

		g_TitleTimer += g_Vars.lvupdate60;

		if (joyGetButtonsPressedThisFrame(0, 0xffffffff)) {
			if (!g_TitleButtonPressed) {
				if (g_TitleTimer < TICKS(60)) {
					g_TitleButtonPressed = true;
			
					if (g_TitleTimer < TICKS(100)) {
						g_TitleTimer = TICKS(100);
					}
				} else {
					g_TitleFastForward = true;
					g_TitleButtonPressed = true;
				}
			}
		}

		if (g_TitleTimer > TICKS(240)
				|| g_TitleFastForward
				|| (g_TitleButtonPressed && g_TitleTimer > TICKS(140))) {
			titleSetNextMode(TITLEMODE_NINTENDOLOGO);
		}
	}
}

float titleRotateClockwise(float arg0)
{
	return ((1.0f - arg0) + (1.0f - arg0)) * M_PI - DEG2RAD(90);
}

Gfx *titleRenderRareLogo(Gfx *gdl)
{
	struct modelrenderdata renderdata = { NULL, true, 3 };
	int i;
	float fracdone = g_TitleTimer / TICKS(240.0f);
	Mtxf sp118;
	int j;
	int s0;

	gdl = titleClear(gdl);

	if (g_TitleTimer < 0) {
		return gdl;
	}

	
	struct coord lightdir = {0, 0, 0};
	float tmp;
	Mtxf spc0;
	struct coord spb4;
	struct modelrwdata_toggle *rwdata;

	lightdir.z = sinf(titleRotateClockwise(fracdone));
	lightdir.x = cosf(titleRotateClockwise(fracdone));

	utilsNormalizeF(&lightdir.x, &lightdir.y, &lightdir.z);

	s0 = 255;

	if (fracdone < 0.1f) {
		s0 = 255.0f * fracdone / 0.1f;
	}

	if (s0 > 255) {
		s0 = 255;
	}

	if (s0 < 0) {
		s0 = 0;
	}

	if (fracdone < 0.2f) {
		titleSetLight(&g_TitleLightNintendoRare,
				(int)(255.0f * fracdone / 0.2f),
				(int)(255.0f * fracdone / 0.2f),
				(int)(255.0f * fracdone / 0.2f),
				0, &lightdir);
	} else {
		titleSetLight(&g_TitleLightNintendoRare, s0, s0, s0, 0, &lightdir);
	}

	s0 = s0 * 192 / 255;

	if (fracdone < 0.5f) {
		lightdir.z = sinf(titleRotateClockwise(0.5f));
		lightdir.x = cosf(titleRotateClockwise(0.5f));
		utilsNormalizeF(&lightdir.x, &lightdir.y, &lightdir.z);
		titleSetLight(&g_TitleLightRareLogo, s0, s0, s0, 0, &lightdir);
	} else {
		titleSetLight(&g_TitleLightRareLogo, s0, s0, s0, 0, &lightdir);
	}

	tmp = 1 - fracdone;

	spb4.x = 0;
	spb4.y = 1.5707963705063f * tmp;
	spb4.z = 0;

	mtx4LoadRotation(&spb4, (Mtx*)&spc0);
	mtxScale3x4(1 + fracdone * 0.25f, (Mtx*)&spc0);

	mtxBuildLookAtMatrixF((Mtx*)&sp118,
			/* pos  */ 0, 0, 4000,
			/* look */ 0, 0, 0,
			/* up   */ 0, 1, 0);

	mtx4MultMtx4InPlace((Mtx*)&sp118, (Mtx*)&spc0);
	mtx4Copy((Mtx*)&spc0, (Mtx*)&sp118);

	renderdata.unk00 = &sp118;
	renderdata.unk10 = gfxAllocate(g_TitleModel->definition->nummatrices * sizeof(Mtxf));
	mtx4Copy((Mtx*)&sp118, (Mtx*)renderdata.unk10);

	g_TitleModel->matrices = renderdata.unk10;

	modelUpdateRelations(g_TitleModel);

	rwdata = modelGetNodeRwData(g_TitleModel, modelGetPart(g_TitleModel->definition, MODELPART_RARELOGO_000B));

	if (rwdata) {
		rwdata->visible = false;
	}

	rwdata = modelGetNodeRwData(g_TitleModel, modelGetPart(g_TitleModel->definition, MODELPART_RARELOGO_000D));

	if (rwdata) {
		rwdata->visible = true;
	}

	rwdata = modelGetNodeRwData(g_TitleModel, modelGetPart(g_TitleModel->definition, MODELPART_RARELOGO_000C));

	if (rwdata) {
		rwdata->visible = false;
	}

	gSPSetLights1(gdl++, g_TitleLightNintendoRare);

	renderdata.flags = 3;
	renderdata.zbufferenabled = 0;
	renderdata.gdl = gdl;

	modelRender(&renderdata, g_TitleModel);

	gdl = renderdata.gdl;

	rwdata = modelGetNodeRwData(g_TitleModel, modelGetPart(g_TitleModel->definition, MODELPART_RARELOGO_000B));

	if (rwdata) {
		rwdata->visible = true;
	}

	rwdata = modelGetNodeRwData(g_TitleModel, modelGetPart(g_TitleModel->definition, MODELPART_RARELOGO_000D));

	if (rwdata) {
		rwdata->visible = false;
	}

	rwdata = modelGetNodeRwData(g_TitleModel, modelGetPart(g_TitleModel->definition, MODELPART_RARELOGO_000C));

	if (rwdata) {
		rwdata->visible = true;
	}

	gSPSetLights1(gdl++, g_TitleLightNintendoRare);

	renderdata.flags = 3;
	renderdata.zbufferenabled = 0;
	renderdata.gdl = gdl;

	modelRender(&renderdata, g_TitleModel);

	gdl = renderdata.gdl;

	return gdl;
}

int g_NumPlayers = 0;

int getNumPlayers(void)
{
	return g_NumPlayers;
}

void setNumPlayers(int numplayers)
{
	g_NumPlayers = numplayers;
}

int playerGetTeam(int playernum)
{
	return g_PlayerConfigsArray[g_Vars.playerstats[playernum].mpindex].base.team;
}

void playerSetTeam(int playernum, int team)
{
	g_PlayerConfigsArray[g_Vars.playerstats[playernum].mpindex].base.team = team;
}

void titleInitSkip(void)
{
	g_TitleNextStage = STAGE_CITRAINING;

	setNumPlayers(1);

	if (g_IsTitleDemo) {
		g_TitleNextStage = STAGE_DEFECTION;
		g_IsTitleDemo = false;
	}

	mainChangeToStage(g_TitleNextStage);

	g_Vars.bondplayernum = 0;
	g_Vars.coopplayernum = -1;
	g_Vars.antiplayernum = -1;

	lvSetDifficulty(DIFF_A);
}

void titleSetNextMode(int mode)
{
	if (g_TitleDelayedMode != mode) {
		g_TitleNextMode = mode;
	}
}

void titleTick(void)
{
	viSetAspect(videoGetAspect());
	//viSetSize(576, g_TitleViewHeight);
	viSetSize(videoGetWidth(), videoGetHeight());
	viSetBufSize(576, g_TitleViewHeight);
	playermgrSetViewSize(576, g_TitleViewHeight);
	viSetViewSize(576, g_TitleViewHeight);
	playermgrSetViewPosition(0, 0);
	viSetViewPosition(0, 0);

	// If there's a new mode to transition to, schedule it to apply in 3 ticks
	// time and call the exit function for the current mode.
	if (g_TitleNextMode >= 0) {
		g_TitleDelayedTimer = 3;
		g_TitleDelayedMode = g_TitleNextMode;

		switch (g_TitleMode) {
		case TITLEMODE_LEGAL:
			break;
		case TITLEMODE_PDLOGO:
			titleExitPdLogo();
			break;
		case TITLEMODE_NINTENDOLOGO:
			titleExitNintendoLogo();
			break;
		case TITLEMODE_RARELOGO:
			titleExitRareLogo();
			break;
		}

		g_TitleNextMode = -1;
	}

	// If there's a new mode scheduled, tick the timer down
	if (g_TitleDelayedTimer != 0) {
		g_TitleDelayedTimer--;

		if (g_TitleMode == -1) {
			g_TitleDelayedTimer = 0;
		}

		if (g_TitleDelayedTimer == 0 && g_TitleDelayedMode != -1) {
			g_TitleNextMode = g_TitleDelayedMode;
			g_TitleDelayedMode = -1;
		}
	}

	// Apply new mode
	if (g_TitleNextMode >= 0) {
		g_TitleMode = g_TitleNextMode;
		g_TitleNextMode = -1;
		g_TitleFastForward = false;

		switch (g_TitleMode) {
		case TITLEMODE_LEGAL:
			titleInitLegal();
			break;
		case TITLEMODE_PDLOGO:
			titleInitPdLogo();
			break;
		case TITLEMODE_NINTENDOLOGO:
			titleInitNintendoLogo();
			break;
		case TITLEMODE_RARELOGO:
			titleInitRareLogo();
			break;
		case TITLEMODE_SKIP:
			titleInitSkip();
			break;
		}
	}

	// Run the current mode's tick function
	switch (g_TitleMode) {
	case TITLEMODE_LEGAL:
		titleTickLegal();
		break;
	case TITLEMODE_PDLOGO:
		titleTickPdLogo();
		break;
	case TITLEMODE_NINTENDOLOGO:
		titleTickNintendoLogo();
		break;
	case TITLEMODE_RARELOGO:
		titleTickRareLogo();
		break;
	case TITLEMODE_SKIP:
		viSetUseZBuf(false);
		titleSetNextMode(TITLEMODE_RARELOGO);
		break;
	}
}

bool titleIsChangingMode(void)
{
	return g_TitleNextMode >= 0;
}

bool titleIsKeepingMode(void)
{
	if (g_TitleNextMode >= 0) {
		return false;
	}

	if (g_TitleMode == -1 || g_TitleMode == TITLEMODE_SKIP) {
		return false;
	}

	return true;
}

void titleExit(void)
{
	switch (g_TitleMode) {
	case TITLEMODE_LEGAL:
		break;
	case TITLEMODE_PDLOGO:
		titleExitPdLogo();
		break;
	case TITLEMODE_NINTENDOLOGO:
		titleExitNintendoLogo();
		break;
	case TITLEMODE_RARELOGO:
		titleExitRareLogo();
		break;
	}

	g_TitleNextMode = -1;
	g_TitleMode = -1;
}

void titleInitFromAiCmd(unsigned int value)
{
	switch (value) {
	case TITLEAIMODE_RARELOGO:
		g_TitleMode = TITLEMODE_RARELOGO;
		titleInitRareLogo();
		break;
	case TITLEAIMODE_NINTENDOLOGO:
		g_TitleMode = TITLEMODE_NINTENDOLOGO;
		titleInitNintendoLogo();
		break;
	case TITLEAIMODE_PDLOGO:
		g_TitleMode = TITLEMODE_PDLOGO;
		titleInitPdLogo();
		break;
	}

	g_TitleNextMode = -1;
}

Gfx *titleRender(Gfx *gdl)
{
	if (g_TitleDelayedTimer == 0) {
		switch (g_TitleMode) {
		case TITLEMODE_LEGAL:
			gdl = titleRenderLegal(gdl);
			break;
		case TITLEMODE_PDLOGO:
			gdl = titleRenderPdLogo(gdl);
			break;
		case TITLEMODE_NINTENDOLOGO:
			gdl = titleRenderNintendoLogo(gdl);
			break;
		case TITLEMODE_RARELOGO:
			gdl = titleRenderRareLogo(gdl);
			break;
		}
	}

	return gdl;
}

void titleSetNextStage(int stagenum)
{
	g_TitleNextStage = stagenum;
}

