#ifndef _SOURCE_HEADER
#define _SOURCE_HEADER

#include <vector>
#include "defs.h"
#include "sconfig.h"

namespace objshear {
    using std::vector;

    /* 
      We can work in two modes:
        - we have an inverse critical density; in this case the 
          header of the file must contain the associated zlens values
        - We have a specific z value for this object; we will generate
          the associated dc (comoving distance) value for speed

      Note we actually want the sin and cos of ra/dec rather than ra
      dec for our calculations.
    */

    struct Source {

        Source () {}

        Source (int mask_style_in) {
            init(MASK_STYLE_NONE);
        }


        Source (int mask_style_in, const vector<double>& zlens) {
            init(mask_style_in, zlens);
        }

        void init(int mask_style_in) {
            mask_style  = mask_style_in;
            scinv_style=SCSTYLE_TRUE;
        }

        void init(int mask_style_in, const vector<double>& zlens_in) {

            mask_style  = mask_style_in;
            scinv_style = SCSTYLE_INTERP;

            long nzl=(long) zlens.size();

            zlens.resize(nzl);
            scinv.resize(nzl);
            for (long i=0; i<nzl; i++) {
                zlens[i] = zlens_in[i];
            }
        }


        double ra;
        double dec;

        double g1;
        double g2;
        double gcov11;
        double gcov12;
        double gcov22;

        double g_sens1;
        double g_sens2;

        long hpixid;

        int mask_style;
        int scstyle;

        // only used when scstyle == SCSTYLE_INTERP
        vector<double> scinv;
        vector<double> zlens;

        // only used when sigmacrit style == SCSTYLE_TRUE
        double z;
        double dc; // for speed

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


    int read(FILE* stream);
    int filter(const SConfig& cfg);
    void print();

} // namespace objshear

#endif
