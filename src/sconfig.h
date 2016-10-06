#ifndef _SCONFIG_HEADER
#define _SCONFIG_HEADER

#include "defs.h"
#include "vector.h"
#include "config.h"

#define CONFIG_KEYSZ 50

typedef struct {
    double H0;
    double omega_m;

    int64 healpix_nside;

    int shear_style;
    int mask_style;
    int scstyle;

    // if the distance to the lens is input
    int Dlens_input;

    // will be zero unless scstyle==SIGMACRIT_STYLE_INTERP
    int64 nzl;
    // only fill in for scstyle==SIGMACRIT_STYLE_INTERP
    dvector* zl;  

    int64 nbin;

    int r_units; // units for radius
    int shear_units; // units for shear, deltasig or shear

    double rmin; // mpc/h
    double rmax;

    // we fill these in
    double log_rmin;
    double log_rmax;
    double log_binsize;

    // optional parameters

    // min z lens to allow instead
    // of the full interpolation range
    double min_zlens_interp;

    // can demand zs > zl + zdiff_min when using source
    // z as truth
    double zdiff_min;

} ShearConfig;

ShearConfig* sconfig_read(const char* url);

ShearConfig* sconfig_free(ShearConfig* config);
void sconfig_print(ShearConfig* config);

#endif
