// This header was auto-generated
#ifndef _VECTOR_H
#define _VECTOR_H
#include <stdint.h>

#include <stdlib.h>

#ifndef float64
#define float64 double
#endif

struct i64vector {
    size_t size;
    int64_t* data;
};

struct i64vector* i64vector_new(size_t num);
struct i64vector* i64vector_fromarray(int64_t* arr, size_t size);
struct i64vector* i64vector_copy(struct i64vector* vec);
struct i64vector* i64vector_range(size_t num);
void i64vector_resize(struct i64vector* vector, size_t newsize);
int __i64vector_compare_el(const void *a, const void *b);
void i64vector_sort(struct i64vector* vector);

// clears all memory in ->data and sets pointer to NULL
struct i64vector* i64vector_delete(struct i64vector* vector);


struct szvector {
    size_t size;
    size_t* data;
};

struct szvector* szvector_new(size_t num);
struct szvector* szvector_fromarray(size_t* arr, size_t size);
struct szvector* szvector_copy(struct szvector* vec);
struct szvector* szvector_range(size_t num);
void szvector_resize(struct szvector* vector, size_t newsize);
int __szvector_compare_el(const void *a, const void *b);
void szvector_sort(struct szvector* vector);

// clears all memory in ->data and sets pointer to NULL
struct szvector* szvector_delete(struct szvector* vector);


struct f64vector {
    size_t size;
    float64* data;
};

struct f64vector* f64vector_new(size_t num);
struct f64vector* f64vector_fromarray(float64* arr, size_t size);
struct f64vector* f64vector_copy(struct f64vector* vec);
struct f64vector* f64vector_range(size_t num);
void f64vector_resize(struct f64vector* vector, size_t newsize);
int __f64vector_compare_el(const void *a, const void *b);
void f64vector_sort(struct f64vector* vector);

// clears all memory in ->data and sets pointer to NULL
struct f64vector* f64vector_delete(struct f64vector* vector);


#endif  // header guard
