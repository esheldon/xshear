#include <stdlib.h>
#include <stdio.h>
#include "lensum.h"
#include "defs.h"
#include "log.h"
#include "sconfig.h"

Lensums* lensums_new(size_t nlens, size_t nbin, int shear_style) {

    wlog("Creating lensums:\n");
    wlog("    nlens: %lu  nbin: %lu\n", nlens, nbin);

    Lensums* lensums=calloc(1,sizeof(Lensums));
    if (lensums == NULL) {
        wlog("failed to allocate lensums struct\n");
        exit(EXIT_FAILURE);
    }

    lensums->data = calloc(nlens, sizeof(Lensum));
    if (lensums->data == NULL) {
        wlog("failed to allocate lensum array\n");
        exit(EXIT_FAILURE);
    }

    lensums->size = nlens;


    for (size_t i=0; i<nlens; i++) {
        Lensum* lensum = &lensums->data[i];

        lensum->shear_style=shear_style;

        lensum->index = -1;
        lensum->nbin  = nbin;
        lensum->npair = calloc(nbin, sizeof(int64));
        lensum->rsum  = calloc(nbin, sizeof(double));
        lensum->wsum  = calloc(nbin, sizeof(double));
        lensum->dsum  = calloc(nbin, sizeof(double));
        lensum->osum  = calloc(nbin, sizeof(double));

        if (lensum->shear_style==SHEAR_STYLE_LENSFIT) {
            lensum->dsensum  = calloc(nbin, sizeof(double));
            lensum->osensum  = calloc(nbin, sizeof(double));
        }

        if (lensum->npair==NULL
                || lensum->wsum==NULL
                || lensum->dsum==NULL
                || lensum->osum==NULL
                || lensum->rsum==NULL) {

            wlog("failed to allocate lensum\n");
            exit(EXIT_FAILURE);
        }

    }
    return lensums;

}

// this is for writing them all at once.  We actually usually
// write them one at a time
void lensums_write(Lensums* lensums, FILE* stream) {

    Lensum* lensum = &lensums->data[0];
    for (size_t i=0; i<lensums->size; i++) {
        lensum_write(lensum, stream);
        
        lensum++;
    }
}


Lensum* lensums_sum(Lensums* lensums) {
    Lensum* tsum=lensum_new(lensums->data[0].nbin,
                                   lensums->data[0].shear_style);

    Lensum* lensum = &lensums->data[0];

    for (size_t i=0; i<lensums->size; i++) {
        tsum->weight   += lensum->weight;
        tsum->totpairs += lensum->totpairs;

        for (size_t j=0; j<lensum->nbin; j++) {
            tsum->npair[j] += lensum->npair[j];
            tsum->rsum[j] += lensum->rsum[j];
            tsum->wsum[j] += lensum->wsum[j];
            tsum->dsum[j] += lensum->dsum[j];
            tsum->osum[j] += lensum->osum[j];

            if (tsum->shear_style==SHEAR_STYLE_LENSFIT) {
                tsum->dsensum[j] += lensum->dsensum[j];
                tsum->osensum[j] += lensum->osensum[j];
            }

        }
        lensum++;
    }
    return tsum;
}



void lensums_print_sum(Lensums* lensums) {
    Lensum* lensum = lensums_sum(lensums);
    lensum_print(lensum);
    lensum=lensum_free(lensum);
}

// these write the stdout
void lensums_print_one(Lensums* lensums, size_t index) {
    wlog("element %ld of lensums:\n",index);
    Lensum* lensum = &lensums->data[index];
    lensum_print(lensum);
}

void lensums_print_firstlast(Lensums* lensums) {
    lensums_print_one(lensums, 0);
    lensums_print_one(lensums, lensums->size-1);
}

Lensums* lensums_free(Lensums* lensums) {
    if (lensums != NULL) {
        if (lensums->data != NULL) {

            for (size_t i=0; i<lensums->size; i++) {
                Lensum* lensum = &lensums->data[i];
                free(lensum->npair);
                free(lensum->rsum);
                free(lensum->wsum);
                free(lensum->dsum);
                free(lensum->osum);

                if (lensum->shear_style==SHEAR_STYLE_LENSFIT) {
                    free(lensum->dsensum);
                    free(lensum->osensum);
                }

            }
            free(lensums->data);
        }
        free(lensums);
    }
    return NULL;
}

Lensum* lensum_new(size_t nbin, int shear_style) {
    Lensum* lensum=calloc(1,sizeof(Lensum));
    if (lensum == NULL) {
        wlog("failed to allocate lensum\n");
        exit(EXIT_FAILURE);
    }

    lensum->shear_style=shear_style;

    lensum->nbin = nbin;

    lensum->npair = calloc(nbin, sizeof(int64));
    lensum->rsum  = calloc(nbin, sizeof(double));
    lensum->wsum  = calloc(nbin, sizeof(double));
    lensum->dsum  = calloc(nbin, sizeof(double));
    lensum->osum  = calloc(nbin, sizeof(double));

    if (lensum->shear_style==SHEAR_STYLE_LENSFIT) {
        lensum->dsensum  = calloc(nbin, sizeof(double));
        lensum->osensum  = calloc(nbin, sizeof(double));
    }

    if (lensum->npair==NULL
            || lensum->wsum==NULL
            || lensum->dsum==NULL
            || lensum->osum==NULL
            || lensum->rsum==NULL) {

        wlog("failed to allocate lensum\n");
        exit(EXIT_FAILURE);
    }

    return lensum;
}

// add the second lensum into the first
void lensum_add(Lensum* dest, Lensum* src) {

    dest->weight   += src->weight;
    dest->totpairs += src->totpairs;

    for (size_t i=0; i<src->nbin; i++) {
        dest->npair[i] += src->npair[i];
        dest->rsum[i] += src->rsum[i];
        dest->wsum[i] += src->wsum[i];
        dest->dsum[i] += src->dsum[i];
        dest->osum[i] += src->osum[i];
        if (src->shear_style==SHEAR_STYLE_LENSFIT) {
            dest->dsensum[i] += src->dsensum[i];
            dest->osensum[i] += src->osensum[i];
        }
    }

}

int lensum_read(FILE* stream, Lensum* lensum) {
    int nbin=lensum->nbin;
    int nexpect = 3+5*nbin;
    int nread=0;
    int i=0;

    nread+=fscanf(stream,"%ld", &lensum->index);
    nread+=fscanf(stream,"%lf", &lensum->weight);
    nread+=fscanf(stream,"%ld", &lensum->totpairs);

    for (i=0; i<nbin; i++) 
        nread+=fscanf(stream,"%ld", &lensum->npair[i]);

    for (i=0; i<nbin; i++) 
        nread+=fscanf(stream,"%lf", &lensum->rsum[i]);
    for (i=0; i<nbin; i++) 
        nread+=fscanf(stream,"%lf", &lensum->wsum[i]);
    for (i=0; i<nbin; i++) 
        nread+=fscanf(stream,"%lf", &lensum->dsum[i]);
    for (i=0; i<nbin; i++) 
        nread+=fscanf(stream,"%lf", &lensum->osum[i]);

    if (lensum->shear_style==SHEAR_STYLE_LENSFIT) {
        for (i=0; i<nbin; i++) 
            nread+=fscanf(stream,"%lf", &lensum->dsensum[i]);
        for (i=0; i<nbin; i++) 
            nread+=fscanf(stream,"%lf", &lensum->osensum[i]);
        nexpect += 2*nbin;
    }

    return (nread == nexpect);
}

void lensum_write(Lensum* lensum, FILE* stream) {
    int nbin = lensum->nbin;
    int i=0;

    fprintf(stream,"%ld %.16g %ld", 
            lensum->index, lensum->weight, lensum->totpairs);

    for (i=0; i<nbin; i++) 
        fprintf(stream," %ld", lensum->npair[i]);

    for (i=0; i<nbin; i++) 
        fprintf(stream," %.16g", lensum->rsum[i]);
    for (i=0; i<nbin; i++) 
        fprintf(stream," %.16g", lensum->wsum[i]);
    for (i=0; i<nbin; i++) 
        fprintf(stream," %.16g", lensum->dsum[i]);
    for (i=0; i<nbin; i++) 
        fprintf(stream," %.16g", lensum->osum[i]);

    if (lensum->shear_style==SHEAR_STYLE_LENSFIT) {
        for (i=0; i<nbin; i++) 
            fprintf(stream," %.16g", lensum->dsensum[i]);
        for (i=0; i<nbin; i++) 
            fprintf(stream," %.16g", lensum->osensum[i]);
    }

    fprintf(stream,"\n");

}

// these write the stdout
void lensum_print(Lensum* lensum) {
    wlog("  index:    %ld\n", lensum->index);
    wlog("  weight:   %lf\n", lensum->weight);
    wlog("  totpairs: %ld\n", lensum->totpairs);
    wlog("  nbin:     %ld\n", lensum->nbin);
    wlog("  bin       npair            wsum           meanr            dsum            osum");
    if (lensum->shear_style==SHEAR_STYLE_LENSFIT) {
        wlog("         dsensum         osensum");
    }
    wlog("\n");

    for (size_t i=0; i<lensum->nbin; i++) {
        //wlog("  %3lu %11ld %15.6lf %15.6lf %15.6lf %15.6lf", 
        wlog("  %3lu %11ld %15.6g %15.6g %15.6g %15.6g", 
             i,
             lensum->npair[i],
             lensum->wsum[i],
             lensum->rsum[i]/lensum->wsum[i],
             lensum->dsum[i],
             lensum->osum[i] );
        if (lensum->shear_style==SHEAR_STYLE_LENSFIT) {
            //wlog(" %15.6lf %15.6lf", lensum->dsensum[i], lensum->osensum[i]);
            wlog(" %15.6g %15.6g", lensum->dsensum[i], lensum->osensum[i]);
        }
        wlog("\n");
    }
}

Lensum* lensum_copy(Lensum* lensum) {
    int i=0;

    Lensum* copy=lensum_new(lensum->nbin, lensum->shear_style);
    copy->index = lensum->index;
    copy->weight = lensum->weight;
    copy->totpairs = lensum->totpairs;

    copy->nbin = lensum->nbin;

    for (i=0; i<copy->nbin; i++) {
        copy->npair[i] = lensum->npair[i];
        copy->rsum[i] = lensum->rsum[i];
        copy->wsum[i] = lensum->wsum[i];
        copy->dsum[i] = lensum->dsum[i];
        copy->osum[i] = lensum->osum[i];
        if (lensum->shear_style==SHEAR_STYLE_LENSFIT) {
            copy->dsensum[i] = lensum->dsensum[i];
            copy->osensum[i] = lensum->osensum[i];
        }
    }

    return copy;
}




void lensum_clear(Lensum* lensum) {

    lensum->index=-1;
    lensum->weight=0;
    lensum->totpairs=0;

    for (size_t i=0; i<lensum->nbin; i++) {
        lensum->npair[i] = 0;
        lensum->wsum[i] = 0;
        lensum->dsum[i] = 0;
        lensum->osum[i] = 0;
        lensum->rsum[i] = 0;

        if (lensum->shear_style==SHEAR_STYLE_LENSFIT) {
            lensum->dsensum[i] = 0;
            lensum->osensum[i] = 0;
        }
    }
}

Lensum* lensum_free(Lensum* lensum) {
    if (lensum != NULL) {
        free(lensum->npair);
        free(lensum->rsum);
        free(lensum->wsum);
        free(lensum->dsum);
        free(lensum->osum);

        if (lensum->shear_style==SHEAR_STYLE_LENSFIT) {
            free(lensum->dsensum);
            free(lensum->osensum);
        }
    }
    free(lensum);
    return NULL;
}
