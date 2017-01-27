INCLUDES= -I$(PWD)

LUA= lua

CC= gcc
CCOPT= -O2 -fomit-frame-pointer -g
CCWARN= -Wall
SOCC= $(CC) -shared
SOCFLAGS= -fPIC $(CCOPT) $(CCWARN) $(DEFINES) $(INCLUDES) $(CFLAGS)
SOLDFLAGS= -fPIC $(LDFLAGS)

RM= rm -f

MODNAME= luacore
LIBNAME=sabo_core
MODSO = $(MODNAME).so

all: $(MODNAME) $(LIBNAME) $(MODSO)

macosx:
	$(MAKE) all "SOCC=MACOSX_DEPLOYMENT_TARGET=10.4 $(CC) -dynamiclib -single_module -undefined dynamic_lookup"

$(MODNAME): 
	$(CC) $(MODNAME).c $(SOCFLAGS) -c

$(LIBNAME):
	$(CC) $(LIBNAME).c $(SOCFLAGS) -c

$(MODSO):
	$(SOCC) $(MODNAME).o $(LIBNAME).o $(SOCFLAGS) -o $(MODSO) $^

clean:
	$(RM) *.o *.so core.*

.PHONY: all macosx test clean
