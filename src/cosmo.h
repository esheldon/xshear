#ifndef __COSMOLIB_H
#define __COSMOLIB_H

#define NPTS 5
#define VNPTS 10
#define FOUR_PI_G_OVER_C_SQUARED 6.0150504541630152e-07
#define CLIGHT 2.99792458e5

typedef struct {
    int flat; // is this a flat cosmology?

    double H0;
    double DH; // hubble distance, just c/H0
    double omega_m; // density parameters rho/rhocrit
    double omega_l;
    double omega_k;

    // this is sqrt(abs(omega_k))/DH
    double tcfac;

    double x[NPTS];
    double w[NPTS];

    double vx[VNPTS];
    double vw[VNPTS];
} Cosmo;

Cosmo* cosmo_new(
        double DH, 
        int flat,
        double omega_m,
        double omega_l,
        double omega_k);

void cosmo_print(Cosmo* c);
Cosmo* cosmo_free(Cosmo* c);

double ez_inverse(Cosmo* c, double z);
double ez_inverse_integral(Cosmo* c, double zmin, double zmax);

/* comoving distance in Mpc */
double Dc(Cosmo* c, double zmin, double zmax);

// transverse comoving distance
double Dm(Cosmo* c, double zmin, double zmax);

// angular diameter distances
double Da(Cosmo* c, double zmin, double zmax);

// luminosity distances
double Dl(Cosmo* c, double zmin, double zmax);

// comoving volume element
double dV(Cosmo* c, double z);

// comoving volume between zmin and zmax
double V(Cosmo* c, double zmin, double zmax);

// inverse critical density for lensing
double scinv(Cosmo* c, double zl, double zs);
double scinv_pre(double zl, double dcl, double dcs);

// generate gauss-legendre abcissa and weights
void gauleg(double x1, double x2, int npts, double* x, double* w);


#endif
