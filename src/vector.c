// This file was auto-generated using vectorgen
// most array methods are generic, see vector.h

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <float.h>
#include "vector.h"

szvector* szvector_new() {
    szvector* self = calloc(1,sizeof(szvector));
    if (self == NULL) {
        fprintf(stderr,"Could not allocate szvector\n");
        return NULL;
    }

    self->capacity = VECTOR_INITCAP;

    self->data = calloc(self->capacity, sizeof(size_t));
    if (self->data == NULL) {
        fprintf(stderr,"Could not allocate data for vector\n");
        exit(1);
    }

    return self;
}


szvector* szvector_copy(szvector* self) {
    szvector* vcopy=szvector_new();
    vector_resize(vcopy, self->size);

    if (self->size > 0) {
        memcpy(vcopy->data, self->data, self->size*sizeof(size_t));
    }

    return vcopy;
}

szvector* szvector_fromarray(size_t* data, size_t size) {
    szvector* self=szvector_new();
    vector_resize(self, size);

    if (self->size > 0) {
        memcpy(self->data, data, size*sizeof(size_t));
    }

    return self;
}

szvector* szvector_zeros(size_t num) {

    szvector* self=szvector_new();
    vector_resize(self, num);
    return self;
}


szvector* szvector_ones(size_t num) {

    szvector* self=szvector_new();
    for (size_t i=0; i<num; i++) {
        vector_push(self,1);
    }
    return self;
}

szvector* szvector_range(long min, long max) {

    szvector* self=szvector_new();
    for (long i=min; i<max; i++) {
        vector_push(self,i);
    }
    
    return self;
}

int __szvector_compare_el(const void *a, const void *b) {
    int64_t temp = 
        (  (int64_t) *( (size_t*)a ) ) 
         -
        (  (int64_t) *( (size_t*)b ) );
    if (temp > 0)
        return 1;
    else if (temp < 0)
        return -1;
    else
        return 0;
}


void szvector_sort(szvector* self) {
    qsort(self->data, self->size, sizeof(size_t), __szvector_compare_el);
}
size_t* szvector_find(szvector* self, size_t el) {
    return (size_t*) bsearch(&el, self->data, self->size, sizeof(size_t), __szvector_compare_el);
}

dvector* dvector_new() {
    dvector* self = calloc(1,sizeof(dvector));
    if (self == NULL) {
        fprintf(stderr,"Could not allocate dvector\n");
        return NULL;
    }

    self->capacity = VECTOR_INITCAP;

    self->data = calloc(self->capacity, sizeof(double));
    if (self->data == NULL) {
        fprintf(stderr,"Could not allocate data for vector\n");
        exit(1);
    }

    return self;
}


dvector* dvector_copy(dvector* self) {
    dvector* vcopy=dvector_new();
    vector_resize(vcopy, self->size);

    if (self->size > 0) {
        memcpy(vcopy->data, self->data, self->size*sizeof(double));
    }

    return vcopy;
}

dvector* dvector_fromarray(double* data, size_t size) {
    dvector* self=dvector_new();
    vector_resize(self, size);

    if (self->size > 0) {
        memcpy(self->data, data, size*sizeof(double));
    }

    return self;
}

dvector* dvector_zeros(size_t num) {

    dvector* self=dvector_new();
    vector_resize(self, num);
    return self;
}


dvector* dvector_ones(size_t num) {

    dvector* self=dvector_new();
    for (size_t i=0; i<num; i++) {
        vector_push(self,1);
    }
    return self;
}

dvector* dvector_range(long min, long max) {

    dvector* self=dvector_new();
    for (long i=min; i<max; i++) {
        vector_push(self,i);
    }
    
    return self;
}

int __dvector_compare_el(const void *a, const void *b) {
    double temp = 
        (  (double) *( (double*)a ) ) 
         -
        (  (double) *( (double*)b ) );
    if (temp > 0)
        return 1;
    else if (temp < 0)
        return -1;
    else
        return 0;
}


void dvector_sort(dvector* self) {
    qsort(self->data, self->size, sizeof(double), __dvector_compare_el);
}
double* dvector_find(dvector* self, double el) {
    return (double*) bsearch(&el, self->data, self->size, sizeof(double), __dvector_compare_el);
}

lvector* lvector_new() {
    lvector* self = calloc(1,sizeof(lvector));
    if (self == NULL) {
        fprintf(stderr,"Could not allocate lvector\n");
        return NULL;
    }

    self->capacity = VECTOR_INITCAP;

    self->data = calloc(self->capacity, sizeof(int64_t));
    if (self->data == NULL) {
        fprintf(stderr,"Could not allocate data for vector\n");
        exit(1);
    }

    return self;
}


lvector* lvector_copy(lvector* self) {
    lvector* vcopy=lvector_new();
    vector_resize(vcopy, self->size);

    if (self->size > 0) {
        memcpy(vcopy->data, self->data, self->size*sizeof(int64_t));
    }

    return vcopy;
}

lvector* lvector_fromarray(int64_t* data, size_t size) {
    lvector* self=lvector_new();
    vector_resize(self, size);

    if (self->size > 0) {
        memcpy(self->data, data, size*sizeof(int64_t));
    }

    return self;
}

lvector* lvector_zeros(size_t num) {

    lvector* self=lvector_new();
    vector_resize(self, num);
    return self;
}


lvector* lvector_ones(size_t num) {

    lvector* self=lvector_new();
    for (size_t i=0; i<num; i++) {
        vector_push(self,1);
    }
    return self;
}

lvector* lvector_range(long min, long max) {

    lvector* self=lvector_new();
    for (long i=min; i<max; i++) {
        vector_push(self,i);
    }
    
    return self;
}

int __lvector_compare_el(const void *a, const void *b) {
    int64_t temp = 
        (  (int64_t) *( (int64_t*)a ) ) 
         -
        (  (int64_t) *( (int64_t*)b ) );
    if (temp > 0)
        return 1;
    else if (temp < 0)
        return -1;
    else
        return 0;
}


void lvector_sort(lvector* self) {
    qsort(self->data, self->size, sizeof(int64_t), __lvector_compare_el);
}
int64_t* lvector_find(lvector* self, int64_t el) {
    return (int64_t*) bsearch(&el, self->data, self->size, sizeof(int64_t), __lvector_compare_el);
}
