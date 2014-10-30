#ifndef _SCONFIG_HEADER
#define _SCONFIG_HEADER

#include "defs.h"
#include "Vector.h"
#include "config.h"

#define CONFIG_KEYSZ 50

struct sconfig {
    double H0;
    double omega_m;

    int64 healpix_nside;

    int shear_style;
    int mask_style;
    int scstyle;

    // will be zero unless scstyle==SIGMACRIT_STYLE_INTERP
    int64 nzl;
    // only fill in for scstyle==SIGMACRIT_STYLE_INTERP
    struct f64vector* zl;  

    int64 nbin;
    int r_units; // units for radius
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

};

struct sconfig* sconfig_read(const char* url);

struct sconfig* sconfig_delete(struct sconfig* config);
void sconfig_print(struct sconfig* config);

int get_mask_style(struct cfg *cfg);
int get_shear_style(struct cfg *cfg);
int get_scstyle(struct cfg *cfg);

#endif
