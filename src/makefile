acinerella: acinerella.c acinerella.h
	gcc -c -fPIC acinerella.c
	gcc -shared -o acinerella.dll acinerella.o -lavformat -lavcodec -lavutil -lm
test: test.c 
	gcc -o test test.c -L. -lacinerella
