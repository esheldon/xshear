#ifndef _lensum_HEADER
#define _lensum_HEADER

#include "defs.h"

struct lensum {
    int64 index;   // index in the lens list
    int64 zindex;  // overall index, to get back to input catalog
    double weight;
    int64 totpairs;

    int64 nbin;
    int64* npair;
    double* rsum;
    double* wsum;
    double* dsum;
    double* osum;
#ifdef LENSFIT
    double* dsensum;
    double* osensum;
#endif
};

struct lensums {
    size_t size;
    struct lensum* data;
};


struct lensum* lensum_new(size_t nbin);
struct lensums* lensums_new(size_t nlens, size_t nbin);

// this one we write all the data out in binary format
void lensums_write(struct lensums* lensums, FILE* stream);

// these write the stdout
struct lensum* lensums_sum(struct lensums* lensums);
void lensums_print_sum(struct lensums* lensums);
void lensums_print_one(struct lensums* lensums, size_t index);
void lensums_print_firstlast(struct lensums* lensums);

struct lensums* lensums_delete(struct lensums* lensum);



struct lensum* lensum_new(size_t nbin);
int lensum_read(FILE* stream, struct lensum* lensum);
struct lensum* lensum_copy(struct lensum* lensum);
void lensum_add(struct lensum* dest, struct lensum* src);

void lensum_write(struct lensum* lensum, FILE* stream);
void lensum_print(struct lensum* lensum);
void lensum_clear(struct lensum* lensum);
struct lensum* lensum_delete(struct lensum* lensum);



#endif
