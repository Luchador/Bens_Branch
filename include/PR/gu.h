#ifndef _GU_H_
#define _GU_H_

/**************************************************************************
 *									  *
 *		 Copyright (C) 1994, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/

/**************************************************************************
 *
 *  $Revision: 1.46 $
 *  $Date: 1997/11/26 00:30:53 $
 *  $Source: /hosts/gate3/exdisk2/cvs/N64OS/Master/cvsmdev2/PR/include/gu.h,v $
 *
 **************************************************************************/

#include <PR/mbi.h>
#include <stdint.h>
#include <PR/sptask.h>

#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif
#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif

#define	FTOFIX32(x)	(int)((x) * (float)0x00010000)
#define	FTOFRAC8(x)	((int) MIN(((x) * (128.0f)), 127.0f) & 0xff)

/*
 * matrix operations:
 *
 * The 'F' version is floating point, in case the application wants
 * to do matrix manipulations and convert to fixed-point at the last
 * minute.
 */
extern void guMtxIdent(Mtx *m);
extern void guMtxIdentF(float mf[4][4]);
extern void guFrustum(Mtx *m, float l, float r, float b, float t,
		      float n, float f, float scale);
extern void guFrustumF(float mf[4][4], float l, float r, float b, float t,
		       float n, float f, float scale);
extern void guLookAtReflect(Mtx *m, LookAt *l,
			float xEye, float yEye, float zEye,
			float xAt,  float yAt,  float zAt,
			float xUp,  float yUp,  float zUp);
extern void guLookAtReflectF(float mf[4][4], LookAt *l,
		      float xEye, float yEye, float zEye,
		      float xAt,  float yAt,  float zAt,
		      float xUp,  float yUp,  float zUp);
extern void guLookAtHilite(Mtx *m, LookAt *l, Hilite *h,
                float xEye, float yEye, float zEye,
                float xAt,  float yAt,  float zAt,
                float xUp,  float yUp,  float zUp,
                float xl1,  float yl1,  float zl1,
                float xl2,  float yl2,  float zl2,
		int   twidth, int theight);
extern void guLookAtHiliteF(float mf[4][4], LookAt *l, Hilite *h,
		float xEye, float yEye, float zEye,
		float xAt,  float yAt,  float zAt,
		float xUp,  float yUp,  float zUp,
		float xl1,  float yl1,  float zl1,
		float xl2,  float yl2,  float zl2,
		int twidth, int theight);
extern void guLookAtStereo(Mtx *m,
			float xEye, float yEye, float zEye,
			float xAt,  float yAt,  float zAt,
			float xUp,  float yUp,  float zUp,
			float eyedist);
extern void guLookAtStereoF(float mf[4][4],
		      	float xEye, float yEye, float zEye,
		      	float xAt,  float yAt,  float zAt,
		      	float xUp,  float yUp,  float zUp,
			float eyedist);
extern void guRotate(Mtx *m, float a, float x, float y, float z);
extern void guRotateF(float mf[4][4], float a, float x, float y, float z);
extern void guRotateRPY(Mtx *m, float r, float p, float y);
extern void guRotateRPYF(float mf[4][4], float r, float p, float h);
extern void guAlignF(float mf[4][4], float a, float x, float y, float z);
extern void guScale(Mtx *m, float x, float y, float z);
extern void guScaleF(float mf[4][4], float x, float y, float z);
extern void guPosition(Mtx *m, float r, float p, float h, float s,
		       float x, float y, float z);
extern void guPositionF(float mf[4][4], float r, float p, float h, float s,
			float x, float y, float z);
extern void guMtxF2L(float mf[4][4], Mtx *m);
extern void guMtxCatF(float m[4][4], float n[4][4], float r[4][4]);
extern void guMtxCatL(Mtx *m, Mtx *n, Mtx *res);
extern void guMtxXFMF(float mf[4][4], float x, float y, float z,
		      float *ox, float *oy, float *oz);
extern void guMtxXFML(Mtx *m, float x, float y, float z,
		      float *ox, float *oy, float *oz);

/*
 *  Math functions
 */

extern signed short sins (unsigned short angle);
extern signed short coss (unsigned short angle);

#endif /* !_GU_H_ */
