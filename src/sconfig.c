#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "sconfig.h"
#include "vector.h"
#include "log.h"
#include "defs.h"
#include "config.h"

static int do_strncmp(char *s1, char *s2)
{
    int l1=strlen(s1);
    int l2=strlen(s2);
    int ncmp=min(l1,l2);

    return strncmp(s1, s2, ncmp);
}


static int get_mask_style(struct cfg *cfg) {
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

static int get_shear_style(struct cfg *cfg) {
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

static int get_scstyle(struct cfg *cfg) {
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
    } else if (0 == do_strncmp(mstr,SIGMACRIT_STYLE_SAMPLE_STR)) {
        scstyle=SIGMACRIT_STYLE_SAMPLE;
    } else {
        fprintf(stderr, "Config Error: bad sigmacrit_style '%s'\n", mstr);
        exit(1);
    }

    free(mstr);

    return scstyle;
}

static int get_sourceid_style(struct cfg *cfg) {
    enum cfg_status status=0;
    int sourceid_style=0;

    char *mstr = cfg_get_string(cfg,"sourceid_style", &status);
    if (status) {
        // tacitly assume the user wants SOURCEID_STYLE_NONE for compatibility
        sourceid_style=SOURCEID_STYLE_NONE;
        return sourceid_style;
    }

    if (0 == do_strncmp(mstr,SOURCEID_STYLE_NONE_STR)) {
        sourceid_style=SOURCEID_STYLE_NONE;
    } else if (0 == do_strncmp(mstr,SOURCEID_STYLE_INDEX_STR)) {
        sourceid_style=SOURCEID_STYLE_INDEX;
    } else {
        fprintf(stderr, "Config Error: bad sourceid_style '%s'\n", mstr);
        exit(1);
    }

    free(mstr);

    return sourceid_style;
}

// defaults to Mpc
static int get_r_units(struct cfg *cfg) {
    enum cfg_status status=0;
    int r_units=0;

    char *mstr = cfg_get_string(cfg,"r_units", &status);
    if (status) {
        wlog("    radius units not sent, defaulting to Mpc\n");
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

// defaults to optimal
static int get_weight_style(struct cfg *cfg) {
    enum cfg_status status=0;
    int weight_style=0;

    char *mstr = cfg_get_string(cfg,"weight_style", &status);
    if (status) {
        wlog("    weight style not sent, defaulting to optimal\n");
        // not sent, default to Mpc
        return WEIGHT_STYLE_OPTIMAL;
    }

    if (0 == do_strncmp(mstr,WEIGHT_STYLE_OPTIMAL_STR)) {
        weight_style=WEIGHT_STYLE_OPTIMAL;
    } else if (0 == do_strncmp(mstr,WEIGHT_STYLE_UNIFORM_STR)) {
        weight_style=WEIGHT_STYLE_UNIFORM;
    } else {
        fprintf(stderr, "Config Error: bad weight_style: '%s'\n", mstr);
        exit(1);
    }

    free(mstr);

    return weight_style;
}


// defaults to false 0
static int get_Dlens_input(struct cfg *cfg) {
    enum cfg_status status=0;
    long Dlens_input=0;

    Dlens_input = cfg_get_long(cfg,"Dlens_input", &status);
    if (status) {
        wlog("    Dlens_input not sent, defaulting to false\n");
        // not sent, default to Mpc
        Dlens_input = 0;
    }

    return Dlens_input;
}



ShearConfig* sconfig_read(const char* url) {

    wlog("Reading config from %s\n", url);

    enum cfg_status status=0, ostatus=0;
    struct cfg *cfg=cfg_read(url, &status);
    if (status) {
        fprintf(stderr,"Config Error: %s\n", cfg_status_string(status));
        exit(1);
    }

    ShearConfig* c=calloc(1, sizeof(ShearConfig));
    char key[CONFIG_KEYSZ];

    // set defaults for optional
    c->healpix_nside=HEALPIX_NSIDE_DEFAULT;

    c->min_zlens_interp=0;
    c->rbin_print_max=0;

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

    c->Dlens_input = get_Dlens_input(cfg);

    c->sourceid_style = get_sourceid_style(cfg);

    c->r_units = get_r_units(cfg);

    c->weight_style = get_weight_style(cfg);

    if (c->Dlens_input && c->scstyle != SIGMACRIT_STYLE_INTERP) {
        fprintf(stderr,"Error: If the lens distance in input \n");
        fprintf(stderr,"       (Dlens_input set in config) the \n");
        fprintf(stderr,"       sigma crit style must be interpolated\n");
        exit(1);
    }
    if (c->scstyle == SIGMACRIT_STYLE_INTERP) {
        c->zl = dvector_new();

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

    int nside = (int64) cfg_get_long(cfg,strcpy(key,"healpix_nside"),&ostatus);
    if (!ostatus) {
        c->healpix_nside = nside;
    }
    
    int rbin_print_max = (int64) cfg_get_long(cfg,strcpy(key,"rbin_print_max"),&ostatus);
    if (!ostatus) {
        c->rbin_print_max = rbin_print_max; 
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

// usage:  config=config_free(config);
ShearConfig* sconfig_free(ShearConfig* self) {
    if (self != NULL) {
        free(self->zl);
    }
    free(self);
    return NULL;
}

void sconfig_print(ShearConfig* c) {
    wlog("    H0:             %lf\n", c->H0);
    wlog("    omega_m:        %lf\n", c->omega_m);
    wlog("    healpix_nside:  %lld\n", c->healpix_nside);
    wlog("    shear style:    %d\n",  c->shear_style);
    wlog("    mask style:     %d\n",  c->mask_style);
    wlog("    weight style:   %d\n",  c->weight_style);
    wlog("    scrit style:    %d\n",  c->scstyle);
    wlog("    sourceid style: %d\n",  c->sourceid_style);
    wlog("    Dlens_input:    %d\n",  c->Dlens_input);
    wlog("    zdiff_min:      %lf\n", c->zdiff_min);
    wlog("    nbin:           %lld\n", c->nbin);
    wlog("    rmin:           %lf\n", c->rmin);
    wlog("    rmax:           %lf\n", c->rmax);
    wlog("    r_units:        %d\n",  c->r_units);
    wlog("    log(rmin):      %lf\n", c->log_rmin);
    wlog("    log(rmax):      %lf\n", c->log_rmax);
    wlog("    log(binsize):   %lf\n", c->log_binsize);
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

void sconfig_open_pair_url(ShearConfig* c, const char* url) {
    c->pair_fd = fopen(url, "w");
    if(!c->pair_fd) {
        fprintf(stderr,"Could not open pair log file %s\n", url);
        exit(1);
    }
}


