xshear
======

Measure the tangential shear around a set of lenses.  This technique is also
known as cross-correlation shear, hence the name xshear

The code is set up to parallelize across sources rather than lenses.  This
makes sense for cross-correlation shear, since the source catalog is generally
much larger than the lense catalog. The source catalog can be split into as
many small chunks as needed, and each can be processed on a different cpu
across many machines.

example
-------

```bash
# process the sources and lenses
cat source_file | xshear config_file lens_file > output_file

# you can parallelize by splitting up the sources.
cat sources1 | xshear config_file lens_file > output_file1
cat sources2 | xshear config_file lens_file > output_file1

# combine the lens sums from the different source catalogs.
cat output_file1 output_file2 | redshear config_file > output_file

# first apply a filter to a set of source files.  This could be an awk
# command, etc.
cat source_file | src_filter | xshear config_file lens_file > output_file
```

example config files
---------------------

### Config file using photoz as truth
```python
# cosmology parameters
H0                = 100.0
omega_m           = 0.25

# nside for healpix
nside             = 64

# masking style, for quadrant cuts. 1 none, 2 SDSS, 3 equatorial
mask_style        = 3

# shear style
#  1: ordinary reduced shear, source catalog rows are like
#      ra dec g1 g2 weight ...
#  2: for lensfit with sensitivities
#      ra dec g1 g2 g1sens g2sens weight ...

shear_style       = 2

# sigma crit style
#  1: using source z as truth. Implies the last column in source cat is z
#  2: using full P(z). Implies last N columns in source cat are 
#     \Sigma_{crit}(zlens)_i

sigmacrit_style   = 1

# number of logarithmically spaced radial bins to use
nbin              = 21

# min and max radius in Mpc
rmin              = 0.02
rmax              = 38

# demand zs > zl + zdiff_min
# optional
zdiff_min         = 0.2
```

### Config file using \Sigma_{crit}(zlens) derived from full P(zsource).   
```python
# sigma crit style
#  1: using source z as truth. Implies the last column is z
#  2: using full P(z). Implies source the last N columns are 
#     \Sigma_{crit}(zlens)_i

sigmacrit_style   = 2

# zlens values for the \Sigam_{crit}(zlens) values tabulated for each source
# note the elements of arrays can be separated by either spaces or commas
zlvals = [0.02 0.035 0.05 0.065 0.08 0.095 0.11 0.125 0.14 0.155 0.17 0.185 0.2 0.215 0.23 0.245 0.26 0.275 0.29 0.305 0.32 0.335 0.35 0.365 0.38 0.395 0.41]

```

format of lens catalogs
-----------------------

The format is white-space delimited ascii.  The columns are

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
The maskflags are only used if you set a mask style that is not 1 (no mask flags)

format of source catalogs
-----------------------
The format is white-space delimited ascii. The columns contained 
depend on the configuration.

When using point photozs (sigmacrit_style=1) the format is the following

```
For shear_style=1 (using simple reduced shear style)
        ra dec g1 g2 weight z

For shear_style=2 (lensfit style)
        ra dec g1 g2 g1sens g2sens weight z
```

The format for sigmacrit_style=2 includes the mean \Sigma_{crit} in
bins of lens redshift.

```
For shear_style=1 (using simple reduced shear style)
        ra dec g1 g2 weight sc_1 sc_2 sc_3 sc_4 ...

For shear_style=2 (lensfit style)
        ra dec g1 g2 g1sens g2sens weight sc_1 sc_2 sc_3 sc_4 ...
```

Meaning of columns:

```
ra:     RA in degrees
dec:    DEC in degrees
g1:     shape component 1
g2:     shape component 2
weight: a weight for the object
z:      a point estimator (when sigmacrit_style=1)
sc_i:   \Sigma_{crit} in bins of lens redshift.  The redshift bins
        are defined in "zlvals" config parameter
```

Format of Output Catalog
------------------------
For shear_style=1, ordinary reduced shear style
```
    index weight totpairs npair_i rsum_i wsum_i dsum_i osum_i
```

For shear style=2, lensfit style
```
    index weight totpairs npair_i rsum_i wsum_i dsum_i osum_i dsensum_i osensum_i
```

Where _i means radial bin i, so there will be Nbins columns for each.

Meaning of columns. In the following, the weight is the weight of the source
times 1/\Sigma_{crit}^2

```
index:     index from lens catalog
weight:    sum of all weights for all source pairs
totpairs:  total pairs used
npair_i:   number of pairs in radial bin i.  N columns.
rsum_i:    sum of radius in radial bin i
wsum_i:    sum of weights in radial bin i
dsum_i:    sum of \Delta\Sigma_+ * weights in radial bin i.
osum_i:    sum of \Delta\Sigma_x * weights in  radial bin i.
dsensum_i: sum of gsens_+ * weights in radial bin i.
osensum_i: sum of gsens_x * weights in  radial bin i.
```

### Averaging the \Delta\Sigma and other quantities

Just divide the columns.  For shear_style=1 (ordinary reduced shear)
```
    <r> = rsum_i/wsum_i
    <\Delta\Sigma>_+^i = dsum_i/wsum_i
    <\Delta\Sigma>_x^i = osum_i/wsum_i
```
For shear_style=2, lensfit style
```
    <\Delta\Sigma>_+^i = dsum_i/dsensum_i
    <\Delta\Sigma>_x^i = osum_i/osensum_i
```

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
