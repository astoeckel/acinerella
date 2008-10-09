acinerella: acinerella.c acinerella.h
	gcc -c -fPIC acinerella.c
	gcc -shared -o acinerella.dll acinerella.o -lavformat -lavcodec -lavutil -lm -lws2_32
test: test.c acinerella.h
	gcc -o test test.c -L. -lacinerella
