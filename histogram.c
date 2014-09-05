#include <stdint.h>
#include "histogram.h"
#include "Vector.h"
#include "stack.h"


// special simplified binsize=1, min=min(data), integer histogrammer
// These simplifications are significant
void i64hist1(const struct i64vector* vec,
              const struct szvector* sort_index,
              struct i64vector* h,
              struct szvector* rev) {


    int64_t* vdata = vec->data;
    size_t* si = sort_index->data;

    int64_t binmin = vdata[si[0]];
    int64_t binmax = vdata[si[sort_index->size-1]];

    size_t nbin = (binmax-binmin) + 1;
    size_t nrev = vec->size + nbin + 1;

    i64vector_resize(h, nbin);
    szvector_resize(rev, nrev);

    // use int64, size_t is unsigned
    int64_t binnum_old = -1;

    for (size_t i=0; i<vec->size; i++) {
        size_t data_index = si[i];

        size_t offset = i + nbin + 1;
        rev->data[offset] = data_index;

        int64_t binnum = vdata[data_index]-binmin;

        // update the reverse indices
        //
        // remember, we are working on sorted data, so we
        // can just check the binnum to tell where we are
        if (binnum > binnum_old) {
            int64_t tbin = binnum_old + 1;
            while (tbin <= binnum) {
                rev->data[tbin] = offset;
                tbin++;
            }
        }

        // Update the histogram
        h->data[binnum] += 1;
        binnum_old = binnum;

    }

    // make sure the last bin is properly dealt with
    int64_t tbin = binnum_old + 1;
    while (tbin <= nbin) {
        rev->data[tbin] = rev->size;
        tbin++;
    }

}

// when the stack is already sorted and just want the reverse indices
struct szvector* i64getrev(const struct i64stack* vec) {


    int64_t* vdata = vec->data;

    int64_t binmin = vdata[0];
    int64_t binmax = vdata[vec->size-1];

    size_t nbin = (binmax-binmin) + 1;
    size_t nrev = vec->size + nbin + 1;

    struct szvector* rev = szvector_new(nrev);

    // use int64, size_t is unsigned
    int64_t binnum_old = -1;

    for (size_t i=0; i<vec->size; i++) {

        size_t offset = i + nbin + 1;
        rev->data[offset] = i;

        int64_t binnum = vdata[i]-binmin;

        // update the reverse indices
        //
        // remember, we are working on sorted data, so we
        // can just check the binnum to tell where we are
        if (binnum > binnum_old) {
            int64_t tbin = binnum_old + 1;
            while (tbin <= binnum) {
                rev->data[tbin] = offset;
                tbin++;
            }
        }

        // Update the histogram
        binnum_old = binnum;

    }

    // make sure the last bin is properly dealt with
    int64_t tbin = binnum_old + 1;
    while (tbin <= nbin) {
        rev->data[tbin] = rev->size;
        tbin++;
    }

    return rev;
}

