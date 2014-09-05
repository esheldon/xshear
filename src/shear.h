#ifndef _SHEAR_HEADER
#define _SHEAR_HEADER

#include <stdio.h>
#include "defs.h"
#include "sconfig.h"
#include "cosmo.h"
#include "healpix.h"
#include "lens.h"
#include "lensum.h"
#include "source.h"

struct shear {
    struct sconfig*  config;
    struct cosmo*   cosmo;
    struct healpix* hpix;
    struct lcat*    lcat;

    // this holds the info for a given lens
    struct lensums* lensums;

    // min/max z for lenses
    double min_zlens, max_zlens;

    // output file pointer. We open at the beginning to make sure we can!
    //FILE* fptr;
};

struct shear* shear_init(const char* config_url, const char* lens_url);
void shear_process_source(struct shear* self, struct source* src);
struct shear* shear_delete(struct shear* self);


void shear_print_sum(struct shear* self);
void shear_write(struct shear* self, FILE* stream);

void shear_procpair(struct shear* self, 
                    struct source* src, 
                    struct lens* lens, 
                    struct lensum* lensum);

#endif

