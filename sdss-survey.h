#ifndef _SDSS_SURVEY_H
#define _SDSS_SURVEY_H

#include "defs.h"

#define SDSS_INSIDE_MAP 1
#define SDSS_QUAD1_OK 2
#define SDSS_QUAD2_OK 4
#define SDSS_QUAD3_OK 8
#define SDSS_QUAD4_OK 16
#define SDSS_QUADALL_OK 30

#define SDSS_QUAD12_OK 6
#define SDSS_QUAD23_OK 12
#define SDSS_QUAD34_OK 24
#define SDSS_QUAD41_OK 18

double lon_bound(double lon, double minval, double maxval);
void eq2sdss(double ra, double dec, double* lam, double* eta);
void eq2sdss_sincos(double ra, double dec, 
                    double* sinlam, double* coslam, 
                    double* sineta, double* coseta);

              
void gcirc_survey(
        double lam1, double eta1, 
        double lam2, double eta2,
        double* dis,  double* theta);


double posangle_survey_sincos(double sinlam1, double coslam1,
                              double sineta1, double coseta1,
                              double sinlam2, double coslam2,
                              double sineta2, double coseta2);
int survey_quad(double theta);

int test_quad_sincos_sdss(int64 maskflags,
                          double sinlam1, double coslam1,
                          double sineta1, double coseta1,
                          double sinlam2, double coslam2,
                          double sineta2, double coseta2);



#endif
