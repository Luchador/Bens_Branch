#include <math.h>
#include "constants.h"
#include "game/quaternion.h"
#include "bss.h"
#include "data.h"
#include "types.h"

void quaternionEulerToQuat(struct coord *angle, float quat[4])
{
	float cosx = cosf(angle->f[0] * 0.5f);
	float sinx = sinf(angle->f[0] * 0.5f);
	float cosy = cosf(angle->f[1] * 0.5f);
	float siny = sinf(angle->f[1] * 0.5f);
	float cosz = cosf(angle->f[2] * 0.5f);
	float sinz = sinf(angle->f[2] * 0.5f);

	float cosx_cosy = cosx * cosy;
	float cosx_siny = cosx * siny;
	float sinx_cosy = sinx * cosy;
	float sinx_siny = sinx * siny;

	quat[0] = cosx_cosy * cosz + sinx_siny * sinz;
	quat[1] = sinx_cosy * cosz - cosx_siny * sinz;
	quat[2] = cosx_siny * cosz + sinx_cosy * sinz;
	quat[3] = cosx_cosy * sinz - sinx_siny * cosz;
}

void quaternionSetRotationAroundX(float angle, float quat[4])
{
	quat[0] = cosf(angle * 0.5f);
	quat[1] = sinf(angle * 0.5f);
	quat[2] = 0.0f;
	quat[3] = 0.0f;
}

void quaternionSetRotationAroundY(float angle, float quat[4])
{
	quat[0] = cosf(angle * 0.5f);
	quat[1] = 0.0f;
	quat[2] = sinf(angle * 0.5f);
	quat[3] = 0.0f;
}

void quaternionSetRotationAroundZ(float angle, float quat[4])
{
	quat[0] = cosf(angle * 0.5f);
	quat[1] = 0.0f;
	quat[2] = 0.0f;
	quat[3] = sinf(angle * 0.5f);
}

void quaternionToMtx(float quat[4], Mtx *mtx)
{
	float mult = 2.0f / (quat[0] * quat[0] + quat[1] * quat[1] + quat[2] * quat[2] + quat[3] * quat[3]);
	float a = quat[1] * mult;
	float b = quat[2] * mult;
	float c = quat[3] * mult;

	float sp34 = quat[0] * a;
	float sp30 = quat[0] * b;
	float sp2c = quat[0] * c;
	float sp28 = quat[1] * a;
	float sp24 = quat[1] * b;
	float sp20 = quat[1] * c;
	float sp1c = quat[2] * b;
	float sp18 = quat[2] * c;
	float sp14 = quat[3] * c;

	(*mtx)[0][0] = 1.0f - (sp1c + sp14);
	(*mtx)[0][1] = sp24 + sp2c;
	(*mtx)[0][2] = sp20 - sp30;

	(*mtx)[1][0] = sp24 - sp2c;
	(*mtx)[1][1] = 1.0f - (sp28 + sp14);
	(*mtx)[1][2] = sp18 + sp34;

	(*mtx)[2][0] = sp20 + sp30;
	(*mtx)[2][1] = sp18 - sp34;
	(*mtx)[2][2] = 1.0f - (sp28 + sp1c);

	(*mtx)[3][0] = 0.0f;
	(*mtx)[3][1] = 0.0f;
	(*mtx)[3][2] = 0.0f;

	(*mtx)[0][3] = 0.0f;
	(*mtx)[1][3] = 0.0f;
	(*mtx)[2][3] = 0.0f;
	(*mtx)[3][3] = 1.0f;
}

void quaternion3x3MtxToQuat(Mtx *mtx, float arg1[4])
{
	float var1;
	float var2;
	float trace = (*mtx)[0][0] + (*mtx)[1][1] + (*mtx)[2][2] + 1.0f;

	// If trace is large enough, the matrix is suitable for a simplified fast conversion
	if (trace > 0.01f) {
		var1 = sqrtf(trace);
		var2 = 0.5f / var1;

		arg1[0] = var1 * 0.5f;
		arg1[1] = ((*mtx)[1][2] - (*mtx)[2][1]) * var2;
		arg1[2] = ((*mtx)[2][0] - (*mtx)[0][2]) * var2;
		arg1[3] = ((*mtx)[0][1] - (*mtx)[1][0]) * var2;
	// If trace is too small, the matrix may be near gimbal lock, use an alternative quaternion extraction formula
	} else {
		int i;
		int j;
		int indices[3] = {1, 2, 0};
		int k;

		i = 0;

		if ((*mtx)[0][0] < (*mtx)[1][1]) {
			i = 1;
		}

		if ((*mtx)[i][i] < (*mtx)[2][2]) {
			i = 2;
		}

		j = indices[i];
		k = indices[j];

		var1 = sqrtf((*mtx)[i][i] - ((*mtx)[j][j] + (*mtx)[k][k]) + 1.0f);
		var2 = 0.5f / var1;

		arg1[i + 1] = var1 * 0.5f;
		arg1[    0] = ((*mtx)[j][k] - (*mtx)[k][j]) * var2;
		arg1[j + 1] = ((*mtx)[i][j] + (*mtx)[j][i]) * var2;
		arg1[k + 1] = ((*mtx)[i][k] + (*mtx)[k][i]) * var2;
	}
}

void quaternionToTransformMtx(struct coord *pos, float rot[4], Mtx *mtx)
{
	quaternionToMtx(rot, mtx);

	(*mtx)[3][0] = pos->x;
	(*mtx)[3][1] = pos->y;
	(*mtx)[3][2] = pos->z;
}

#define EPSILON 0.00001001f

void quaternionSlerp(float q1[4], float q2[4], float t, float result[4])
{
	float dot = q1[0] * q2[0] + q1[1] * q2[1] + q1[2] * q2[2] + q1[3] * q2[3];
	float theta;
	float theta_q1;
	float theta_q2;
	float sine;
	float coeff_q1;
	float coeff_q2;

	if (dot < -1.0f + EPSILON) {
		result[0] = (1.0f - t) * q1[0] - q2[0] * t;
		result[1] = (1.0f - t) * q1[1] - q2[1] * t;
		result[2] = (1.0f - t) * q1[2] - q2[2] * t;
		result[3] = (1.0f - t) * q1[3] - q2[3] * t;
	} else if (dot <= 1.0f - EPSILON) {
		theta = acosf(dot);
		theta_q1 = (1.0f - t) * theta;
		theta_q2 = t * theta;
		sine = sinf(theta);
		coeff_q1 = sinf(theta_q1) / sine;
		coeff_q2 = sinf(theta_q2) / sine;
		result[0] = coeff_q1 * q1[0] + q2[0] * coeff_q2;
		result[1] = coeff_q1 * q1[1] + q2[1] * coeff_q2;
		result[2] = coeff_q1 * q1[2] + q2[2] * coeff_q2;
		result[3] = coeff_q1 * q1[3] + q2[3] * coeff_q2;
	} else {
		result[0] = (1.0f - t) * q1[0] + q2[0] * t;
		result[1] = (1.0f - t) * q1[1] + q2[1] * t;
		result[2] = (1.0f - t) * q1[2] + q2[2] * t;
		result[3] = (1.0f - t) * q1[3] + q2[3] * t;
	}
}

/*
This function performs slerp between the identity quaternion [1, 0, 0, 0] and a target quaternion q, storing the interpolated quaternion in result
*/
void quaternionSlerpFromIdentity(float target[4], float t, float result[4])
{
	float sp34 = target[0];
	float sp30 = 1.0f;
	float sp2c;
	float sp28;
	float sp24;
	float sp20;
	float sp1c;
	float sp18;

	if (target[0] < 0.0f) {
		sp34 = -sp34;
		sp30 = -sp30;
	}

	if (sp34 < -0.99998999f) {
		result[0] = target[0] * t - (1.0f - t) * sp30;
		result[1] = target[1] * t;
		result[2] = target[2] * t;
		result[3] = target[3] * t;
	} else if (sp34 <= 0.99998999f) {
		sp2c = acosf(sp34);
		sp28 = t * sp2c;
		sp24 = (1.0f - t) * sp2c;
		sp20 = sinf(sp2c);
		sp1c = sinf(sp28) / sp20;
		sp18 = sinf(sp24) / sp20;

		result[0] = target[0] * sp1c + sp18 * sp30;
		result[1] = target[1] * sp1c;
		result[2] = target[2] * sp1c;
		result[3] = target[3] * sp1c;
	} else {
		result[0] = target[0] * t + (1.0f - t) * sp30;
		result[1] = target[1] * t;
		result[2] = target[2] * t;
		result[3] = target[3] * t;
	}
}

//Prevents the camera from suddenly flipping such as when using the Hoverbike.
void quaternionAvoidFlips(float q1[4], float q2[4])
{
	float dot = q1[0] * q2[0] + q1[1] * q2[1] + q1[2] * q2[2] + q1[3] * q2[3];

	if (dot < 0.0f) {
		q2[0] = -q2[0];
		q2[1] = -q2[1];
		q2[2] = -q2[2];
		q2[3] = -q2[3];
	}
}

void quaternionMultQuaternion(float a[4], float b[4], float result[4])
{
	result[0] = a[0] * b[0] - a[1] * b[1] - a[2] * b[2] - a[3] * b[3];
	result[1] = a[0] * b[1] + b[0] * a[1] + a[2] * b[3] - a[3] * b[2];
	result[2] = a[0] * b[2] + b[0] * a[2] + a[3] * b[1] - a[1] * b[3];
	result[3] = a[0] * b[3] + b[0] * a[3] + a[1] * b[2] - a[2] * b[1];
}

void quaternionMultQuaternionInPlace(float a[4], float dst[4])
{
	float tmp[4];

	quaternionMultQuaternion(a, dst, tmp);

	dst[0] = tmp[0];
	dst[1] = tmp[1];
	dst[2] = tmp[2];
	dst[3] = tmp[3];
}
