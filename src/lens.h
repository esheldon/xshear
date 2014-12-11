#ifndef _LENS_HEADER
#define _LENS_HEADER

#include "cosmo.h"
#include "tree.h"
#include "defs.h"
#include "healpix.h"

typedef struct {

    int64 index;
    double ra;
    double dec;
    double z;

    double da;

    double sinra;
    double cosra;
    double sindec;
    double cosdec;

    int64 maskflags;

    // We will fill these with the healpix pixels within
    // the search radius for each lens
    lvector* hpix;
    //struct szvector* rev;

    double cos_search_angle;

    double sinlam;
    double coslam;
    double sineta;
    double coseta;

} Lens;


typedef struct {
    size_t size;
    Lens* data;

    // a tree to tell us which lenses were intersected
    // with what pixels.  Can search by pixel value to
    // get the indices of the associated lenses
    TreeNode* hpix_tree;
} LensCatalog;

LensCatalog* lcat_new(size_t n_lens);
LensCatalog* lcat_read(const char* lens_url); 

#ifdef HDFS
#include "hdfs_lines.h"
LensCatalog* hdfs_lcat_read(const char* lens_url); 
#endif


void lcat_add_da(LensCatalog* lcat, Cosmo* cosmo);
void lcat_add_search_angle(LensCatalog* lcat, double rmax);

void lcat_print_one(LensCatalog* lcat, size_t el);
void lcat_print_firstlast(LensCatalog* lcat);

LensCatalog* lcat_delete(LensCatalog* lcat);


void lcat_disc_intersect(LensCatalog* lcat, HealPix* hpix, double rmax);
void lcat_build_hpix_tree(HealPix* hpix, LensCatalog* lcat);

#endif
