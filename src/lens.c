#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "math.h"
#include "lens.h"
#include "cosmo.h"
#include "log.h"
#include "healpix.h"
#include "vector.h"
#include "urls.h"
#include "sdss-survey.h"

#include "sconfig.h"

#ifdef HDFS
#include "hdfs_lines.h"
#endif

LensCatalog* lcat_new(size_t n_lens) {

    if (n_lens == 0) {
        wlog("lcat_new: size must be > 0\n");
        exit(EXIT_FAILURE);
    }

    //LensCatalog* lcat = malloc(sizeof(LensCatalog));
    LensCatalog* lcat = calloc(1,sizeof(LensCatalog));
    if (lcat == NULL) {
        wlog("Could not allocate LensCatalog\n");
        exit(EXIT_FAILURE);
    }

    lcat->size = n_lens;

    //lcat->data = malloc(n_lens*sizeof(Lens));
    lcat->data = calloc(n_lens,sizeof(Lens));
    if (lcat->data == NULL) {
        wlog("Could not allocate %ld lenses in lcat\n", n_lens);
        exit(EXIT_FAILURE);
    }

    lcat->hpix_tree=NULL;

    return lcat;
}


LensCatalog* lcat_read(const ShearConfig* config, const char* lens_url) {

    int nread=0, expected=0;
    if (config->Dlens_input) {
        expected=6;
    } else {
        expected=5;
    }

    wlog("Reading lenses from %s\n", lens_url);
    FILE* stream=open_url(lens_url, "r");

    wlog("counting lines....");
    size_t nlens = count_lines_and_rewind(stream);

    wlog("%lu\n", nlens);

    wlog("    creating lcat...");

    LensCatalog* lcat=lcat_new(nlens);
    wlog("OK\n");

    wlog("    reading data...");
    double ra_rad,dec_rad;
    for (size_t i=0; i<nlens; i++) {

        Lens* lens = &lcat->data[i];

        if (config->Dlens_input) {
            nread=fscanf(stream,"%ld %lf %lf %lf %lf %ld",
                    &lens->index,&lens->ra,&lens->dec,&lens->z,&lens->da,&lens->maskflags);
        } else {
            nread=fscanf(stream,"%ld %lf %lf %lf %ld",
                    &lens->index,&lens->ra,&lens->dec,&lens->z,&lens->maskflags);
        }
        if (nread != expected) {
            wlog("Failed to read row %lu from %s\n", i, lens_url);
            wlog("nread=%d index=%d ra=%lf dec=%lf z=%lf maskflags=%d\n", nread, lens->index, lens->ra, lens->dec, lens->z, lens->maskflags);
            exit(EXIT_FAILURE);
        }

        ra_rad = lens->ra*D2R;
        dec_rad = lens->dec*D2R;

        lens->sinra = sin(ra_rad);
        lens->cosra = cos(ra_rad);
        lens->sindec = sin(dec_rad);
        lens->cosdec = cos(dec_rad);

        // add sin(lam),cos(lam),sin(eta),cos(eta)
        eq2sdss_sincos(lens->ra,lens->dec,
                       &lens->sinlam, &lens->coslam,
                       &lens->sineta, &lens->coseta);


    }
    fclose(stream);
    wlog("OK\n");

    return lcat;
}


#ifdef HDFS


LensCatalog* hdfs_lcat_read(const char* lens_url) {

    wlog("Reading lenses from hdfs %s\n", lens_url);


    hdfsFS fs;
    tSize file_buffsize=1024;

    fs = hdfs_connect();
    hdfsFile hf = hdfs_open(fs, lens_url, O_RDONLY, file_buffsize);
    size_t nlens;

    size_t lbsz=255;
    char* lbuf=calloc(lbsz, sizeof(char));

    hdfs_getline(hf, &lbuf, &lbsz); sscanf(lbuf, "%lu", &nlens);
    wlog("Reading %lu lenses\n", nlens);
    wlog("    creating lcat...");

    LensCatalog* lcat=lcat_new(nlens);
    wlog("OK\n");

    wlog("    reading data...");
    double ra_rad,dec_rad;
    for (size_t i=0; i<nlens; i++) {

        Lens* lens = &lcat->data[i];

        hdfs_getline(hf, &lbuf, &lbsz);

        int nread=sscanf(lbuf,"%ld %lf %lf %lf %ld",
                &lens->index,&lens->ra,&lens->dec,&lens->z,&lens->maskflags);

        if (5 != nread) {
            wlog("Failed to read row %lu from %s\n", i, lens_url);
            exit(EXIT_FAILURE);
        }

        ra_rad = lens->ra*D2R;
        dec_rad = lens->dec*D2R;

        lens->sinra = sin(ra_rad);
        lens->cosra = cos(ra_rad);
        lens->sindec = sin(dec_rad);
        lens->cosdec = cos(dec_rad);

        // add sin(lam),cos(lam),sin(eta),cos(eta)
        eq2sdss_sincos(lens->ra,lens->dec,
                       &lens->sinlam, &lens->coslam,
                       &lens->sineta, &lens->coseta);


    }

    hdfsCloseFile(fs, hf);
    hdfsDisconnect(fs);
    free(lbuf);
    wlog("OK\n");
    return lcat;


}

#endif

void lcat_add_da(LensCatalog* lcat, Cosmo* cosmo) {
    for (size_t i=0; i<lcat->size; i++) {
        Lens* lens = &lcat->data[i];
        lens->da = Da(cosmo, 0.0, lens->z);
    }
}

void lcat_add_search_angle(LensCatalog* lcat, double rmax) {
    for (size_t i=0; i<lcat->size; i++) {
        Lens* lens = &lcat->data[i];

        double search_angle = rmax/lens->da;
        lens->cos_search_angle = cos(search_angle);
    }
}



void lcat_disc_intersect(LensCatalog* lcat, HealPix* hpix, double rmax) {


    for (size_t i=0; i<lcat->size; i++) {

        Lens* lens = &lcat->data[i];

        lens->hpix = lvector_new();

        double search_angle = rmax/lens->da;
        hpix_disc_intersect(
                hpix, 
                lens->ra, lens->dec, 
                search_angle, 
                lens->hpix);

        // this should speed up initial checks of sources
        // since we can ask if within min/max pixel values
        lvector_sort(lens->hpix);

    }
}

void lcat_build_hpix_tree(HealPix* hpix, LensCatalog* lcat) {
    int64* ptr=NULL;

    for (size_t i=0; i<lcat->size; i++) {

        Lens* lens = &lcat->data[i];

        // add to the tree
        for (size_t j=0; j<lens->hpix->size; j++) {
            ptr = &lens->hpix->data[j];
            tree_insert(&lcat->hpix_tree, (*ptr)-hpix->half_npix, i);
        }

    }

}

void lcat_print_one(LensCatalog* lcat, size_t el) {
    Lens* lens = &lcat->data[el];
    wlog("element %ld of lcat:\n", el);
    wlog("    ra:        %lf\n", lens->ra);
    wlog("    dec:       %lf\n", lens->dec);
    wlog("    z:         %lf\n", lens->z);
    wlog("    da:        %lf\n", lens->da);
    wlog("    maskflags: %ld\n", lens->maskflags);
    wlog("    sinlam:    %lf\n", lens->sinlam);
    wlog("    coslam:    %lf\n", lens->coslam);
    wlog("    sineta:    %lf\n", lens->sineta);
    wlog("    coseta:    %lf\n", lens->coseta);
    wlog("    n(hpix):   %lu", lens->hpix->size);
    wlog(" hpix[0]: %ld hpix[%lu]: %ld\n", 
            lens->hpix->data[0],
            lens->hpix->size-1,
            lens->hpix->data[lens->hpix->size-1]);
}
void lcat_print_firstlast(LensCatalog* lcat) {
    lcat_print_one(lcat, 0);
    lcat_print_one(lcat, lcat->size-1);
}



// use like this:
//   lcat = lcat_free(lcat);
// This ensures that the lcat pointer is set to NULL
LensCatalog* lcat_free(LensCatalog* lcat) {

    if (lcat != NULL) {
        Lens* lens=&lcat->data[0];
        for (size_t i=0; i<lcat->size; i++) {
            vector_free(lens->hpix);
            lens++;
        }
        free(lcat->data);
        lcat->hpix_tree = tree_free(lcat->hpix_tree);
        free(lcat);
    }
    return NULL;
}
