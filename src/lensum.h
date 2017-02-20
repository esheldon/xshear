#ifndef _lensum_HEADER
#define _lensum_HEADER

#include "defs.h"

typedef struct {
    int shear_style;
    int shear_units;
    int scstyle;

    int64 index;   // index in the lens list
    double weight;
    int64 totpairs;

    int64 nbin;
    int64* npair;

    double* rsum; // sum of weight*scinv^2*radius
    double* wsum; // sum of weight*scinv^2
    double* ssum; // sum of weight*scinv
    double* dsum; // sum of weight*scinv*gt=weight*scinv^2*DeltaSigma
    double* osum; // sum of weight*scinv*gx=weight*scinv^2*DeltaSigma_x
    
    double* dsensum_w; // sum of weight*scinv^2*sens_t
    double* osensum_w; // sum of weight*scinv^2*sens_x
    double* dsensum_s; // sum of weight*scinv*sens_t
    double* osensum_s; // sum of weight*scinv*sens_x
} Lensum;

typedef struct {
    size_t size;
    Lensum* data;
} Lensums;


Lensums* lensums_new(size_t nlens, size_t nbin, int shear_style, int scstyle, int shear_units);

// this one we write all the data out in binary format
void lensums_write(Lensums* self, FILE* stream);

// these write the stdout
Lensum* lensums_sum(Lensums* lensums);
void lensums_print_sum(Lensums* lensums);
void lensums_print_one(Lensums* lensums, size_t index);
void lensums_print_firstlast(Lensums* lensums);

Lensums* lensums_free(Lensums* lensum);



Lensum* lensum_new(size_t nbin, int shear_style, int scstyle, int shear_units);
int lensum_read_into(Lensum* self, FILE* stream);
Lensum* lensum_copy(Lensum* lensum);
void lensum_add(Lensum* self, Lensum* src);

void lensum_write(Lensum* self, FILE* stream);
void lensum_print(Lensum* self);
void lensum_clear(Lensum* self);
Lensum* lensum_free(Lensum* self);



#endif
