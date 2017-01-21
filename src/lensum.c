#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include "lensum.h"
#include "defs.h"
#include "log.h"
#include "sconfig.h"

Lensums* lensums_new(size_t nlens, size_t nbin, int shear_style, int scstyle) {

    wlog("Creating lensums:\n");
    wlog("    nlens: %lu  nbin: %lu\n", nlens, nbin);

    Lensums* self=calloc(1,sizeof(Lensums));
    if (self == NULL) {
        wlog("failed to allocate lensums struct\n");
        exit(EXIT_FAILURE);
    }

    self->data = calloc(nlens, sizeof(Lensum));
    if (self->data == NULL) {
        wlog("failed to allocate lensum array\n");
        exit(EXIT_FAILURE);
    }

    self->size = nlens;


    for (size_t i=0; i<nlens; i++) {
        Lensum* lensum = &self->data[i];

        lensum->shear_style=shear_style;
        lensum->scstyle=scstyle;

        lensum->index = -1;
        lensum->nbin  = nbin;
        lensum->npair = calloc(nbin, sizeof(int64));
        lensum->rsum  = calloc(nbin, sizeof(double));
        lensum->wsum  = calloc(nbin, sizeof(double));
        lensum->ssum  = calloc(nbin, sizeof(double));
        lensum->dsum  = calloc(nbin, sizeof(double));
        lensum->osum  = calloc(nbin, sizeof(double));

        lensum->dsensum_w  = calloc(nbin, sizeof(double));
        lensum->osensum_w  = calloc(nbin, sizeof(double));
        lensum->dsensum_s  = calloc(nbin, sizeof(double));
        lensum->osensum_s  = calloc(nbin, sizeof(double));

        if (lensum->npair==NULL
                || lensum->wsum==NULL
                || lensum->ssum==NULL
                || lensum->dsum==NULL
                || lensum->osum==NULL
                || lensum->rsum==NULL
                || lensum->dsensum_w==NULL
                || lensum->osensum_w==NULL
                || lensum->dsensum_s==NULL
                || lensum->osensum_s==NULL
                ) {

            wlog("failed to allocate lensum\n");
            exit(EXIT_FAILURE);
        }

    }
    return self;

}

// this is for writing them all at once.  We actually usually
// write them one at a time
void lensums_write(Lensums* self, FILE* stream) {

    Lensum* lensum = &self->data[0];
    for (size_t i=0; i<self->size; i++) {
        lensum_write(lensum, stream);
        
        lensum++;
    }
}


Lensum* lensums_sum(Lensums* self) {
    Lensum* tsum=lensum_new(self->data[0].nbin,
                            self->data[0].shear_style, 
                            self->data[0].scstyle);


    for (size_t i=0; i<self->size; i++) {

        Lensum* lensum = &self->data[i];

        lensum_add(tsum, lensum);
    }
    return tsum;
}



void lensums_print_sum(Lensums* self) {
    Lensum* lensum = lensums_sum(self);
    lensum_print(lensum);
    lensum=lensum_free(lensum);
}

// these write the stdout
void lensums_print_one(Lensums* self, size_t index) {
    wlog("element %ld of lensums:\n",index);
    Lensum* lensum = &self->data[index];
    lensum_print(lensum);
}

void lensums_print_firstlast(Lensums* self) {
    lensums_print_one(self, 0);
    lensums_print_one(self, self->size-1);
}

Lensums* lensums_free(Lensums* self) {
    if (self != NULL) {
        if (self->data != NULL) {

            for (size_t i=0; i<self->size; i++) {
                Lensum* lensum = &self->data[i];
                free(lensum->npair);
                free(lensum->rsum);
                free(lensum->wsum);
                free(lensum->ssum);
                free(lensum->dsum);
                free(lensum->osum);
                free(lensum->dsensum_w);
                free(lensum->osensum_w);
                free(lensum->dsensum_s);
                free(lensum->osensum_s);
            }
            free(self->data);
        }
        free(self);
    }
    return NULL;
}

Lensum* lensum_new(size_t nbin, int shear_style, int scstyle) {
    Lensum* lensum=calloc(1,sizeof(Lensum));
    if (lensum == NULL) {
        wlog("failed to allocate lensum\n");
        exit(EXIT_FAILURE);
    }

    lensum->shear_style=shear_style;
    lensum->scstyle=scstyle;

    lensum->nbin = nbin;

    lensum->npair = calloc(nbin, sizeof(int64));
    lensum->rsum  = calloc(nbin, sizeof(double));
    lensum->wsum  = calloc(nbin, sizeof(double));
    lensum->ssum  = calloc(nbin, sizeof(double));
    lensum->dsum  = calloc(nbin, sizeof(double));
    lensum->osum  = calloc(nbin, sizeof(double));

    lensum->dsensum_w  = calloc(nbin, sizeof(double));
    lensum->osensum_w  = calloc(nbin, sizeof(double));
    lensum->dsensum_s  = calloc(nbin, sizeof(double));
    lensum->osensum_s  = calloc(nbin, sizeof(double));


    if (lensum->npair==NULL
            || lensum->wsum==NULL
            || lensum->ssum==NULL
            || lensum->dsum==NULL
            || lensum->osum==NULL
            || lensum->rsum==NULL
            || lensum->dsensum_w==NULL
            || lensum->osensum_w==NULL
            || lensum->dsensum_s==NULL
            || lensum->osensum_s==NULL
            ) {

        wlog("failed to allocate lensum\n");
        exit(EXIT_FAILURE);
    }

    return lensum;
}

// add the second lensum into the first
void lensum_add(Lensum* self, Lensum* src) {

    self->weight   += src->weight;
    self->totpairs += src->totpairs;

    for (size_t i=0; i<src->nbin; i++) {
        self->npair[i] += src->npair[i];
        self->rsum[i] += src->rsum[i];
        self->wsum[i] += src->wsum[i];
        self->ssum[i] += src->ssum[i];
        self->dsum[i] += src->dsum[i];
        self->osum[i] += src->osum[i];
        self->dsensum_w[i] += src->dsensum_w[i];
        self->osensum_w[i] += src->osensum_w[i];
        self->dsensum_s[i] += src->dsensum_s[i];
        self->osensum_s[i] += src->osensum_s[i];
    }

}

int lensum_read_into(Lensum* self, FILE* stream) {
    int nbin=self->nbin;
    int nexpect = 3+10*nbin;
    int nread=0;
    int i=0;

    nread+=fscanf(stream,"%lld", &self->index);
    nread+=fscanf(stream,"%lf", &self->weight);
    nread+=fscanf(stream,"%lld", &self->totpairs);

    for (i=0; i<nbin; i++) 
        nread+=fscanf(stream,"%lld", &self->npair[i]);

    for (i=0; i<nbin; i++) 
        nread+=fscanf(stream,"%lf", &self->rsum[i]);
    for (i=0; i<nbin; i++) 
        nread+=fscanf(stream,"%lf", &self->wsum[i]);
    for (i=0; i<nbin; i++) 
        nread+=fscanf(stream,"%lf", &self->ssum[i]);
    for (i=0; i<nbin; i++) 
        nread+=fscanf(stream,"%lf", &self->dsum[i]);
    for (i=0; i<nbin; i++) 
        nread+=fscanf(stream,"%lf", &self->osum[i]);

    for (i=0; i<nbin; i++) 
        nread+=fscanf(stream,"%lf", &self->dsensum_w[i]);
    for (i=0; i<nbin; i++) 
        nread+=fscanf(stream,"%lf", &self->osensum_w[i]);
    for (i=0; i<nbin; i++) 
        nread+=fscanf(stream,"%lf", &self->dsensum_s[i]);
    for (i=0; i<nbin; i++) 
        nread+=fscanf(stream,"%lf", &self->osensum_s[i]);

    return (nread == nexpect);
}

void lensum_write(Lensum* self, FILE* stream) {
    int nbin = self->nbin;
    int i=0;

    fprintf(stream,"%lld %.16g %lld",
            self->index, self->weight, self->totpairs);

    for (i=0; i<nbin; i++) 
        fprintf(stream," %lld", self->npair[i]);

    for (i=0; i<nbin; i++) 
        fprintf(stream," %.16g", self->rsum[i]);
    for (i=0; i<nbin; i++) 
        fprintf(stream," %.16g", self->wsum[i]);
    for (i=0; i<nbin; i++) 
        fprintf(stream," %.16g", self->ssum[i]);
    for (i=0; i<nbin; i++) 
        fprintf(stream," %.16g", self->dsum[i]);
    for (i=0; i<nbin; i++) 
        fprintf(stream," %.16g", self->osum[i]);

    for (i=0; i<nbin; i++) 
        fprintf(stream," %.16g", self->dsensum_w[i]);
    for (i=0; i<nbin; i++) 
        fprintf(stream," %.16g", self->osensum_w[i]);
    for (i=0; i<nbin; i++) 
        fprintf(stream," %.16g", self->dsensum_s[i]);
    for (i=0; i<nbin; i++) 
        fprintf(stream," %.16g", self->osensum_s[i]);

    fprintf(stream,"\n");

}

// these write the stdout
void lensum_print(Lensum* self) {

    wlog("  index:    %lld\n", self->index);
    wlog("  weight:   %g\n",  self->weight);
    wlog("  totpairs: %lld\n", self->totpairs);
    wlog("  nbin:     %lld\n", self->nbin);                            
    wlog("  bin       npair          wsum          ssum       meanr          dsum          osum");
    wlog("     dsensum_w     osensum_w     dsensum_s     osensum_s  DeltaSigma");
    wlog("\n");

    for (size_t i=0; i<self->nbin; i++) {
        //wlog("  %3lu %11ld %15.6lf %15.6lf %15.6lf %15.6lf", 
        wlog("  %3lu %11lld %13.6g %13.6g %11.6g %13.6g %13.6g %13.6g %13.6g %13.6g %13.6g %11.6g", 
             i,
             self->npair[i],
             self->wsum[i],
             self->ssum[i],
             self->rsum[i]/max(self->wsum[i],DBL_MIN),
             self->dsum[i],
             self->osum[i],
             self->dsensum_w[i],
             self->osensum_w[i],
             self->dsensum_s[i],
             self->osensum_s[i],
             self->dsum[i]/max(self->dsensum_s[i],DBL_MIN)
              );
        wlog("\n");
    }
}

Lensum* lensum_copy(Lensum* lensum) {
    int i=0;

    Lensum* copy=lensum_new(lensum->nbin, lensum->shear_style, lensum->scstyle);
    copy->index = lensum->index;
    copy->weight = lensum->weight;
    copy->totpairs = lensum->totpairs;

    copy->nbin = lensum->nbin;

    for (i=0; i<copy->nbin; i++) {
        copy->npair[i] = lensum->npair[i];
        copy->rsum[i] = lensum->rsum[i];
        copy->wsum[i] = lensum->wsum[i];
        copy->ssum[i] = lensum->ssum[i];
        copy->dsum[i] = lensum->dsum[i];
        copy->osum[i] = lensum->osum[i];
        copy->dsensum_s[i] = lensum->dsensum_s[i];
        copy->osensum_s[i] = lensum->osensum_s[i];
        copy->dsensum_w[i] = lensum->dsensum_w[i];
        copy->osensum_w[i] = lensum->osensum_w[i];
    }

    return copy;
}




void lensum_clear(Lensum* self) {

    self->index=-1;
    self->weight=0;
    self->totpairs=0;

    for (size_t i=0; i<self->nbin; i++) {
        self->npair[i] = 0;
        self->wsum[i] = 0;
        self->ssum[i] = 0;
        self->dsum[i] = 0;
        self->osum[i] = 0;
        self->rsum[i] = 0;
        self->dsensum_s[i] = 0;
        self->osensum_s[i] = 0;
        self->dsensum_w[i] = 0;
        self->osensum_w[i] = 0;
    }
}

Lensum* lensum_free(Lensum* self) {
    if (self != NULL) {
        free(self->npair);
        free(self->rsum);
        free(self->wsum);
        free(self->ssum);
        free(self->dsum);
        free(self->osum);
        free(self->dsensum_w);
        free(self->osensum_w);
        free(self->dsensum_s);
        free(self->osensum_s);
    }
    free(self);
    return NULL;
}
