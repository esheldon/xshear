from fabricate import *
import sys, os
import optparse

import glob


parser = optparse.OptionParser()
# make an options list, also send to fabricate
optlist=[optparse.Option('--prefix','-p', default=sys.exec_prefix, help="where to install"),
         optparse.Option('-C', default='gcc', help="compiler to use, default %default.  Can be a full path.")]
parser.add_options(optlist)

options,args = parser.parse_args()
prefix=os.path.expanduser( options.prefix )

CC=options.C

LINKFLAGS=['-lm']

CFLAGS=['-std=c99','-Wall','-Werror','-O2']

xshear_sources = ['sconfig', 'config', 'stack', 'Vector','source',
                  'lens','cosmo','healpix',
                  'shear','lensum','histogram','tree','interp','urls',
                  'xshear','sdss-survey','quad']
redshear_sources = ['healpix','cosmo','tree','stack','lens','lensum',
                    'sconfig','config',
                    'urls','Vector',
                    'util',
                    'redshear','sdss-survey']

xshear_sources = [os.path.join('src',n) for n in xshear_sources]
redshear_sources = [os.path.join('src',n) for n in redshear_sources]



# base names
programs = [{'name':'xshear',   'sources':xshear_sources},
            {'name':'redshear', 'sources':redshear_sources}]
for p in programs:
    p['name'] = os.path.join('src',p['name'])

install_targets = [(prog['name'],'bin') for prog in programs]


def build():
    compile()
    link()

def compile():
    for prog in programs:
        for source in prog['sources']:
            run(CC, '-c', '-o',source+'.o', CFLAGS, source+'.c')

def link():
    for prog in programs:
        objects = [s+'.o' for s in prog['sources']]
        run(CC,'-o', prog['name'], objects, LINKFLAGS)

def clean():
    autoclean()


def install():
    import shutil

    # make sure everything is built first
    build()

    for target in install_targets:
        (name,subdir) = target
        subdir = os.path.join(prefix, subdir)
        if not os.path.exists(subdir):
            os.makedirs(subdir)

        dest=os.path.join(subdir, os.path.basename(name))
        sys.stdout.write("install: %s\n" % dest)
        shutil.copy(name, dest)

# send options so it won't crash on us
main(extra_options=optlist)

