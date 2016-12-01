#include <stdlib.h>
#include <stdio.h>
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
        lensum->dsum  = calloc(nbin, sizeof(double));
        lensum->osum  = calloc(nbin, sizeof(double));

        if (lensum->shear_style==SHEAR_STYLE_LENSFIT) {
            lensum->dsensum  = calloc(nbin, sizeof(double));
            lensum->osensum  = calloc(nbin, sizeof(double));
        }

        if (lensum->scstyle==SIGMACRIT_STYLE_SAMPLE) {
            lensum->scinvsum = calloc(nbin, sizeof(double));
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
                free(lensum->dsum);
                free(lensum->osum);

                if (lensum->shear_style==SHEAR_STYLE_LENSFIT) {
                    free(lensum->dsensum);
                    free(lensum->osensum);
                }

                if (lensum->scstyle==SIGMACRIT_STYLE_SAMPLE) {
                    free(lensum->scinvsum);
                }

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
    lensum->dsum  = calloc(nbin, sizeof(double));
    lensum->osum  = calloc(nbin, sizeof(double));

    if (lensum->shear_style==SHEAR_STYLE_LENSFIT) {
        lensum->dsensum  = calloc(nbin, sizeof(double));
        lensum->osensum  = calloc(nbin, sizeof(double));
    }
    if (lensum->scstyle==SIGMACRIT_STYLE_SAMPLE) {
        lensum->scinvsum  = calloc(nbin, sizeof(double));
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
void lensum_add(Lensum* self, Lensum* src) {

    self->weight   += src->weight;
    self->totpairs += src->totpairs;

    for (size_t i=0; i<src->nbin; i++) {
        self->npair[i] += src->npair[i];
        self->rsum[i] += src->rsum[i];
        self->wsum[i] += src->wsum[i];
        self->dsum[i] += src->dsum[i];
        self->osum[i] += src->osum[i];
        if (src->shear_style==SHEAR_STYLE_LENSFIT) {
            self->dsensum[i] += src->dsensum[i];
            self->osensum[i] += src->osensum[i];
        }
        if (src->scstyle==SIGMACRIT_STYLE_SAMPLE) {
            self->scinvsum[i] += src->scinvsum[i];
        }
    }

}

int lensum_read_into(Lensum* self, FILE* stream) {
    int nbin=self->nbin;
    int nexpect = 3+5*nbin;
    int nread=0;
    int i=0;

    nread+=fscanf(stream,"%ld", &self->index);
    nread+=fscanf(stream,"%lf", &self->weight);
    nread+=fscanf(stream,"%ld", &self->totpairs);

    for (i=0; i<nbin; i++) 
        nread+=fscanf(stream,"%ld", &self->npair[i]);

    for (i=0; i<nbin; i++) 
        nread+=fscanf(stream,"%lf", &self->rsum[i]);
    for (i=0; i<nbin; i++) 
        nread+=fscanf(stream,"%lf", &self->wsum[i]);
    for (i=0; i<nbin; i++) 
        nread+=fscanf(stream,"%lf", &self->dsum[i]);
    for (i=0; i<nbin; i++) 
        nread+=fscanf(stream,"%lf", &self->osum[i]);

    if (self->shear_style==SHEAR_STYLE_LENSFIT) {
        for (i=0; i<nbin; i++) 
            nread+=fscanf(stream,"%lf", &self->dsensum[i]);
        for (i=0; i<nbin; i++) 
            nread+=fscanf(stream,"%lf", &self->osensum[i]);
        nexpect += 2*nbin;
    }
    if (self->scstyle==SIGMACRIT_STYLE_SAMPLE) {
        for (i=0; i<nbin; i++) 
            nread+=fscanf(stream,"%lf", &self->scinvsum[i]);
        nexpect += nbin;
    }

    return (nread == nexpect);
}

void lensum_write(Lensum* self, FILE* stream) {
    int nbin = self->nbin;
    int i=0;

    fprintf(stream,"%ld %.16g %ld",
            self->index, self->weight, self->totpairs);

    for (i=0; i<nbin; i++) 
        fprintf(stream," %ld", self->npair[i]);

    for (i=0; i<nbin; i++) 
        fprintf(stream," %.16g", self->rsum[i]);
    for (i=0; i<nbin; i++) 
        fprintf(stream," %.16g", self->wsum[i]);
    for (i=0; i<nbin; i++) 
        fprintf(stream," %.16g", self->dsum[i]);
    for (i=0; i<nbin; i++) 
        fprintf(stream," %.16g", self->osum[i]);

    if (self->shear_style==SHEAR_STYLE_LENSFIT) {
        for (i=0; i<nbin; i++) 
            fprintf(stream," %.16g", self->dsensum[i]);
        for (i=0; i<nbin; i++) 
            fprintf(stream," %.16g", self->osensum[i]);
    }

    if (self->scstyle==SIGMACRIT_STYLE_SAMPLE) {
        for (i=0; i<nbin; i++)
            fprintf(stream," %.16g", self->scinvsum[i]);
    }

    fprintf(stream,"\n");

}

// these write the stdout
void lensum_print(Lensum* self) {

    wlog("  index:    %ld\n", self->index);
    wlog("  weight:   %g\n",  self->weight);
    wlog("  totpairs: %ld\n", self->totpairs);
    wlog("  nbin:     %ld\n", self->nbin);
    wlog("  bin       npair            wsum           meanr            dsum            osum");
    if (self->shear_style==SHEAR_STYLE_LENSFIT) {
        wlog("         dsensum         osensum");
    }
    if (self->scstyle==SIGMACRIT_STYLE_SAMPLE) {
        wlog("         scinvsum");
    }
    wlog("\n");

    for (size_t i=0; i<self->nbin; i++) {
        //wlog("  %3lu %11ld %15.6lf %15.6lf %15.6lf %15.6lf", 
        wlog("  %3lu %11ld %15.6g %15.6g %15.6g %15.6g", 
             i,
             self->npair[i],
             self->wsum[i],
             self->rsum[i]/self->wsum[i],
             self->dsum[i],
             self->osum[i] );
        if (self->shear_style==SHEAR_STYLE_LENSFIT) {
            wlog(" %15.6g %15.6g", self->dsensum[i], self->osensum[i]);
        }
        if (self->scstyle==SIGMACRIT_STYLE_SAMPLE) {
            wlog(" %15.6g", self->scinvsum[i]);
        }

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
        copy->dsum[i] = lensum->dsum[i];
        copy->osum[i] = lensum->osum[i];
        if (lensum->shear_style==SHEAR_STYLE_LENSFIT) {
            copy->dsensum[i] = lensum->dsensum[i];
            copy->osensum[i] = lensum->osensum[i];
        }
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
        self->dsum[i] = 0;
        self->osum[i] = 0;
        self->rsum[i] = 0;

        if (self->shear_style==SHEAR_STYLE_LENSFIT) {
            self->dsensum[i] = 0;
            self->osensum[i] = 0;
        }
        if (self->scstyle==SIGMACRIT_STYLE_SAMPLE) {
            self->scinvsum[i] = 0;
        }
    }
}

Lensum* lensum_free(Lensum* self) {
    if (self != NULL) {
        free(self->npair);
        free(self->rsum);
        free(self->wsum);
        free(self->dsum);
        free(self->osum);

        if (self->shear_style==SHEAR_STYLE_LENSFIT) {
            free(self->dsensum);
            free(self->osensum);
        }
        if (self->scstyle==SIGMACRIT_STYLE_SAMPLE) {
            free(self->scinvsum);
        }
    }
    free(self);
    return NULL;
}
