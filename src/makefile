acinerella: acinerella.c acinerella.h
	gcc -c -fPIC acinerella.c
	gcc -shared -o ../bin/acinerella.dll acinerella.o -lavformat -lavcodec -lavutil -lm
test: test.c 
	gcc -o ../bin/test test.c -L. -lacinerella
