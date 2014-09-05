xshear
======

Measure the tangential shear around a set of lenses.  This technique is also
known as cross-correlation shear, hence the name xshear

compilation
-----------

# for averaging using straight shears (ala im3shape)
python build.py

# for lensfit style
python build.py --lensfit

installation
------------
python build.py --prefix=/some/path install
python build.py --lensfit --prefix=/some/path install
