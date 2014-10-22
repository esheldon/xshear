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

xshear config_file lens_file < source_file > output_file

# you can parallelize by splitting up the sources.

cat sources1 | xshear config_file lens_file > lensum_file1
cat sources2 | xshear config_file lens_file > lensum_file1

# combine the lens sums from the different source catalogs.  This is a "reduction",
# hence the program is named "redshear"

cat lensum_file1 lensum_file2 | redshear config_file > lensum_file_tot

# first apply a filter to a set of source files.  This could be an awk
# command, etc.

cat s1 s2 s3 | src_filter | xshear config_file lens_file > lensum_file
cat s1 s2 s3 | src_filter | xshear config_file lens_file > lensum_file
```

example config files
---------------------

### Config file using photoz as truth
```
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
#  1: using source z as truth. Implies the last column is z
#  2: using full P(z). Implies source the last N columns are 
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
