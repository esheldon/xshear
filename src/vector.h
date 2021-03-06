// This header was auto-generated using vectorgen
#ifndef _VECTORGEN_H
#define _VECTORGEN_H

#include <stdint.h>
#include <string.h>

// initial capacity of vectors created with new()
// and capacity of cleared vectors
#define VECTOR_INITCAP 1

// make sure this is an integer for now
#define VECTOR_PUSH_REALLOC_MULTVAL 2

// properties, generic macros
#define vector_size(vec) (vec)->size
#define vector_capacity(vec) (vec)->capacity

// getters and setters, generic macros
// unsafe; maybe make safe?
#define vector_get(vec, i) (vec)->data[i]

#define vector_set(vec, i, val) do {                                         \
    (vec)->data[(i)] = (val);                                                \
} while(0)

// pointer to the underlying data
#define vector_data(vec) (vec)->data

// pointer to beginning
#define vector_begin(vec) (vec)->data

// pointer past end, don't dereference, just use for stopping iteration
#define vector_end(vec) (vec)->data + (vec)->size

// generic iteration over elements.  The iter name is a pointer
// sadly only works for -std=gnu99
//
// vector_foreach(iter, vec) {
//     printf("val is: %%d\n", *iter);
// }

#define vector_foreach(itername, vec)                                        \
    for(typeof((vec)->data) (itername)=vector_begin(vec),                    \
        _iter_end_##itername=vector_end((vec));                              \
        (itername) != _iter_end_##itername;                                  \
        (itername)++)

// frees vec and its data, sets vec==NULL
#define vector_free(vec) do {                                                \
    if ((vec)) {                                                             \
        free((vec)->data);                                                   \
        free((vec));                                                         \
        (vec)=NULL;                                                          \
    }                                                                        \
} while(0)


// perform reallocation on the underlying data vector. This does
// not change the size field unless the new size is smaller
// than the viewed size
//
// note the underlying data will not go below capacity 1
// but if newcap==0 then size will be set to 0

#define vector_realloc(vec, newcap) do {                                    \
    size_t _newcap=(newcap);                                                \
    size_t _oldcap=(vec)->capacity;                                         \
                                                                            \
    if (_newcap < (vec)->size) {                                            \
        (vec)->size=_newcap;                                                \
    }                                                                       \
                                                                            \
    if (_newcap < 1) _newcap=1;                                             \
                                                                            \
    size_t _sizeof_type = sizeof((vec)->data[0]);                           \
                                                                            \
    if (_newcap != _oldcap) {                                               \
        (vec)->data = realloc((vec)->data, _newcap*_sizeof_type);           \
        if (!(vec)->data) {                                                 \
            fprintf(stderr, "failed to reallocate\n");                     \
            exit(1);                                                        \
        }                                                                   \
        if (_newcap > _oldcap) {                                            \
            size_t _num_new_bytes = (_newcap-_oldcap)*_sizeof_type;         \
            memset((vec)->data + _oldcap, 0, _num_new_bytes);               \
        }                                                                   \
                                                                            \
        (vec)->capacity = _newcap;                                          \
    }                                                                       \
} while (0)


// if size > capacity, then a reallocation occurs
// if size <= capacity, then only the ->size field is reset

#define vector_resize(self, newsize) do {                                   \
    if ((newsize) > (self)->capacity) {                                     \
        vector_realloc((self), (newsize));                                  \
    }                                                                       \
    (self)->size=newsize;                                                   \
} while (0)

// reserve at least the specified amount of slots.  If the new capacity is
// smaller than the current capacity, nothing happens.  If larger, a
// reallocation occurs.  No change to current contents occurs.
//
// currently, the exact requested amount is used but in the future we can
// optimize to page boundaries.

#define vector_reserve(self, newcap) do {                                   \
    if ((newcap) > (self)->capacity) {                                      \
        vector_realloc((self), (newcap));                                   \
    }                                                                       \
} while (0)

// set size to zero and realloc to have default initial capacity
#define vector_clear(self) do {                                             \
    vector_realloc((self), VECTOR_INITCAP);                                 \
    (self)->size=0;                                                         \
} while (0)

// push a new element onto the vector
// if reallocation is needed, size is increased by some factor
// unless size is zero, when a fixed amount are allocated

#define vector_push(self, val) do {                                        \
    if ((self)->size == (self)->capacity) {                                \
                                                                           \
        size_t _newsize=0;                                                 \
        if ((self)->capacity == 0) {                                       \
            _newsize=VECTOR_INITCAP ;                                      \
        } else {                                                           \
            _newsize = (self)->capacity*VECTOR_PUSH_REALLOC_MULTVAL;       \
        }                                                                  \
                                                                           \
        vector_realloc((self), _newsize);                                  \
                                                                           \
    }                                                                      \
                                                                           \
    (self)->size++;                                                        \
    (self)->data[self->size-1] = val;                                      \
} while (0)

// pop the last element and decrement size; no reallocation is performed
// if the vector is empty, an error message is printed and garbage is 
// returned
//
// we rely on the fact that capacity never goes to zero, so the "garbage"
// is the zeroth element

#define vector_pop(self) ({                                                  \
    size_t _index=0;                                                         \
    if ((self)->size > 0) {                                                  \
        _index=(self)->size-1;                                               \
        (self)->size--;                                                      \
    } else {                                                                 \
        fprintf(stderr,                                                      \
        "VectorError: attempt to pop from empty vector, returning garbage\n");   \
    }                                                                        \
    (self)->data[_index];                                                    \
})

// add the elements of v2 to v1
// if the vectors are not the same size, then only the smallest
// number are added
#define vector_add_inplace(v1, v2) do {                                    \
    size_t num=0;                                                          \
    size_t n1=vector_size( (v1) );                                         \
    size_t n2=vector_size( (v2) );                                         \
    if (n1 != n2) {                                                        \
        fprintf(stderr,                                                    \
         "VectorWarning: vectors are not the same size, adding subset\n");      \
        if (n1 < n2) {                                                     \
            num=n1;                                                        \
        } else {                                                           \
            num=n2;                                                        \
        }                                                                  \
    } else {                                                               \
        num=n1;                                                            \
    }                                                                      \
    for (size_t i=0; i<num; i++) {                                         \
        (v1)->data[i] += (v2)->data[i];                                    \
    }                                                                      \
} while (0)


// not using foreach here since that requires gnu99
#define vector_add_scalar(self, val) do {                                  \
    for (size_t i=0; i < vector_size( (self) ); i++) {                     \
        (self)->data[i] += (val);                                          \
    }                                                                      \
} while (0)

// multiply the elements of v2 to v1
// if the vectors are not the same size, then only the smallest
// number are multiplied
#define vector_mult_inplace(v1, v2) do {                                   \
    size_t num=0;                                                          \
    size_t n1=vector_size( (v1) );                                         \
    size_t n2=vector_size( (v2) );                                         \
    if (n1 != n2) {                                                        \
        fprintf(stderr,                                                    \
         "warning: vectors are not the same size, multiplying subset\n"); \
        if (n1 < n2) {                                                     \
            num=n1;                                                        \
        } else {                                                           \
            num=n2;                                                        \
        }                                                                  \
    } else {                                                               \
        num=n1;                                                            \
    }                                                                      \
    for (size_t i=0; i<num; i++) {                                         \
        (v1)->data[i] *= (v2)->data[i];                                    \
    }                                                                      \
} while (0)


// not using foreach here since that requires gnu99
#define vector_mult_scalar(self, val) do {                                  \
    for (size_t i=0; i < vector_size( (self) ); i++) {                     \
        (self)->data[i] *= (val);                                          \
    }                                                                      \
} while (0)




typedef struct {
    size_t size;            // number of elements that are visible to the user
    size_t capacity;        // number of allocated elements in data vector
    size_t* data;
} szvector;

// create a new vector with VECTOR_INITCAP capacity and zero visible size
szvector* szvector_new();

// make a new copy of the vector
szvector* szvector_copy(szvector* self);

// make a new vector with data copied from the input array
szvector* szvector_fromarray(size_t* data, size_t size);

// make a vector with the specified initial size, zeroed
szvector* szvector_zeros(size_t num);

//
// these are only written for the builtins
//

// make a vector with the specified initial size, set to 1
szvector* szvector_ones(size_t num);

szvector* szvector_range(long min, long max);

int __szvector_compare_el(const void *a, const void *b);
void szvector_sort(szvector* self);
size_t* szvector_find(szvector* self, size_t el);

typedef struct {
    size_t size;            // number of elements that are visible to the user
    size_t capacity;        // number of allocated elements in data vector
    double* data;
} dvector;

// create a new vector with VECTOR_INITCAP capacity and zero visible size
dvector* dvector_new();

// make a new copy of the vector
dvector* dvector_copy(dvector* self);

// make a new vector with data copied from the input array
dvector* dvector_fromarray(double* data, size_t size);

// make a vector with the specified initial size, zeroed
dvector* dvector_zeros(size_t num);

//
// these are only written for the builtins
//

// make a vector with the specified initial size, set to 1
dvector* dvector_ones(size_t num);

dvector* dvector_range(long min, long max);

int __dvector_compare_el(const void *a, const void *b);
void dvector_sort(dvector* self);
double* dvector_find(dvector* self, double el);

typedef struct {
    size_t size;            // number of elements that are visible to the user
    size_t capacity;        // number of allocated elements in data vector
    int64_t* data;
} lvector;

// create a new vector with VECTOR_INITCAP capacity and zero visible size
lvector* lvector_new();

// make a new copy of the vector
lvector* lvector_copy(lvector* self);

// make a new vector with data copied from the input array
lvector* lvector_fromarray(int64_t* data, size_t size);

// make a vector with the specified initial size, zeroed
lvector* lvector_zeros(size_t num);

//
// these are only written for the builtins
//

// make a vector with the specified initial size, set to 1
lvector* lvector_ones(size_t num);

lvector* lvector_range(long min, long max);

int __lvector_compare_el(const void *a, const void *b);
void lvector_sort(lvector* self);
int64_t* lvector_find(lvector* self, int64_t el);

#endif
