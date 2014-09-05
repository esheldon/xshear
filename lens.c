#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "math.h"
#include "lens.h"
#include "cosmo.h"
#include "log.h"
#include "healpix.h"
#include "stack.h"
#include "urls.h"
#include "sdss-survey.h"

#ifdef HDFS
#include "hdfs_lines.h"
#endif

//#include "histogram.h"


struct lcat* lcat_new(size_t n_lens) {

    if (n_lens == 0) {
        wlog("lcat_new: size must be > 0\n");
        exit(EXIT_FAILURE);
    }

    //struct lcat* lcat = malloc(sizeof(struct lcat));
    struct lcat* lcat = calloc(1,sizeof(struct lcat));
    if (lcat == NULL) {
        wlog("Could not allocate struct lcat\n");
        exit(EXIT_FAILURE);
    }

    lcat->size = n_lens;

    //lcat->data = malloc(n_lens*sizeof(struct lens));
    lcat->data = calloc(n_lens,sizeof(struct lens));
    if (lcat->data == NULL) {
        wlog("Could not allocate %ld lenses in lcat\n", n_lens);
        exit(EXIT_FAILURE);
    }

    lcat->hpix_tree=NULL;

    return lcat;
}


struct lcat* lcat_read(const char* lens_url) {

#ifdef HDFS
    // if compiled with hdfs, and begins with hdfs:// then read as an hdfs file
    if (is_in_hdfs(lens_url)) {
        return hdfs_lcat_read(lens_url);
    }
#endif

    int nread=0;

    wlog("Reading lenses from %s\n", lens_url);
    FILE* stream=open_url(lens_url, "r");
    size_t nlens;

    nread=fscanf(stream,"%lu", &nlens);
    wlog("Reading %lu lenses\n", nlens);
    wlog("    creating lcat...");

    struct lcat* lcat=lcat_new(nlens);
    wlog("OK\n");

    wlog("    reading data...");
    struct lens* lens = &lcat->data[0];
    double ra_rad,dec_rad;
    for (size_t i=0; i<nlens; i++) {
        nread=fscanf(stream,"%ld %lf %lf %lf %ld",
                &lens->zindex,&lens->ra,&lens->dec,&lens->z,&lens->maskflags);
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

        lens++;
    }
    fclose(stream);
    wlog("OK\n");

    return lcat;
}


#ifdef HDFS


struct lcat* hdfs_lcat_read(const char* lens_url) {

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

    struct lcat* lcat=lcat_new(nlens);
    wlog("OK\n");

    wlog("    reading data...");
    struct lens* lens = &lcat->data[0];
    double ra_rad,dec_rad;
    for (size_t i=0; i<nlens; i++) {

        hdfs_getline(hf, &lbuf, &lbsz);

        int nread=sscanf(lbuf,"%ld %lf %lf %lf %ld",
                &lens->zindex,&lens->ra,&lens->dec,&lens->z,&lens->maskflags);

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

        lens++;

    }

    hdfsCloseFile(fs, hf);
    hdfsDisconnect(fs);
    free(lbuf);
    wlog("OK\n");
    return lcat;


}

#endif

void lcat_add_da(struct lcat* lcat, struct cosmo* cosmo) {
    struct lens* lens = &lcat->data[0];
    for (size_t i=0; i<lcat->size; i++) {
        lens->da = Da(cosmo, 0.0, lens->z);
        lens++;
    }
}

void lcat_add_search_angle(struct lcat* lcat, double rmax) {
    struct lens* lens = &lcat->data[0];
    for (size_t i=0; i<lcat->size; i++) {

        double search_angle = rmax/lens->da;
        lens->cos_search_angle = cos(search_angle);
        lens++;
    }
}



void lcat_disc_intersect(struct lcat* lcat, struct healpix* hpix, double rmax) {

    struct lens* lens = &lcat->data[0];

    for (size_t i=0; i<lcat->size; i++) {
        lens->hpix = i64stack_new(0);

        double search_angle = rmax/lens->da;
        hpix_disc_intersect(
                hpix, 
                lens->ra, lens->dec, 
                search_angle, 
                lens->hpix);

        // this should speed up initial checks of sources
        // since we can ask if within min/max pixel values
        i64stack_sort(lens->hpix);

        //lens->rev = i64getrev(lens->hpix);
        lens++;
    }
}

void lcat_build_hpix_tree(struct healpix* hpix, struct lcat* lcat) {
    int64* ptr=NULL;

    struct lens* lens = &lcat->data[0];
    for (size_t i=0; i<lcat->size; i++) {
        // add to the tree
        ptr = &lens->hpix->data[0];
        for (size_t j=0; j<lens->hpix->size; j++) {
            tree_insert(&lcat->hpix_tree, (*ptr)-hpix->half_npix, i);
            ptr++;
        }

        lens++;
    }

}

void lcat_print_one(struct lcat* lcat, size_t el) {
    struct lens* lens = &lcat->data[el];
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
void lcat_print_firstlast(struct lcat* lcat) {
    lcat_print_one(lcat, 0);
    lcat_print_one(lcat, lcat->size-1);
}



// use like this:
//   lcat = lcat_delete(lcat);
// This ensures that the lcat pointer is set to NULL
struct lcat* lcat_delete(struct lcat* lcat) {

    if (lcat != NULL) {
        struct lens* lens=&lcat->data[0];
        for (size_t i=0; i<lcat->size; i++) {
            lens->hpix = i64stack_delete(lens->hpix);
            //lens->rev = szvector_delete(lens->rev);
            lens++;
        }
        free(lcat->data);
        lcat->hpix_tree = tree_delete(lcat->hpix_tree);
        free(lcat);
    }
    return NULL;
}
