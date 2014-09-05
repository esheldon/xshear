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
# process the sources as a stream on standard input.  Use the indicated config
# file and lens files write to standard output

cat sources1 | xshear config_file lens_file > lensum_file1
cat sources2 | xshear config_file lens_file > lensum_file1

# combine the lensums from the different source catalogs.  This is a "reduction",
# hence the program is named "redshear"

cat lensum_file1 lensum_file2 | redshear config_file > lensum_file_tot


# first apply a filter to a set of source files.  This could be an awk
# command, etc.

cat s1 s2 s3 | src_filter | xshear config_file lens_file > lensum_file
cat s1 s2 s3 | src_filter | xshear config_file lens_file > lensum_file

```

compilation
-----------

python build.py

installation
------------

python build.py --prefix=/some/path install

dependencies
------------

A C compiler supporting gnu 99
