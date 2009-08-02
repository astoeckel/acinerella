acinerella: acinerella.c acinerella.h
	gcc -c -fPIC acinerella.c

ifeq "$(OSTYPE)" "linux"
	gcc -shared -o libacinerella.so acinerella.o -lavformat -lavcodec -lavutil -lm -lswscale
	strip libacinerella.so
else
	gcc -shared -o acinerella.dll acinerella.o -lavformat -lavcodec -lavutil -lm -lswscale -lws2_32
	strip acinerella.dll
endif

test: test.c acinerella.h
	gcc -o test test.c -L. -lacinerella
