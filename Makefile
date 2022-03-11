
objects = main.o ImageReadWrite.o ImageProcess.o ImageMinutiaExtract.o GetFeature.o Stack.o ImageMatch.o

test : $(objects) main.h
	gcc -o test $(objects) -lm

main.o : main.c main.h
	gcc -c main.c -o main.o
ImageReadWrite.o : ImageReadWrite.c ImageReadWrite.h
	gcc -c ImageReadWrite.c -o ImageReadWrite.o
ImageProcess.o : ImageProcess.c ImageProcess.h
	gcc -c ImageProcess.c -o ImageProcess.o
ImageMinutiaExtract.o : ImageMinutiaExtract.c ImageMinutiaExtract.h
	gcc -c ImageMinutiaExtract.c -o ImageMinutiaExtract.o
GetFeature.o : GetFeature.c GetFeature.h
	gcc -c GetFeature.c -o GetFeature.o
Stack.o : Stack.c Stack.h
	gcc -c Stack.c -o Stack.o
ImageMatch.o : ImageMatch.c
	gcc -c ImageMatch.c -o ImageMatch.o

.PHONY : clean
clean :
	-rm $(objects) test
