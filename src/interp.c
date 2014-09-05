#include "Vector.h"
#include "interp.h"

// the vectors must be sorted according to x
// Note points outside of the range still get interpolated, so be careful!
double f64interplin(const struct f64vector* vx, const struct f64vector* vy, double u) {

    // default to last point
    size_t ilo = vx->size-2;
    size_t ihi = vx->size-1;

    for (size_t i=0; i<vx->size; i++) {

        if (vx->data[i] >= u) {
            if (i == 0) {
                ilo = 0;
                ihi = 1;
            } else {
                ilo = i-1;
                ihi = i;
            }
            break;
        }
    }


    double* x=vx->data;
    double* y=vy->data;
    double val;
    val = ( u-x[ilo] )*( y[ihi] - y[ilo] )/( x[ihi] - x[ilo] ) + y[ilo];

    return val;
}
