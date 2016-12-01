#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "math.h"
#include "source.h"
#include "vector.h"
#include "log.h"
#include "sdss-survey.h"
#include "sconfig.h"

Source* source_new(const ShearConfig* config) {
    Source* src = calloc(1,sizeof(Source));
    if (src == NULL) {
        wlog("Could not allocate Source\n");
        exit(EXIT_FAILURE);
    }

    src->shear_style = config->shear_style;
    src->mask_style = config->mask_style;
    src->scstyle = config->scstyle;
    src->sourceid_style = config->sourceid_style;

    if (src->scstyle == SIGMACRIT_STYLE_INTERP) {
        src->scinv = dvector_zeros(config->nzl);

        if (src->scinv == NULL) {
            wlog("Could not allocate %ld scinv for source\n", config->nzl);
            exit(EXIT_FAILURE);
        }
    }

    return src;
}

// use like this:
//   source = source_free(source);
Source* source_free(Source* src) {

    if (src != NULL) {

        if (src->scstyle == SIGMACRIT_STYLE_INTERP) {
            if (src->scinv != NULL) {
                vector_free(src->scinv);
            }
        }

        free(src);
    }
    return NULL;
}

// source must already be allocated
int source_read(FILE* stream, Source* src) {
    size_t i=0;
    double ra_rad,dec_rad;
    int nread=0;
    int nexpect=0;

    if(src->sourceid_style==SOURCEID_STYLE_INDEX) {
    	nread += fscanf(stream, "%ld %lf %lf %lf %lf", 
                    &src->index,
                    &src->ra, &src->dec,
                    &src->g1, &src->g2);
    	nexpect += 5;
    } else {
    	nread += fscanf(stream, "%lf %lf %lf %lf",
                    &src->ra, &src->dec,
                    &src->g1, &src->g2);
    	src->index=0;
    	nexpect += 4;    
    }

    if (src->shear_style==SHEAR_STYLE_LENSFIT) {
        nread += fscanf(stream, "%lf %lf", &src->g1sens, &src->g2sens);
        nexpect += 2;
    }

    nread += fscanf(stream, "%lf", &src->weight);

    nexpect += 1;

    if (src->scstyle == SIGMACRIT_STYLE_INTERP) {
        nexpect += src->scinv->size;
        for (i=0; i<src->scinv->size; i++) {
            nread += fscanf(stream,"%lf", &src->scinv->data[i]);
        }
    } else {
        nread += fscanf(stream,"%lf", &src->z);
        nexpect += 1;
    }

    if (src->scstyle == SIGMACRIT_STYLE_SAMPLE) {
        nread += fscanf(stream,"%lf", &src->zs);
        nexpect += 1;
    }

    ra_rad = src->ra*D2R;
    dec_rad = src->dec*D2R;
    src->sinra = sin(ra_rad);
    src->cosra = cos(ra_rad);
    src->sindec = sin(dec_rad);
    src->cosdec = cos(dec_rad);

    // add sin(lam),cos(lam),sin(eta),cos(eta)
    if (src->mask_style == MASK_STYLE_SDSS) {
        eq2sdss_sincos(src->ra,src->dec,
                &src->sinlam, &src->coslam,
                &src->sineta, &src->coseta);
    }

    return (nread == nexpect);
}

void source_print(Source* src) {
    wlog("    index:  %ld\n", src->index);
    wlog("    ra:     %lf\n", src->ra);
    wlog("    dec:    %lf\n", src->dec);
    wlog("    g1:     %lf\n", src->g1);
    wlog("    g2:     %lf\n", src->g2);
    if (src->shear_style==SHEAR_STYLE_LENSFIT) {
        wlog("    g1sens: %lf\n", src->g1sens);
        wlog("    g2sens: %lf\n", src->g2sens);
    }
    wlog("    weight: %lf\n", src->weight);
    wlog("    hpixid: %ld\n", src->hpixid);

    if (src->mask_style == MASK_STYLE_SDSS) {
        wlog("    sinlam: %lf\n", src->sinlam);
        wlog("    coslam: %lf\n", src->coslam);
        wlog("    sineta: %lf\n", src->sineta);
        wlog("    coseta: %lf\n", src->coseta);
    }

    if (src->scstyle == SIGMACRIT_STYLE_POINT) {
        wlog("    z:      %lf\n", src->z);
        wlog("    dc:     %lf\n", src->dc);
    } else if (src->scstyle == SIGMACRIT_STYLE_SAMPLE) {
        wlog("    z:      %lf\n", src->z);
        wlog("    dc:     %lf\n", src->dc);
        wlog("    zs:      %lf\n", src->zs);
        wlog("    dcs:     %lf\n", src->dcs);
    } else {
        size_t nzl = src->zlens->size;
        wlog("    zlens[0]: %lf  szinv[0]: %e\n", 
                src->zlens->data[0], src->scinv->data[0]);
        wlog("    zlens[%ld]: %lf  szinv[%ld]: %e\n", 
                nzl-1, src->zlens->data[nzl-1], nzl-1, src->scinv->data[nzl-1]);
    }
}


