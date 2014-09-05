#include <stdlib.h>
#include <stdio.h>
#include "lensum.h"
#include "defs.h"
#include "log.h"

struct lensums* lensums_new(size_t nlens, size_t nbin) {
    wlog("Creating lensums:\n");
    wlog("    nlens: %lu  nbin: %lu\n", nlens, nbin);

    struct lensums* lensums=calloc(1,sizeof(struct lensums));
    if (lensums == NULL) {
        wlog("failed to allocate lensums struct\n");
        exit(EXIT_FAILURE);
    }

    lensums->data = calloc(nlens, sizeof(struct lensum));
    if (lensums->data == NULL) {
        wlog("failed to allocate lensum array\n");
        exit(EXIT_FAILURE);
    }

    lensums->size = nlens;

    struct lensum* lensum = &lensums->data[0];

    for (size_t i=0; i<nlens; i++) {
        lensum->index = i;
        lensum->nbin = nbin;
        lensum->npair = calloc(nbin, sizeof(int64));
        lensum->wsum  = calloc(nbin, sizeof(double));
        lensum->dsum  = calloc(nbin, sizeof(double));
        lensum->osum  = calloc(nbin, sizeof(double));
#ifdef LENSFIT
        lensum->dsensum  = calloc(nbin, sizeof(double));
        lensum->osensum  = calloc(nbin, sizeof(double));
#endif
        lensum->rsum  = calloc(nbin, sizeof(double));

        if (lensum->npair==NULL
                || lensum->wsum==NULL
                || lensum->dsum==NULL
                || lensum->osum==NULL
                || lensum->rsum==NULL) {

            wlog("failed to allocate lensum\n");
            exit(EXIT_FAILURE);
        }

        lensum++;
    }
    return lensums;

}

// this is for writing them all at once.  We actually usually
// write them one at a time
void lensums_write(struct lensums* lensums, FILE* stream) {

    struct lensum* lensum = &lensums->data[0];
    for (size_t i=0; i<lensums->size; i++) {
        lensum_write(lensum, stream);
        
        lensum++;
    }
}


struct lensum* lensums_sum(struct lensums* lensums) {
    struct lensum* tsum=lensum_new(lensums->data[0].nbin);

    struct lensum* lensum = &lensums->data[0];

    for (size_t i=0; i<lensums->size; i++) {
        tsum->weight   += lensum->weight;
        tsum->totpairs += lensum->totpairs;

        for (size_t j=0; j<lensum->nbin; j++) {
            tsum->npair[j] += lensum->npair[j];
            tsum->rsum[j] += lensum->rsum[j];
            tsum->wsum[j] += lensum->wsum[j];
            tsum->dsum[j] += lensum->dsum[j];
            tsum->osum[j] += lensum->osum[j];
#ifdef LENSFIT
            tsum->dsensum[j] += lensum->dsensum[j];
            tsum->osensum[j] += lensum->osensum[j];
#endif
        }
        lensum++;
    }
    return tsum;
}



void lensums_print_sum(struct lensums* lensums) {
    struct lensum* lensum = lensums_sum(lensums);
    lensum_print(lensum);
    lensum=lensum_delete(lensum);
}

// these write the stdout
void lensums_print_one(struct lensums* lensums, size_t index) {
    wlog("element %ld of lensums:\n",index);
    struct lensum* lensum = &lensums->data[index];
    lensum_print(lensum);
}

void lensums_print_firstlast(struct lensums* lensums) {
    lensums_print_one(lensums, 0);
    lensums_print_one(lensums, lensums->size-1);
}

struct lensums* lensums_delete(struct lensums* lensums) {
    if (lensums != NULL) {
        struct lensum* lensum = &lensums->data[0];

        for (size_t i=0; i<lensums->size; i++) {
            free(lensum->npair);
            free(lensum->wsum);
            free(lensum->dsum);
            free(lensum->osum);
#ifdef LENSFIT
            free(lensum->dsensum);
            free(lensum->osensum);
#endif
            free(lensum->rsum);
            lensum++;
        }
    }
    free(lensums);
    return NULL;
}

struct lensum* lensum_new(size_t nbin) {
    struct lensum* lensum=calloc(1,sizeof(struct lensum));
    if (lensum == NULL) {
        wlog("failed to allocate lensum\n");
        exit(EXIT_FAILURE);
    }

    lensum->nbin = nbin;

    lensum->npair = calloc(nbin, sizeof(int64));
    lensum->wsum  = calloc(nbin, sizeof(double));
    lensum->dsum  = calloc(nbin, sizeof(double));
    lensum->osum  = calloc(nbin, sizeof(double));

#ifdef LENSFIT
    lensum->dsensum  = calloc(nbin, sizeof(double));
    lensum->osensum  = calloc(nbin, sizeof(double));
#endif

    lensum->rsum  = calloc(nbin, sizeof(double));

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
void lensum_add(struct lensum* dest, struct lensum* src) {

    dest->weight   += src->weight;
    dest->totpairs += src->totpairs;

    for (size_t i=0; i<src->nbin; i++) {
        dest->npair[i] += src->npair[i];
        dest->rsum[i] += src->rsum[i];
        dest->wsum[i] += src->wsum[i];
        dest->dsum[i] += src->dsum[i];
        dest->osum[i] += src->osum[i];
#ifdef LENSFIT
        dest->dsensum[i] += src->dsensum[i];
        dest->osensum[i] += src->osensum[i];
#endif
    }

}

int lensum_read(FILE* stream, struct lensum* lensum) {
    int nbin=lensum->nbin;
    int nexpect = 7+5*nbin;
    int nread=0;
    int i=0;

    nread+=fscanf(stream,"%ld", &lensum->index);
    nread+=fscanf(stream,"%ld", &lensum->zindex);
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
#ifdef lensfit
    for (i=0; i<nbin; i++) 
        nread+=fscanf(stream,"%lf", &lensum->dsensum[i]);
    for (i=0; i<nbin; i++) 
        nread+=fscanf(stream,"%lf", &lensum->osensum[i]);
    nexpect += 2*nbin;
#endif

    return (nread == nexpect);
}

void lensum_write(struct lensum* lensum, FILE* stream) {
    int nbin = lensum->nbin;
    int i=0;

    // id with tab at beginning is demanded by hadoop map reduce
    // should we just use zindex there?
    fprintf(stream,"%ld %ld %.16g %ld ", 
            lensum->index, lensum->zindex, lensum->weight, lensum->totpairs);

    for (i=0; i<nbin; i++) 
        fprintf(stream,"%ld", lensum->npair[i]);

    for (i=0; i<nbin; i++) 
        fprintf(stream," %.16g", lensum->rsum[i]);
    for (i=0; i<nbin; i++) 
        fprintf(stream," %.16g", lensum->wsum[i]);
    for (i=0; i<nbin; i++) 
        fprintf(stream," %.16g", lensum->dsum[i]);
    for (i=0; i<nbin; i++) 
        fprintf(stream," %.16g", lensum->osum[i]);

#ifdef LENSFIT
    for (i=0; i<nbin; i++) 
        fprintf(stream," %.16g", lensum->dsensum[i]);
    for (i=0; i<nbin; i++) 
        fprintf(stream," %.16g", lensum->osensum[i]);
#endif

    fprintf(stream,"\n");

}

// these write the stdout
void lensum_print(struct lensum* lensum) {
    wlog("  zindex:   %ld\n", lensum->zindex);
    wlog("  weight:   %lf\n", lensum->weight);
    wlog("  totpairs: %ld\n", lensum->totpairs);
    wlog("  nbin:     %ld\n", lensum->nbin);
    wlog("  bin       npair            meanr           dsum            osum");
#ifdef LENSFIT
    wlog("           dsensum        osensum");
#endif
    wlog("\n");

    for (size_t i=0; i<lensum->nbin; i++) {
        wlog("  %3lu %11ld %15.6lf %15.6lf %15.6lf", 
             i,
             lensum->npair[i],
             lensum->rsum[i]/lensum->npair[i],
             lensum->dsum[i],
             lensum->osum[i] );
#ifdef LENSFIT
        wlog(" %15.6lf %15.6lf", lensum->dsensum[i], lensum->osensum[i]);
#endif
        wlog("\n");
    }
}

struct lensum* lensum_copy(struct lensum* lensum) {
    int i=0;

    struct lensum* copy=lensum_new(lensum->nbin);
    copy->index = lensum->index;
    copy->zindex = lensum->zindex;
    copy->weight = lensum->weight;
    copy->totpairs = lensum->totpairs;

    copy->nbin = lensum->nbin;

    for (i=0; i<copy->nbin; i++) {
        copy->npair[i] = lensum->npair[i];
        copy->rsum[i] = lensum->rsum[i];
        copy->wsum[i] = lensum->wsum[i];
        copy->dsum[i] = lensum->dsum[i];
        copy->osum[i] = lensum->osum[i];
#ifdef LENSFIT
        copy->dsensum[i] = lensum->dsensum[i];
        copy->osensum[i] = lensum->osensum[i];
#endif
    }

    return copy;
}




void lensum_clear(struct lensum* lensum) {

    lensum->zindex=-1;
    lensum->weight=0;
    lensum->totpairs=0;

    for (size_t i=0; i<lensum->nbin; i++) {
        lensum->npair[i] = 0;
        lensum->wsum[i] = 0;
        lensum->dsum[i] = 0;
        lensum->osum[i] = 0;
        lensum->rsum[i] = 0;

#ifdef LENSFIT
        lensum->dsensum[i] = 0;
        lensum->osensum[i] = 0;
#endif
    }
}

struct lensum* lensum_delete(struct lensum* lensum) {
    if (lensum != NULL) {
        free(lensum->npair);
        free(lensum->rsum);
        free(lensum->wsum);
        free(lensum->dsum);
        free(lensum->osum);

#ifdef LENSFIT
        free(lensum->dsensum);
        free(lensum->osensum);
#endif
    }
    free(lensum);
    return NULL;
}
