acinerella: acinerella.c acinerella.h
	gcc -c -fPIC acinerella.c

ifeq ($(OS_ARCH),UNIX)
	gcc -shared -o libacinerella.so acinerella.o -lavformat -lavcodec -lavutil -lm -lswscale
else
	gcc -shared -o acinerella.dll acinerella.o -lavformat -lavcodec -lavutil -lm -lswscale -lws2_32
endif

test: test.c acinerella.h
	gcc -o test test.c -L. -lacinerella
