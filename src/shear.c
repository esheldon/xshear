#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>

#include "defs.h"
#include "shear.h"
#include "sconfig.h"
#include "cosmo.h"
#include "healpix.h"
#include "lens.h"
#include "lensum.h"
#include "source.h"
#include "interp.h"
#include "tree.h"
#include "sdss-survey.h"
#include "quad.h"
#include "log.h"


Shear* shear_init(const char* config_url, const char* lens_url) {

    Shear* shear = calloc(1, sizeof(Shear));
    if (shear == NULL) {
        wlog("Failed to alloc shear struct\n");
        exit(EXIT_FAILURE);
    }

    shear->config=sconfig_read(config_url);

    ShearConfig* config=shear->config;
    wlog("config structure:\n");
    sconfig_print(config);

    // now initialize the structures we need
    wlog("Initalizing cosmo in flat universe\n");
    int flat=1;
    double omega_k=0;
    shear->cosmo = cosmo_new(config->H0, flat, 
                             config->omega_m, 
                             1-config->omega_m, 
                             omega_k);

    wlog("cosmo structure:\n");
    cosmo_print(shear->cosmo);

    wlog("Initalizing healpix at nside: %lld\n", config->healpix_nside);
    shear->hpix = hpix_new(config->healpix_nside);

    shear->lcat = lcat_read(config, lens_url);

    // order is important here
    if (!config->Dlens_input) {
        wlog("Adding Da to lenses\n");
        lcat_add_da(shear->lcat, shear->cosmo);
    } else {

    }

    wlog("Adding cos(search_angle) to lenses\n");
    lcat_add_search_angle(shear->lcat, config->rmax);

    wlog("Intersecting all lenses with healpix at rmax: %lf\n", config->rmax);
    lcat_disc_intersect(shear->lcat, shear->hpix, config->rmax);

    wlog("Building hpix tree for lenses\n");
    lcat_build_hpix_tree(shear->hpix, shear->lcat);

    lcat_print_firstlast(shear->lcat);

    // this holds the sums for each lens
    shear->lensums = lensums_new(shear->lcat->size,
                                 config->nbin,
                                 config->shear_style,
                                 config->scstyle,
                                 config->shear_units);
    shear->tpairs = 0;
    shear->totpairs = calloc(config->nbin, sizeof(int64));
    for (size_t i=0; i<shear->lensums->size; i++) {
        shear->lensums->data[i].index = shear->lcat->data[i].index;
    }

    if (config->scstyle == SIGMACRIT_STYLE_INTERP) {
        // interpolation region
        double* zl=config->zl->data;
        int64 nzl=config->nzl;
        shear->max_zlens = zl[nzl-1];
        shear->min_zlens = fmax(zl[0], shear->config->min_zlens_interp);
    } else {
        shear->min_zlens = 0;
        shear->max_zlens = 9999;
    }

    wlog("min_zlens: %lf  max_zlens: %lf\n", shear->min_zlens, shear->max_zlens);


    return shear;

}

void shear_process_source(Shear* self, Source* src) {
    src->hpixid = hpix_eq2pix(self->hpix, src->ra, src->dec);

    int64 hpix_mod = src->hpixid - self->hpix->half_npix;
    TreeNode* node = tree_find(self->lcat->hpix_tree, hpix_mod);

    if (node == NULL) {
        return;
    }

    Lens* lens=NULL;
    Lensum* lensum=NULL;

    vector_foreach(ind, node->indices) {

        lens = &self->lcat->data[*ind];
        lensum = &self->lensums->data[*ind];

        if (lens->z >= self->min_zlens && lens->z <= self->max_zlens) {
            shear_procpair(self, src, lens, lensum);
        }
    }

}

static void reset_longitude_bounds(double *angle,
                                   double min,
                                   double max)
{
    while (*angle<min) {
        *angle += 360.0;
    }
    while (*angle>=max) {
        *angle -= 360.0;
    }
    return;
}

/*
   r, posangle in radians

   returns 0 if the radius is beyond the max allowed radius as defined in
   lens->cos_search_angle. In that case fewer calculations are performed

*/
static int get_quad_info(const Lens* lens,
                         const Source* src,
                         int *quadrant,
                         double *r_radians,
                         double *posangle_radians)
{
    double cosradiff, sinradiff, arg, cos_r, pa;

    cosradiff = src->cosra*lens->cosra + src->sinra*lens->sinra;
    sinradiff = src->sinra*lens->cosra - src->cosra*lens->sinra;

    // cos of angle separation
    cos_r = lens->sindec*src->sindec + lens->cosdec*src->cosdec*cosradiff;

    if (cos_r < lens->cos_search_angle) {
        // outside search radius
        return 0;
    }

    if (cos_r > 1.0)
        cos_r=1.0;
    else if (cos_r < -1.0)
        cos_r=-1.0;

    // angle of separation in radians
    (*r_radians) = acos(cos_r);

    // position angle
    arg = lens->sindec*cosradiff - lens->cosdec*src->sindec/src->cosdec;

    // -pi,pi
    (*posangle_radians) = atan2(sinradiff, arg) - M_PI_2;

    // degrees -180,180
    pa = (*posangle_radians)*R2D;
    reset_longitude_bounds(&pa, -180.0, 180.0);

    (*quadrant) = quadeq_get_quadrant(pa);

    return 1;
}

/*

  SDSS specific, make sure the source is in an acceptable quadrant for this
  lens

 */
static int shear_test_quad_sdss(Lens* l, Source* s) {
    return test_quad_sincos_sdss(l->maskflags,
                                 l->sinlam, l->coslam,
                                 l->sineta, l->coseta,
                                 s->sinlam, s->coslam,
                                 s->sineta, s->coseta);
}




void shear_procpair(Shear* self, 
                    Source* src, 
                    Lens* lens, 
                    Lensum* lensum) {

    ShearConfig* config=self->config;

    // make sure object is in a pair of unmasked adjacent
    // quadrants.  Using short-circuiting in if statement


    // checks against max radius
    int quadrant;
    double r_radians, posangle_radians;
    int keep=get_quad_info(lens, src,
                           &quadrant,
                           &r_radians,
                           &posangle_radians);

    if (!keep) {
        goto _procpair_bail;
    }

    if (config->mask_style == MASK_STYLE_EQ) {
        if (!quadeq_check_quadrant(lens->maskflags, quadrant)) {
            goto _procpair_bail;
        }
    } else if (config->mask_style == MASK_STYLE_SDSS) {
        if (!shear_test_quad_sdss(lens, src)) {
            goto _procpair_bail;
        }
    }

    double cos2pa = cos(2*posangle_radians);
    double sin2pa = sin(2*posangle_radians);


    // Philosophy:
    // Internal bookkeeping is the same, regardless of the input and intended output format.
    //
    // D=\sum_i(w_i*scinv_i^2*DeltaSigma_i)=\sum_i(w_i*scinv_i*g_i)
    // DeltaSigma=D/W where W=\sum_i(w_i*scinv_i^2) is a surface mass overdensity in physical units (Msol/Mpc^2)
    // g=D/S where S=\sum_i(w_i*scinv_i) is a unitless mean shear, weighted by (w_i*scinv_i)
    //
    // So we always output three columns: D, W=\sum_i(w_i*scinv_i^2) and S=\sum_i(w_i*scinv_i)
    //
    // We allow two different weighting style: config->weight_style=uniform and config->weight_style=optimal
    // 'optimal' will do what's written above, which maximizes the S/N
    // 'uniform' will use the weights w'_i = w_i/scinv_i; in that case, g is a unitless mean shear weighted by w_i 
    //           and DeltaSigma is still a (non-minimum variance) surface mass overdensity in physical units (Msol/Mpc^2)
    //
    // In the code below, we use the variable s for the weight on shear (i.e., D=\sum_i(s_i*g_i)).
    // In weight_style=optimal, s=w_i*scinv_i; in weight_style=uniform, s=w_i
    // 
    // The config->shear_units parameter controls the output format. "deltasig" and "shear" 
    // produce the backward-compatible output of D and W or, respectively, S
    // Output format "both" outputs both and also s_i and s_i*scinv_i-weighted sensitivities


    // (1) set sigma_crit^-1
    double scinv; // the sigma_crit^-1 that goes into S
    double zs;
    switch(config->scstyle) {
        case SIGMACRIT_STYLE_INTERP:
            scinv = interplin(src->zlens, src->scinv, lens->z);
            zs=-1.;
            break;
        case SIGMACRIT_STYLE_POINT:
            if ( (src->z - lens->z) < config->zdiff_min) {
                goto _procpair_bail;
            }
            double dcl = lens->da*(1.+lens->z);
            scinv = scinv_pre(lens->z, dcl, src->dc);
            zs=src->z;
            break;
        case SIGMACRIT_STYLE_SAMPLE:
            if(src->zs <= lens->z) {
                scinv=0.;
            } else {
                double dcl = lens->da*(1.+lens->z);
                scinv = scinv_pre(lens->z, dcl, src->dcs); // at sampled z of the source
            }
            zs=src->zs;
            break;
        default:
            fprintf(stderr, "unknown sigmacrit style %d\n", config->scstyle);
    }
    
    
    // (2) set shear weight s
    double s=src->weight;
    if(config->weight_style==WEIGHT_STYLE_OPTIMAL) {
        switch(config->scstyle) {
            case SIGMACRIT_STYLE_INTERP:
            case SIGMACRIT_STYLE_POINT:
                s *= scinv;
                break;
            case SIGMACRIT_STYLE_SAMPLE:
                if ( (src->z - lens->z) < config->zdiff_min) {
                    goto _procpair_bail;
                }
                double dcl = lens->da*(1.+lens->z);
                s *= scinv_pre(lens->z, dcl, src->dc); // src->dc is dc at the <z>, not the sampled z of the source
                break;
            default:
                fprintf(stderr, "unknown sigmacrit style %ld\n", config->scstyle);
        }
    }

//    fprintf(stdout, "Mpc comoving %d", config->r_units);
    // (3) determine radius
    double r;
    if (config->r_units==UNITS_MPC) {
        // Mpc
        r = r_radians*lens->da;
    } else if (config->r_units==UNITS_MPC_COM){
        // Mpc comoving
//        fprintf(stderr, "Mpc comoving");
        r = r_radians*lens->da*(1.+lens->z);
        s *= (1. + lens->z) * (1. + lens->z);
        scinv *= (1. + lens->z) * (1. + lens->z);
    } else {
        // arcmin
        r = r_radians*R2D*60.;
    }

    double logr = log10(r);
    if (logr < config->log_rmin || logr > config->log_rmax) {
        goto _procpair_bail;
    }

    int rbin = (int)( (logr-config->log_rmin)/config->log_binsize );
    if (rbin < 0 || rbin >= config->nbin) {
        goto _procpair_bail;
    }

    // sign here depends on convention
    //double gt = -(src->g1*cos2pa + src->g2*sin2pa);
    //double gx =  (src->g1*sin2pa - src->g2*cos2pa);
    double gt = -(src->g1*cos2pa - src->g2*sin2pa);
    double gx =  (src->g1*sin2pa + src->g2*cos2pa);

    lensum->weight += s*scinv;
    lensum->totpairs += 1;
    self->totpairs[rbin] += 1;    
    lensum->npair[rbin] += 1;
    lensum->wsum[rbin] += s*scinv;
    lensum->ssum[rbin] += s;
    
    lensum->e1sum[rbin] += s*src->g1;
    lensum->e2sum[rbin] += s*src->g2;
    
    lensum->dsum[rbin] += s*gt;
    lensum->osum[rbin] += s*gx;
    
    lensum->rsum[rbin] += s*scinv*r;

    // sensitivity of gt/gx to shear in that direction
    double gsens_t=1.;
    double gsens_x=1.;

    if (config->shear_style==SHEAR_STYLE_LENSFIT || config->shear_style==SHEAR_STYLE_METACAL) {
        // sensitivity of gt is
        // dgt/dgammat=-dg1/dgammat*cos2pa-dg2/dgammat*sin2pa
        //            =-dg1/dgamma1*dgamma1/dgammat*cos2pa-dg2/dgamma2*dgamma2/dgammat*sin2pa
        //            =g1sens*cos2pa^2+g2sens*sin2pa^2
    
        // the alternative is to just average them
        // double gsens = 0.5*(src->g1sens + src->g2sens); 
        // the only reason to not do that are strong-ish shears, where gsens is significantly
        // different along the tangential direction because tangential shears aren't small

        // note for LENSFIT, g12sens=0, while for METACAL we have measured it        

        gsens_t=src->g1sens*cos2pa*cos2pa+src->g2sens*sin2pa*sin2pa+2.*src->g12sens*sin2pa*cos2pa;
        gsens_x=src->g1sens*sin2pa*sin2pa+src->g2sens*cos2pa*cos2pa+2.*src->g12sens*sin2pa*cos2pa;
    }
    
    // will have to implement something more complex here for metacal, maybe
        
    lensum->dsensum_w[rbin] += s*scinv*gsens_t;
    lensum->osensum_w[rbin] += s*scinv*gsens_x;
    lensum->dsensum_s[rbin] += s*gsens_t;
    lensum->osensum_s[rbin] += s*gsens_x;
    self->tpairs ++;

    // This  change output format based on som cell indexing if necessary
    if (src->sourceid_style == 2) {
        if(rbin<config->pairlog_rmax && rbin>=config->pairlog_rmin &&
          (config->pairlog_nmax<=0 || self->totpairs[rbin]<=config->pairlog_nmax)) {
            fprintf(config->pair_fd, "%ld %ld %d %le %le %le %le\n", lens->index, src->index, rbin, s, scinv, gsens_t, zs);
        }
    } else if (src->sourceid_style == 3) {
        if(rbin<config->pairlog_rmax && rbin>=config->pairlog_rmin &&
          (config->pairlog_nmax<=0 || self->totpairs[rbin]<=config->pairlog_nmax)) {
                fprintf(config->pair_fd, "%ld %ld %ld %d %le %le %le %le\n", lens->index, src->index, src->somcell, rbin, s, scinv, gsens_t, zs);
        }
    };


_procpair_bail:

    return;

}




void shear_print_sum(Shear* self) {
    wlog("Total sums:\n\n");

    lensums_print_sum(self->lensums);

}

// this is for when we haven't written the file line by line
void shear_write(Shear* self, FILE* stream) {
    lensums_write(self->lensums, stream);
}

Shear* shear_free(Shear* self) {

    if (self != NULL) {

        self->config   = sconfig_free(self->config);
        self->lcat     = lcat_free(self->lcat);
        self->hpix     = hpix_free(self->hpix);
        self->cosmo    = cosmo_free(self->cosmo);
        self->lensums  = lensums_free(self->lensums);

    }
    free(self);
    return NULL;
}


