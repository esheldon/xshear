#ifndef _lensum_HEADER
#define _lensum_HEADER

#include "defs.h"

typedef struct {
    int shear_style;

    int64 index;   // index in the lens list
    double weight;
    int64 totpairs;

    int64 nbin;
    int64* npair;
    double* rsum;
    double* wsum;
    double* dsum;
    double* osum;

    // only used for lensfit style
    double* dsensum;
    double* osensum;
} Lensum;

typedef struct {
    size_t size;
    Lensum* data;
} Lensums;


Lensums* lensums_new(size_t nlens, size_t nbin, int shear_style);

// this one we write all the data out in binary format
void lensums_write(Lensums* lensums, FILE* stream);

// these write the stdout
Lensum* lensums_sum(Lensums* lensums);
void lensums_print_sum(Lensums* lensums);
void lensums_print_one(Lensums* lensums, size_t index);
void lensums_print_firstlast(Lensums* lensums);

Lensums* lensums_free(Lensums* lensum);



Lensum* lensum_new(size_t nbin, int shear_style);
int lensum_read(FILE* stream, Lensum* lensum);
Lensum* lensum_copy(Lensum* lensum);
void lensum_add(Lensum* dest, Lensum* src);

void lensum_write(Lensum* lensum, FILE* stream);
void lensum_print(Lensum* lensum);
void lensum_clear(Lensum* lensum);
Lensum* lensum_free(Lensum* lensum);



#endif
