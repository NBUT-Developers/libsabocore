#################################################
# Copyright 2016-2017 Alex(zchao1995@gmail.com) #
#################################################

CC=gcc
CD=cd
CP=cp
TESTAPI=api/lua/
TESTLIB=$(TESTAPI)libsabocore.a
CFLAGS=-O2 -Wall
DY=-shared -fPIC
AR=ar
target=libsabocore
luaso=$(TESTAPI)luacore.so



default: so a

so:
	$(CC) sabo_core.c $(CFLAGS) $(DY) -o $(target).so

a:
	$(CC) sabo_core.c -c $(CFLAGS)
	$(AR) crv $(target).a sabo_core.o

clean:
	rm -f *.so *.a *.o

test:
	$(CP) libsabocore.a $(TESTAPI)
	$(CD) $(TESTAPI) && make
	$(CP) $(luaso) t/
