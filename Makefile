CC := gcc
prefix := /usr/local

CFLAGS =-std=c99 -Wall -Werror -O2
LDFLAGS=-lm

SRCDIR  =./src

XSHEAR_BASE = xshear
REDSHEAR_BASE = redshear

XSHEAR = $(SRCDIR)/$(XSHEAR_BASE)
REDSHEAR = $(SRCDIR)/$(REDSHEAR_BASE)

INSTALL_BIN_DIR=$(prefix)/bin

INSTALLED_PROGS_BASE = $(addprefix $(INSTALL_BIN_DIR)/,$(XSHEAR_BASE) $(REDSHEAR_BASE))

LOCAL_PROGS=$(XSHEAR) $(REDSHEAR)

SOURCES = $(wildcard src/*.c)
OBJS    = $(SOURCES:.c=.o)

XSHEAR_OBJ = 	$(addprefix $(SRCDIR)/,sconfig.o config.o stack.o Vector.o source.o \
						lens.o cosmo.o healpix.o \
						shear.o lensum.o histogram.o tree.o interp.o urls.o \
						xshear.o sdss-survey.o quad.o)

REDSHEAR_OBJ = 	$(addprefix $(SRCDIR)/,healpix.o cosmo.o tree.o stack.o lens.o lensum.o  \
						sconfig.o config.o  \
						urls.o Vector.o  \
						util.o  \
						redshear.o sdss-survey.o)

default: all

all: $(LOCAL_PROGS)

$(XSHEAR): $(XSHEAR_OBJ)
	$(CC) -o $@ $(XSHEAR_OBJ) $(LDFLAGS) 

$(REDSHEAR): $(REDSHEAR_OBJ)
	$(CC) -o $@ $(REDSHEAR_OBJ) $(LDFLAGS) 

# pull in the dependencies
-include $(OBJS:.o=.d)

install: $(LOCAL_PROGS)
	mkdir -p $(INSTALL_BIN_DIR)
	cp $(LOCAL_PROGS) $(INSTALL_BIN_DIR)/

uninstall:
	rm -f $(INSTALLED_PROGS_BASE)

# magick from http://scottmcpeak.com/autodepend/autodepend.html
%.o : %.c
	$(CC) -c $(CFLAGS) $*.c -o $*.o
	$(CC) -MM $(CFLAGS) $*.c > $*.dep
	@mv -f $*.dep $*.dep.tmp
	@sed -e 's|.*:|$*.o:|' < $*.dep.tmp > $*.dep
	@sed -e 's/.*://' -e 's/\\$$//' < $*.dep.tmp | fmt -1 | \
		sed -e 's/^ *//' -e 's/$$/:/' >> $*.dep
	@rm -f $*.dep.tmp

clean:
	rm -f $(LOCAL_PROGS) $(SRCDIR)/*.o $(SRCDIR)/*.dep
