#include "quad.h"

/*
   This must agree with the code in healpix_util

   position angle in degrees [-180,180]

   quadrant 1,2,3,4
*/
int quadeq_get_quadrant(double pa_degrees)
{
    int quadrant;
    if (pa_degrees < -90.) {
        quadrant=3;
    } else if (pa_degrees < 0.0) {
        quadrant=4;
    } else if (pa_degrees < 90.) {
        quadrant=1;
    } else {
        quadrant=2;
    }

    return quadrant;
}

/*

  If all quadrants are not ok, we check pairs. But which pairs to check in
  what order?  
 
  I'm checking 12 then 23 then 34 then 41.  For the same lens, I'll always end
  up with the same unmasked pair that I'm checking, so that's consistent.  But
  I might want to choose another order that would better reduce systematics.

*/


int quadeq_check_quadrant(int64 maskflags, int quadrant)
{
	static const int OK=1, BAD=0;

    int allcheck = maskflags & QUADEQ_ALL_OK;
    if (allcheck == 0) {
        return BAD;
    }
    if (allcheck == QUADEQ_ALL_OK) {
        return OK;
    }

    // work with the first good adjacent pair we find.  We already checked the
    // case where all quadrants are OK, so it is now arbitrary which one we
    // check against first, but we should be consistent for a given lens.
    // Might figure out which gives better systematics?
    //
    // It would *not* be right to try another pair if the first good one
    // fails; we need to always check on for a given lens to make sure
    // we only draw pairs from that one.
 
    if ((maskflags & QUADEQ_12_OK) == QUADEQ_12_OK) {
        if ( quadrant==1 || quadrant==2) {
            return OK;
        } else {
            return BAD;
        }
    }
    if ((maskflags & QUADEQ_23_OK) == QUADEQ_23_OK) {
        if ( quadrant==2 || quadrant==3) {
            return OK;
        } else {
            return BAD;
        }
    }
    if ((maskflags & QUADEQ_34_OK) == QUADEQ_34_OK) {
        if ( quadrant==3 || quadrant==4) {
            return OK;
        } else {
            return BAD;
        }
    }
    if ((maskflags & QUADEQ_41_OK) == QUADEQ_41_OK) {
        if ( quadrant==4 || quadrant==1) {
            return OK;
        } else {
            return BAD;
        }
    }

    // if we get here we didn't match a good pair of quadrants
    return BAD;

}
