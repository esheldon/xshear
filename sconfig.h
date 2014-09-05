#ifndef _SCONFIG_HEADER
#define _SCONFIG_HEADER

#include "defs.h"
#include "Vector.h"

#ifdef HDFS
#include "hdfs_lines.h"
#endif

#define CONFIG_KEYSZ 50

struct sconfig {
    double H0;
    double omega_m;
    int64 npts;  // for cosmo integration

    int64 nside; // hpix

    int64 mask_style;

    int64 scstyle;

    // will be zero unless scstyle==SCSTYLE_INTERP
    int64 nzl;
    // only fill in for scstyle==SCSTYLE_INTERP
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

    double mag_min;
    double mag_max;
    double R_min;
    double R_max;
};

struct sconfig* sconfig_read(const char* url);

#ifdef HDFS
struct sconfig* hdfs_config_read(const char* url);
#endif 

struct sconfig* sconfig_delete(struct sconfig* config);
void sconfig_print(struct sconfig* config);

#endif
