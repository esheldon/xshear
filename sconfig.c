#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "sconfig.h"
#include "Vector.h"
#include "log.h"
#include "defs.h"
#include "config.h"

#ifdef HDFS
#include "hdfs_lines.h"
#endif


struct sconfig* sconfig_read(const char* url) {

#ifdef HDFS
    // if compiled with hdfs, and begins with hdfs:// then read as an hdfs file
    if (is_in_hdfs(url)) {
        return hdfs_config_read(url);
    }
#endif

    wlog("Reading config from %s\n", url);

    enum cfg_status status=0, ostatus=0;
    struct cfg *cfg=cfg_read(url, &status);
    if (status) {
        fprintf(stderr,"Config Error: %s\n", cfg_status_string(status));
        exit(1);
    }


    struct sconfig* c=calloc(1, sizeof(struct sconfig));
    char key[CONFIG_KEYSZ];

    // set defaults
    c->mag_min=0;
    c->mag_max=9999;
    c->R_min = 0;
    c->R_max = 1;
    c->r_units=UNITS_MPC;

    c->min_zlens_interp=0;

    // this strcpy business is so we can print error messages
    // below
    c->H0 = cfg_get_double(cfg,strcpy(key,"H0"),&status);
    if (status) goto _sconfig_read_bail;

    c->omega_m = cfg_get_double(cfg,strcpy(key,"omega_m"),&status);
    if (status) goto _sconfig_read_bail;

    c->npts = (int64) cfg_get_long(cfg,strcpy(key,"npts"),&status);
    if (status) goto _sconfig_read_bail;

    c->nside = (int64) cfg_get_long(cfg,strcpy(key,"nside"),&status);
    if (status) goto _sconfig_read_bail;

    c->mask_style = (int64) cfg_get_long(cfg,strcpy(key,"mask_style"),&status);
    if (status) goto _sconfig_read_bail;

    c->scstyle = (int64) cfg_get_long(cfg,strcpy(key,"sigmacrit_style"),&status);
    if (status) goto _sconfig_read_bail;

    c->nbin = (int64) cfg_get_long(cfg,strcpy(key,"nbin"),&status);
    if (status) goto _sconfig_read_bail;

    c->rmin = cfg_get_double(cfg,strcpy(key,"rmin"),&status);
    if (status) goto _sconfig_read_bail;

    c->rmax = cfg_get_double(cfg,strcpy(key,"rmax"),&status);
    if (status) goto _sconfig_read_bail;

    if (c->scstyle == SCSTYLE_INTERP) {
        c->zl = f64vector_new(0);

        c->zl->data=cfg_get_dblarr(cfg, strcpy(key,"zlvals"), &c->zl->size, &status);
        if (status) goto _sconfig_read_bail;

        c->nzl = (int64) c->zl->size;

    }

    // optional pars
    size_t sz=0;
    double *mag_range = cfg_get_dblarr(cfg,"mag_range",&sz,&ostatus);
    if (!ostatus) {
        c->mag_min=mag_range[0];
        c->mag_max=mag_range[1];
        free(mag_range);
    }
    double *R_range = cfg_get_dblarr(cfg,"R_range",&sz,&ostatus);
    if (!ostatus) {
        c->R_min = R_range[0];
        c->R_max = R_range[1];
        free(R_range);
    }

    double mzl = cfg_get_double(cfg, "min_zlens_interp", &ostatus);
    if (!ostatus) {
        c->min_zlens_interp=mzl;
    }

    int r_units = (int) cfg_get_long(cfg, "r_units", &ostatus);
    if (!ostatus) {
        c->r_units=r_units;
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

/*
struct sconfig* sconfig_read_old(const char* url) {

#ifdef HDFS
    // if compiled with hdfs, and begins with hdfs:// then read as an hdfs file
    if (is_in_hdfs(url)) {
        return hdfs_config_read(url);
    }
#endif

    wlog("Reading config from %s\n", url);

    FILE* stream=fopen(url,"r");
    if (stream==NULL) {
        wlog("Could not open url: %s\n", url);
        exit(EXIT_FAILURE);
    }

    struct sconfig* c=calloc(1, sizeof(struct sconfig));
    c->zl=NULL;

    char key[CONFIG_KEYSZ];
    fscanf(stream, "%s %lf", key, &c->H0);
    fscanf(stream, "%s %lf", key, &c->omega_m);
    fscanf(stream, "%s %ld", key, &c->npts);
    fscanf(stream, "%s %ld", key, &c->nside);
    fscanf(stream, "%s %ld", key, &c->mask_style);
    fscanf(stream, "%s %ld", key, &c->scstyle);
    fscanf(stream, "%s %ld", key, &c->nbin);
    fscanf(stream, "%s %lf", key, &c->rmin);
    fscanf(stream, "%s %lf", key, &c->rmax);
    if (c->scstyle == SCSTYLE_INTERP) {
        size_t i;
        fscanf(stream, "%s %lu", key, &c->nzl);
        c->zl = f64vector_new(c->nzl);
        // this is the zlvals keyword
        fscanf(stream," %s ", key);
        for (i=0; i<c->zl->size; i++) {
            fscanf(stream, "%lf", &c->zl->data[i]);
        }

        // optional parameter for min z to use
        if (1 == fscanf(stream, "%s", key)) {
           if (0 == strcmp(key,"min_zlens_interp")) {
               fscanf(stream, "%lf", &c->min_zlens_interp);
           }
        }
    }

    c->log_rmin = log10(c->rmin);
    c->log_rmax = log10(c->rmax);
    c->log_binsize = (c->log_rmax - c->log_rmin)/c->nbin;

    fclose(stream);

    return c;
}
*/
#ifdef HDFS

struct sconfig* hdfs_config_read(const char* url) {

    hdfsFS fs;
    size_t lbsz=255;
    char* lbuf=calloc(lbsz, sizeof(char));

    struct sconfig* c=calloc(1, sizeof(struct sconfig));
    fs = hdfs_connect();

    wlog("Reading config from %s\n", url);

    tSize file_buffsize=1024;
    hdfsFile hf = hdfs_open(fs, url, O_RDONLY, file_buffsize);


    c->zl=NULL;

    char key[CONFIG_KEYSZ];

    hdfs_getline(hf, &lbuf, &lbsz); sscanf(lbuf, "%s %lf", key, &c->H0);
    hdfs_getline(hf, &lbuf, &lbsz); sscanf(lbuf, "%s %lf", key, &c->omega_m);
    hdfs_getline(hf, &lbuf, &lbsz); sscanf(lbuf, "%s %ld", key, &c->npts);
    hdfs_getline(hf, &lbuf, &lbsz); sscanf(lbuf, "%s %ld", key, &c->nside);
    hdfs_getline(hf, &lbuf, &lbsz); sscanf(lbuf, "%s %ld", key, &c->mask_style);
    hdfs_getline(hf, &lbuf, &lbsz); sscanf(lbuf, "%s %ld", key, &c->scstyle);
    hdfs_getline(hf, &lbuf, &lbsz); sscanf(lbuf, "%s %ld", key, &c->nbin);
    hdfs_getline(hf, &lbuf, &lbsz); sscanf(lbuf, "%s %lf", key, &c->rmin);
    hdfs_getline(hf, &lbuf, &lbsz); sscanf(lbuf, "%s %lf", key, &c->rmax);

    if (c->scstyle == 2) {
        int nread=0;
        char* lptr;

        hdfs_getline(hf, &lbuf, &lbsz); sscanf(lbuf, "%s %lu", key, &c->nzl);
        c->zl = f64vector_new(c->nzl);

        // this is the zlvals keyword
        // note space, that tells it to skip white space
        // reading until tab space or newline (although
        // we don't expect keywords alone on a line)
        hdfs_getline(hf, &lbuf, &lbsz);
        lptr = lbuf;
        sscanf(lptr, " %49[^\t \n]%n", key, &nread);

        // after each read, skip what we read plus delimiter
        lptr += nread+1;

        for (size_t i=0; i<c->zl->size; i++) {
            sscanf(lptr, "%lf%n", &c->zl->data[i], &nread);
            lptr += nread+1;
        }
    }

    c->log_rmin = log10(c->rmin);
    c->log_rmax = log10(c->rmax);
    c->log_binsize = (c->log_rmax - c->log_rmin)/c->nbin;

    hdfsCloseFile(fs, hf);
    hdfsDisconnect(fs);

    free(lbuf);

    return c;
}


#endif



// usage:  config=config_delete(config);
struct sconfig* sconfig_delete(struct sconfig* self) {
    if (self != NULL) {
        free(self->zl);
    }
    free(self);
    return NULL;
}

void sconfig_print(struct sconfig* c) {
    wlog("    H0:           %lf\n", c->H0);
    wlog("    omega_m:      %lf\n", c->omega_m);
    wlog("    npts:         %ld\n", c->npts);
    wlog("    nside:        %ld\n", c->nside);
    wlog("    mask style:   %ld\n", c->mask_style);
    wlog("    scrit style:  %ld\n", c->scstyle);
    wlog("    mag_min:      %lf\n", c->mag_min);
    wlog("    mag_max:      %lf\n", c->mag_max);
    wlog("    R_min:         %lf\n", c->R_min);
    wlog("    R_max:         %lf\n", c->R_max);
    wlog("    nbin:         %ld\n", c->nbin);
    wlog("    rmin:         %lf\n", c->rmin);
    wlog("    rmax:         %lf\n", c->rmax);
    wlog("    log(rmin):    %lf\n", c->log_rmin);
    wlog("    log(rmax):    %lf\n", c->log_rmax);
    wlog("    log(binsize): %lf\n", c->log_binsize);
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
