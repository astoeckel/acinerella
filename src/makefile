OS	= $(shell uname -o)
ifeq ($(OS), Linux)
	OS_PREFIX = __LINUX
endif

ifeq ($(OS), Msys)
	OS_PREFIX = __WINDOWS
endif

ifeq ($(OS), Cygwin)
	OS_PREFIX = __WINDOWS
endif

acinerella: acinerella.c acinerella.h
	gcc -c -fPIC acinerella.c

ifeq ($(OS), Linux)
	gcc -D $(OS_PREFIX) -shared -o libacinerella.so acinerella.o -lavformat -lavcodec -lavutil -lm -lswscale -lz -g
	strip libacinerella.so
else
	gcc -D $(OS_PREFIX) -shared -o acinerella.dll acinerella.o -lavformat -lavcodec -lavutil -lm -lswscale -lz -lws2_32 -g
	strip acinerella.dll
endif

test: test.c acinerella.h
	gcc -D $(OS_PREFIX) -o test test.c -L. -lacinerella
