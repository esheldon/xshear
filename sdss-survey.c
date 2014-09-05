#include <math.h>
#include "sdss-survey.h"
#include "defs.h"

double lon_bound(double lon, double minval, double maxval) {
    while (lon < minval) {
        lon += 360.;
    }

    while (lon > maxval) {
        lon -= 360.;
    }
    return lon;
}

/*
 * ra,dec input in degrees
 * lam,eta output in degrees
 */
void eq2sdss(double ra, double dec, double* lam, double* eta) {

    // put everything in radians first
    ra *= D2R;
    dec *= D2R;

    // this is the SDSS node
    ra -= 1.6580627893946132;

    double cdec = cos(dec);
    
    double x = cos(ra)*cdec;
    double y = sin(ra)*cdec;
    double z = sin(dec);

    *lam = -asin(x);
    *eta = atan2(z, y);

    // this is the eta pole
    *eta -= 0.56723200689815712;

    *eta *= R2D;
    *lam *= R2D;

    *eta = lon_bound(*eta, -180., 180.);
}

void eq2sdss_sincos(double ra, double dec, 
                    double* sinlam, double* coslam, 
                    double* sineta, double* coseta) {
    double lam,eta;
    eq2sdss(ra, dec, &lam, &eta);

    lam *= D2R;
    eta *= D2R;
    *sinlam = sin(lam);
    *coslam = cos(lam);
    *sineta = sin(eta);
    *coseta = cos(eta);
}

/*
 * The returned angle theta works properly with how I transformed from
 * pixels to lambda,eta in the SDSS
 */
void gcirc_survey(
        double lam1, double eta1, 
        double lam2, double eta2,
        double* dis,  double* theta)
{

  double tlam1 = lam1*D2R;
  //sinlam1 = sin(tlam1);
  double coslam1 = cos(tlam1);
  double sinlam1 = sin(tlam1);

  double tlam2 = lam2*D2R;
  double coslam2 = cos(tlam2);
  double sinlam2 = sin(tlam2);

  double etadiff = (eta2-eta1)*D2R;
  double cosetadiff = cos(etadiff);
  double sinetadiff = sin(etadiff);

  double cosdis = sinlam1*sinlam2 + coslam1*coslam2*cosetadiff;

  if (cosdis < -1.0) cosdis=-1.0;
  if (cosdis >  1.0) cosdis= 1.0;

  *dis = acos(cosdis);

  *theta = atan2( sinetadiff, sinlam1*cosetadiff - coslam1*sinlam2/coslam2 );

  *theta -= M_PI_2;
}

/*
 * Get the quadrant of the second object relative to the first
 */
double posangle_survey_sincos(double sinlam1, double coslam1,
                              double sineta1, double coseta1,
                              double sinlam2, double coslam2,
                              double sineta2, double coseta2) {

    double cosetadiff = coseta2*coseta1 + sineta2*sineta1;
    double sinetadiff = sineta2*coseta1 - coseta2*sineta2;

    double theta = atan2( sinetadiff, sinlam1*cosetadiff - coslam1*sinlam2/coslam2 );
    theta -= M_PI_2;

    return theta;
}

/*
 * NOTE: this converts theta to (p/2 - theta)*D2R from gcirc_survey as used in
 * stomp
 */

int survey_quad(double theta) {
    theta = (M_PI_2 - theta)*R2D;
    if (theta >= 0. && theta < 90.) {
        return 0;
    } else if (theta >= 90. && theta < 180.) {
        return 1;
    } else if (theta >= 180. && theta < 270.) {
        return 2;
    } else {
        return 3;
    }
}


/*
 * If all quadrants are not ok, we check pairs. But which pairs to check in
 * what order?  
 *
 * I'm checking 12 then 23 then 34 then 41.  For the same lens, I'll always end
 * up with the same unmasked pair that I'm checking, so that's consistent.  But
 * I might want to choose another order that would better reduce systematics.
 */

int test_quad_sincos_sdss(int64 maskflags,
                          double sinlam1, double coslam1,
                          double sineta1, double coseta1,
                          double sinlam2, double coslam2,
                          double sineta2, double coseta2) {

	static const int OK=1, BAD=0;
    // none of the quadrants are ok
    int allcheck = maskflags & SDSS_QUADALL_OK;
    if (allcheck == 0) {
        return BAD;
    }
    // all quadrants are OK
    if ( allcheck == SDSS_QUADALL_OK) {
        return OK;
    }

    // first determine which quandrant the source is in

    double theta = posangle_survey_sincos(sinlam1, coslam1,
                                          sineta1, coseta1,
                                          sinlam2, coslam2,
                                          sineta2, coseta2);
    // note quadrant is in [0,3] to be consistent with stomp
    int quadrant = survey_quad(theta);

    // this we can and with the bits
    int quadbit = 1 << (quadrant+1);

    // work with the first good adjacent pair we find.  We already checked the
    // case where all quadrants are OK, so it is now arbitrary which one we
    // check against first, but we should be consistent for a given lens.
    // Might figure out which gives better systematics?
    //
    // It would *not* be right to try another pair if the first good one
    // fails; we need to always check on for a given lens to make sure
    // we only draw pairs from that one.
    
    if ((maskflags & SDSS_QUAD12_OK) == SDSS_QUAD12_OK) {
        if ((quadbit & SDSS_QUAD12_OK) != 0) {
            return OK;
        } else {
            return BAD;
        }
    }
    if ((maskflags & SDSS_QUAD34_OK) == SDSS_QUAD34_OK) {
        if ((quadbit & SDSS_QUAD34_OK) != 0) {
            return OK;
        } else {
            return BAD;
        }
    }
    if ((maskflags & SDSS_QUAD23_OK) == SDSS_QUAD23_OK) {
        if ((quadbit & SDSS_QUAD23_OK) != 0) {
            return OK;
        } else {
            return BAD;
        }
    }
    if ((maskflags & SDSS_QUAD41_OK) == SDSS_QUAD41_OK) {
        if ((quadbit & SDSS_QUAD41_OK) != 0) {
            return OK;
        } else {
            return BAD;
        }
    }


    // if we get here we didn't match a good pair of quadrants
    return BAD;

}
