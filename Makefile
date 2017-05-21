#################################################
# Copyright 2016-2017 Alex(zchao1995@gmail.com) #
#################################################

CC=gcc
CD=cd
CP=cp
CFLAGS=-O2 -Wall -pipe
DY=-shared -fPIC
AR=ar
target=libsabocore
RM=rm

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(PWD)/t


default: so a

so:
	$(CC) sabo_core.c $(CFLAGS) $(DY) -o $(target).so

a:
	$(CC) sabo_core.c -c $(CFLAGS)
	$(AR) crv $(target).a sabo_core.o

lua:
	$(CC) $(CFLAGS) -c -fPIC sabo_core.c
	$(CP) api/lua/lcore.c ./
	$(CC) lcore.c -llua -fPIC -c $(CFLAGS)
	$(RM) lcore.c
	$(CC) $(DY) sabo_core.o lcore.o -o lcore.so


clean:
	rm -f *.so *.a *.o
	rm -f t/*.so

test:
	$(CP) $(target).so t/
	$(CD) t/ && luajit test_libsabocore.lua

.PHONY: so a lua clean test
