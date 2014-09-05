xshear
======

Measure the tangential shear around a set of lenses.  This technique is also
known as cross-correlation shear, hence the name xshear

example
-------

# process the sources as a stream on standard input.  Use the indicated config
# file and lens files write to standard output

cat sources | xshear config_file lens_file > lensum_file

# first apply a filter to a set of source files.  This could be an awk
# command, etc.

cat s1 s2 s3 | src_filter | xshear config_file lens_file > lensum_file

compilation
-----------

python build.py

installation
------------

python build.py --prefix=/some/path install
