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

typedef struct {
    ShearConfig*  config;
    Cosmo*   cosmo;
    HealPix* hpix;
    LensCatalog*    lcat;

    // this holds the info for a given lens
    Lensums* lensums;
    
    int64 tpairs;
    int64* totpairs;
    // min/max z for lenses
    double min_zlens, max_zlens;

    // output file pointer. We open at the beginning to make sure we can!
    //FILE* fptr;
} Shear;

Shear* shear_init(const char* config_url, const char* lens_url);
void shear_process_source(Shear* self, Source* src);
Shear* shear_free(Shear* self);


void shear_print_sum(Shear* self);
void shear_write(Shear* self, FILE* stream);

void shear_procpair(Shear* self, 
                    Source* src, 
                    Lens* lens, 
                    Lensum* lensum);

#endif

