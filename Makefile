#################################################
# Copyright 2016-2017 Alex(zchao1995@gmail.com) #
#################################################

CC=gcc
CD=cd
CP=cp
CFLAGS=-O2 -Wall
DY=-shared -fPIC
AR=ar
target=libsabocore

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(PWD)/t


default: so a

so:
	$(CC) sabo_core.c $(CFLAGS) $(DY) -o $(target).so

a:
	$(CC) sabo_core.c -c $(CFLAGS)
	$(AR) crv $(target).a sabo_core.o

clean:
	rm -f *.so *.a *.o

test:
	$(CP) $(target).so t/
	$(CD) t/ && luajit test_libsabocore.lua
