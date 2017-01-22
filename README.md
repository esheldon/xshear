xshear
======

Measure the tangential shear around a set of lenses.  This technique is also
known as cross-correlation shear, hence the name xshear

Runs of the code can easily be parallelized across sources as well as lenses.  

Parallelizing across sources makes sense for cross-correlation shear, because
the source catalog is generally much larger than the lens catalog. The source
catalog can be split into as many small chunks as needed, and each can be
processed on a different cpu across many machines.

Lenses can also be split up and the result simply concatenated.

Example Usage
-------------

```bash
# process the sources and lenses
cat source_file | xshear config_file lens_file > output_file

# you can parallelize by splitting up the sources.
cat sources1 | xshear config_file lens_file > output_file1
cat sources2 | xshear config_file lens_file > output_file2

# combine the lens sums from the different source catalogs using
# the redshear program (red for reduce: this is a map-reduce!).
cat output_file1 output_file2 | redshear config_file > output_file

# first apply a filter to a set of source files.
cat source_file | src_filter | xshear config_file lens_file > output_file

# The src_filter could be an awk command, etc.
cat source_file | awk '($6 > 0.2)' | xshear config_file lens_file > output_file

# xshear will output a list of source-lens pair IDs and weights if you pass an additional
# filename to write this list to as a command line argument; use 
cat source_cat | xshear config_file lens_file pair_logfile > output_file
```

Example Config Files
---------------------

### Config File Using photoz as Truth
```python
# cosmology parameters
H0      = 100.0
omega_m = 0.30

# optional, nside for healpix, default 64
healpix_nside = 64

# masking style, for quadrant cuts. "none", "sdss", "equatorial"
mask_style = "equatorial"

# shear style
#  "reduced": normal reduced shear shapes, source catalog rows are like
#      (id) ra dec g1 g2 weight ...
#  "lensfit": for lensfit with sensitivities
#      (id) ra dec g1 g2 g1sens g2sens weight ...

shear_style = "reduced"

# sigma crit style
#   "point": point z for sources. Implies the last column in source cat is z
#  "interp": Interpolate 1/sigma_crit calculated from full P(z)).
#            Implies last N columns in source cat are 1/sigma_crit(zlens)_i
#  "sample": random sample of p(z). Implies the second-to-last column in
#            source cat is a mean-z estimate to be used for weighting and
#            last column in source cat contains a random sample from the p(z). 

sigmacrit_style = "point"

# weight style
# "uniform": calculate mean shear and mean sigma_crit^-1, weighting sources by the 
#            weight column in the input catalog only
# "optimal": weight sources by the product of weight colum in the input catalog and 
#            expectation value of sigma_crit^-1 for a minimum variance estimate (default)
#
# The output allows you to calculate (weighted) mean shear and DeltaSigma in both styles. 

weight_style = "optimal"

# source id style
#    "none": no source id in first column
#   "index": integer source id in first column

sourceid_style = "index"

# number of logarithmically spaced radial bins to use
nbin = 21

# index of outermost radius bin for which xshear should print pairs to logfile 
# (optional, default is 0 which means no printing)
rbin_print_max = 0

# min and max radius (units default to Mpc, see below)
rmin = 0.02
rmax = 35.15

# units of radius (Mpc or arcmin). If not set defaults to Mpc; alternative is "arcmin"
r_units = "Mpc"

# demand zs > zl + zdiff_min
# optional, used for sigmacrit_style "point" and "sample" (on z_mean for the latter)
zdiff_min       = 0.2
```

### Config File Using \Sigma_{crit}(zlens) Derived from Full P(zsource).   
```python
sigmacrit_style = "interp"

# zlens values for the 1/sigma_crit(zlens) values tabulated for each source
# note the elements of arrays can be separated by either spaces or commas
zlvals = [0.02 0.035 0.05 0.065 0.08 0.095 0.11 0.125 0.14 0.155 0.17 0.185 0.2 0.215 0.23 0.245 0.26 0.275 0.29 0.305 0.32 0.335 0.35 0.365 0.38 0.395 0.41]

```


Format of Lens Catalogs
-----------------------

The format is white-space delimited ascii with no newline at the end.  The columns are

```
index ra dec z maskflags
```
For example:
```
0 239.5554049954774 27.24812220183897 0.09577668458223343 31
1 250.0985461117883 46.70283749181137 0.2325130850076675 31
2 197.8753117014942 -1.345204982754328 0.1821855753660202 7
...
```

The meaning of each column is
```
index:     a user-defined index
ra:        RA in degrees
dec:       DEC in degrees
z:         redshift
maskflags: flags for quadrant checking
```

The maskflags are only used if you set a mask style that is not "none" (no mask
flags)

Format of Source Catalogs
-----------------------
The format is white-space delimited ascii. The columns contained 
depend on the configuration.

You can pass catalogs with an integer source ID in the first column (with 
sourceid_style="index") or no ID (with sourceid_style="none", in which case the first 
column is expected to contain ra).

When using point photozs (sigmacrit_style="point") the format is the following

```
For shear_style="reduced" (using simple reduced shear style)
        (id) ra dec g1 g2 source_weight z

For shear_style="lensfit" (lensfit style)
        (id) ra dec g1 g2 g1sens g2sens source_weight z
```

The format for sigmacrit_style="interp" includes the mean 1/sigma_crit in bins
of lens redshift.

```
For shear_style="reduced" (using simple reduced shear style)
        (id) ra dec g1 g2 source_weight sc_1 sc_2 sc_3 sc_4 ...

For shear_style="lensfit" (lensfit style)
        (id) ra dec g1 g2 g1sens g2sens source_weight sc_1 sc_2 sc_3 sc_4 ...
```

When using sigmacrit_style="sample", the source catalog needs to contain a mean-z 
estimate to be used for weighting and a random sample from the p(z).

```
For shear_style="reduced" (using simple reduced shear style)
        (id) ra dec g1 g2 source_weight z_mean z_sample

For shear_style="lensfit" (lensfit style)
        (id) ra dec g1 g2 g1sens g2sens source_weight z_mean z_sample
```

Meaning of columns:

```
id:            integer source id
ra:            RA in degrees
dec:           DEC in degrees
g1:            shape component 1
g2:            shape component 2
source_weight: a weight for the source
z:             a point estimator (when sigmacrit_style="point")
sc_i:          1/sigma_crit in bins of lens redshift.  The redshift bins
               are defined in "zlvals" config parameter
z_mean:        expectation value of source redshift, used for selecting/weighting the source
z_sample:      random sample from source p(z), used for calculating sigma_crit^-1
```

Format of Output Catalog
------------------------
The output catalog has a row for each lens.  It contains the same columns regardless of
input format or styles selected above.

```
    index weight_tot totpairs npair_i rsum_i wsum_i ssum_i dsum_o osum_i dsensum_w_i osensum_w_i dsensum_s_i osensum_s_i 
```

where

index:      index from lens catalog
weight_tot: sum of all weights for all source pairs in all radial bins
totpairs:   total pairs used
npair_i:    number of pairs in radial bin i.  N columns.
rsum_i=Sum(w_i*r_i), with w_i the weight defined by weight_style
wsum_i=Sum(w_i)
ssum_i=Sum(w_i*(Sigma_crit^-1)_i)
dsum_i=Sum(w_i*(g_t)_i)
osum_i=Sum(w_i*(g_x)_i)
dsensum_w_i=Sum(w_i*(gsens_t)_i), where gsens_t is the sensitivity of the tangential component
osensum_w_i=Sum(w_i*(gsens_x)_i), where gsens_t is the sensitivity of the cross component
dsensum_s_i=Sum(w_i*(Sigma_crit^-1)_i*(gsens_t)_i)
osensum_s_i=Sum(w_i*(Sigma_crit^-1)_i*(gsens_x)_i)

In each case, _i means radial bin i, so there will be Nbins columns for each, ordered by 
radial bin.


### Averaging the \Delta\Sigma and Other Quantities

Just divide the columns.

For example, you can calculate the following w-weighted quantities:

<r>_i                = rsum_i/wsum_i
<\Sigma_{crit}^-1>_i = ssum_i/wsum_i
<e_t>_i              = dsum_i/wsum_i
<e_x>_i              = osum_i/wsum_i
<g_t>_i              = dsum_i/dsensum_w_i
<g_x>_i              = dsum_i/osensum_w_i
<\Delta\Sigma>_i     = dsum_i/dsensum_s_i
<\Delta\Sigma_x>_i   = osum_i/osensum_s_i

Note that in shear_style = "reduced", the *sens* columns use gsens_i=1, so you can always
calculate mean shears <g_t> and DeltaSigma as defined above.


compilation
-----------

```bash
# default build uses gcc
make

# use intel C compiler.
make CC=icc
```

install/uninstall
-----------------

```bash
make install prefix=/some/path
make uninstall prefix=/some/path
```

dependencies
------------

A C99 compliant compiler.
