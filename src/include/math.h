#ifndef IN_MATH_H
#define IN_MATH_H

#include_next <math.h>
#undef M_PI
#undef M_TAU
// HACK: for some reason the #include_next above doesn't really do anything, so
float fabsf(float x);
float roundf(float x);

#define M_PI    3.141592741f

#define M_TAU    (M_PI * 2)

#define DEG2RAD(deg)    ((deg) * (M_PI / 180.0f))

#define RAD2DEG(rad) ((rad) * (180.0f / M_PI))
#define RAD2DEG2(rad) ((rad) * 180.0f / M_PI)

#endif
