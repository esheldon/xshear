#ifndef _SOURCE_HEADER
#define _SOURCE_HEADER

#include "vector.h"
#include "defs.h"
#include "sconfig.h"

/* 
  We can work in two modes:
    - we have an inverse critical density; in this case the 
      header of the file must contain the associated zlens values
    - We have a specific z value for this object; we will generate
      the associated dc (comoving distance) value for speed

  Note we actually want the sin and cos of ra/dec rather than ra
  dec for our calculations.
*/
struct source {

    int shear_style;
    int mask_style;
    int scstyle;

    double ra;
    double dec;

    double g1;
    double g2;

    // only used for shear_style==LENSFIT
    double g1sens;
    double g2sens;

    double weight;

    int64 hpixid;

    //
    // these only used when scstyle == SIGMACRIT_STYLE_INTERP
    //

    // note this is same size as zlens kept in 
    // catalog structure.

    dvector* scinv; 

    // For convenience; this should just point to memory owned by
    // config->zlens

    const dvector* zlens; 

    // only used when sigmacrit style == SIGMACRIT_STYLE_POINT
    double z;
    double dc;

    // calculate these for speed
    double sinra; 
    double cosra;
    double sindec;
    double cosdec;

    // only used for mask_style==MASK_STYLE_SDSS
    double sinlam;
    double coslam;
    double sineta;
    double coseta;
};


// n_zlens == 0 indicates we are using "true z" style
struct source* source_new(const struct sconfig* config);

int source_read(FILE* stream, struct source* src);

void source_print(struct source* src);

struct source* source_delete(struct source* src);

#endif
