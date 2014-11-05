#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "sconfig.h"
#include "Vector.h"
#include "log.h"
#include "defs.h"
#include "config.h"

struct sconfig* sconfig_read(const char* url) {

    wlog("Reading config from %s\n", url);

    enum cfg_status status=0, ostatus=0;
    struct cfg *cfg=cfg_read(url, &status);
    if (status) {
        fprintf(stderr,"Config Error: %s\n", cfg_status_string(status));
        exit(1);
    }

    struct sconfig* c=calloc(1, sizeof(struct sconfig));
    char key[CONFIG_KEYSZ];

    // set defaults for optional
    c->healpix_nside=HEALPIX_NSIDE_DEFAULT;

    c->min_zlens_interp=0;

    // this strcpy business is so we can print error messages
    // below
    c->H0 = cfg_get_double(cfg,strcpy(key,"H0"),&status);
    if (status) goto _sconfig_read_bail;

    c->omega_m = cfg_get_double(cfg,strcpy(key,"omega_m"),&status);
    if (status) goto _sconfig_read_bail;

    c->nbin = (int64) cfg_get_long(cfg,strcpy(key,"nbin"),&status);
    if (status) goto _sconfig_read_bail;

    c->rmin = cfg_get_double(cfg,strcpy(key,"rmin"),&status);
    if (status) goto _sconfig_read_bail;

    c->rmax = cfg_get_double(cfg,strcpy(key,"rmax"),&status);
    if (status) goto _sconfig_read_bail;

    // strings.  Error checking interior to functions
    c->mask_style = get_mask_style(cfg);

    c->shear_style = get_shear_style(cfg);

    c->scstyle = get_scstyle(cfg);

    c->r_units = get_r_units(cfg);

    if (c->scstyle == SIGMACRIT_STYLE_INTERP) {
        c->zl = f64vector_new(0);

        c->zl->data=cfg_get_dblarr(cfg, strcpy(key,"zlvals"), &c->zl->size, &status);
        if (status) goto _sconfig_read_bail;

        c->nzl = (int64) c->zl->size;

    }

    // optional
    double mzl = cfg_get_double(cfg, "min_zlens_interp", &ostatus);
    if (!ostatus) {
        c->min_zlens_interp=mzl;
    }

    double dz = cfg_get_double(cfg, "zdiff_min", &ostatus);
    if (!ostatus) {
        c->zdiff_min=dz;
    }

    int nside = (int64) cfg_get_long(cfg,strcpy(key,"healpix_nside"),&status);
    if (!ostatus) {
        c->healpix_nside = nside;
    }


    c->log_rmin = log10(c->rmin);
    c->log_rmax = log10(c->rmax);
    c->log_binsize = (c->log_rmax - c->log_rmin)/c->nbin;

_sconfig_read_bail:
    if (status) {
        fprintf(stderr,"Config Error for key '%s': %s\n", key,cfg_status_string(status));
        exit(1);
    }

    cfg=cfg_free(cfg);
    return c;
}

// usage:  config=config_delete(config);
struct sconfig* sconfig_delete(struct sconfig* self) {
    if (self != NULL) {
        free(self->zl);
    }
    free(self);
    return NULL;
}

void sconfig_print(struct sconfig* c) {
    wlog("    H0:            %lf\n", c->H0);
    wlog("    omega_m:       %lf\n", c->omega_m);
    wlog("    healpix_nside: %ld\n", c->healpix_nside);
    wlog("    shear style:   %d\n",  c->shear_style);
    wlog("    mask style:    %d\n",  c->mask_style);
    wlog("    scrit style:   %d\n",  c->scstyle);
    wlog("    zdiff_min:     %lf\n", c->zdiff_min);
    wlog("    nbin:          %ld\n", c->nbin);
    wlog("    rmin:          %lf\n", c->rmin);
    wlog("    rmax:          %lf\n", c->rmax);
    wlog("    r_units:       %d\n",  c->r_units);
    wlog("    log(rmin):     %lf\n", c->log_rmin);
    wlog("    log(rmax):     %lf\n", c->log_rmax);
    wlog("    log(binsize):  %lf\n", c->log_binsize);
    if (c->zl != NULL) {
        size_t i;
        wlog("    zlvals[%lu]:", c->zl->size);
        for (i=0; i<c->zl->size; i++) {
            if ((i % 10) == 0) {
                wlog("\n        ");
            }
            wlog("%lf ", c->zl->data[i]);
        }
        wlog("\n");
    }
}

static int do_strncmp(char *s1, char *s2)
{
    int l1=strlen(s1);
    int l2=strlen(s2);
    int ncmp=min(l1,l2);

    return strncmp(s1, s2, ncmp);
}

int get_mask_style(struct cfg *cfg) {
    enum cfg_status status=0;
    int mask_style=0;

    char *mstr = cfg_get_string(cfg,"mask_style", &status);
    if (status) {
        fprintf(stderr,
                "Config Error for key mask_style %s\n",
                cfg_status_string(status));
        exit(1);
    }

    if (0 == do_strncmp(mstr,MASK_STYLE_NONE_STR)) {
        mask_style=MASK_STYLE_NONE;
    } else if (0 == do_strncmp(mstr,MASK_STYLE_SDSS_STR)) {
        mask_style=MASK_STYLE_SDSS;
    } else if (0 == do_strncmp(mstr,MASK_STYLE_EQ_STR)) {
        mask_style=MASK_STYLE_EQ;
    } else {
        fprintf(stderr, "Config Error: bad mask_style '%s'\n", mstr);
        exit(1);
    }

    free(mstr);

    return mask_style;
}

int get_shear_style(struct cfg *cfg) {
    enum cfg_status status=0;
    int shear_style=0;

    char *mstr = cfg_get_string(cfg,"shear_style", &status);
    if (status) {
        fprintf(stderr,
                "Config Error for key shear_style %s\n",
                cfg_status_string(status));
        exit(1);
    }

    if (0 == do_strncmp(mstr,SHEAR_STYLE_REDUCED_STR)) {
        shear_style=SHEAR_STYLE_REDUCED;
    } else if (0 == do_strncmp(mstr,SHEAR_STYLE_LENSFIT_STR)) {
        shear_style=SHEAR_STYLE_LENSFIT;
    } else {
        fprintf(stderr, "Config Error: bad shear_style '%s'\n", mstr);
        exit(1);
    }

    free(mstr);

    return shear_style;
}

int get_scstyle(struct cfg *cfg) {
    enum cfg_status status=0;
    int scstyle=0;

    char *mstr = cfg_get_string(cfg,"sigmacrit_style", &status);
    if (status) {
        fprintf(stderr,
                "Config Error for key sigmacrit_style %s\n",
                cfg_status_string(status));
        exit(1);
    }

    if (0 == do_strncmp(mstr,SIGMACRIT_STYLE_POINT_STR)) {
        scstyle=SIGMACRIT_STYLE_POINT;
    } else if (0 == do_strncmp(mstr,SIGMACRIT_STYLE_INTERP_STR)) {
        scstyle=SIGMACRIT_STYLE_INTERP;
    } else {
        fprintf(stderr, "Config Error: bad sigmacrit_style '%s'\n", mstr);
        exit(1);
    }

    free(mstr);

    return scstyle;
}

// defaults to Mpc
int get_r_units(struct cfg *cfg) {
    enum cfg_status status=0;
    int r_units=0;

    char *mstr = cfg_get_string(cfg,"r_units", &status);
    if (status) {
        wlog("    units not sent, defaulting to Mpc\n");
        // not sent, default to Mpc
        return UNITS_MPC;
    }

    if (0 == do_strncmp(mstr,UNITS_MPC_STR)) {
        r_units=UNITS_MPC;
    } else if (0 == do_strncmp(mstr,UNITS_ARCMIN_STR)) {
        r_units=UNITS_ARCMIN;
    } else {
        fprintf(stderr, "Config Error: bad r_units: '%s'\n", mstr);
        exit(1);
    }

    free(mstr);

    return r_units;
}

