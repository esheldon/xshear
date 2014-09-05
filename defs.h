#ifndef _MYDEFS_H
#define _MYDEFS_H

#include <math.h>
#include <stdint.h>

// these are no longer in math.h for the c99 standard, so 
// define these if not already defined

#ifndef M_PI
# define M_E		2.7182818284590452354	/* e */
# define M_LOG2E	1.4426950408889634074	/* log_2 e */
# define M_LOG10E	0.43429448190325182765	/* log_10 e */
# define M_LN2		0.69314718055994530942	/* log_e 2 */
# define M_LN10		2.30258509299404568402	/* log_e 10 */
# define M_PI		3.14159265358979323846	/* pi */
# define M_PI_2		1.57079632679489661923	/* pi/2 */
# define M_PI_4		0.78539816339744830962	/* pi/4 */
# define M_1_PI		0.31830988618379067154	/* 1/pi */
# define M_2_PI		0.63661977236758134308	/* 2/pi */
# define M_2_SQRTPI	1.12837916709551257390	/* 2/sqrt(pi) */
# define M_SQRT2	1.41421356237309504880	/* sqrt(2) */
# define M_SQRT1_2	0.70710678118654752440	/* 1/sqrt(2) */

#endif

# define M_TWO_PI   6.28318530717958647693 /* 2*pi */
# define M_TWOTHIRD 0.66666666666666666666

#define D2R  0.017453292519943295
#define R2D  57.295779513082323

typedef int64_t int64;

#define MASK_STYLE_NONE 1
#define MASK_STYLE_SDSS 2
#define MASK_STYLE_EQ 3

#define SCSTYLE_TRUE 1
#define SCSTYLE_INTERP 2
#define LENSPERDOT 1000
#define LENSPERCHUNK 100000
//#define LENSPERCHUNK 3000

// move this to config file?
#define SHAPENOISE2 0.1024
// (shape noise/2)**2  to give noise on gamma
#define GSN2 0.0256

// we now do this with optional config parameter
// min lens redshift to consider
//#define MIN_ZLENS 0.05

#define UNITS_MPC 1
#define UNITS_ARCMIN 2

#endif
