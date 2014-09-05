#ifndef _QUAD_H
#define _QUAD_H

#include "defs.h"

#define QUADEQ_INSIDE_MAP 1
#define QUADEQ_12_OK 2
#define QUADEQ_23_OK 4
#define QUADEQ_34_OK 8
#define QUADEQ_41_OK 16
#define QUADEQ_ALL_OK 31

/*
   should mirror quadant def in healpix_util
*/
int quadeq_get_quadrant(double posangle);
int quadeq_check_quadrant(int64 maskflags, int quadrant);

#endif
