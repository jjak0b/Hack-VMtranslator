VMtranslator: main.o list.o utility.o
	gcc -o VMtranslator main.o list.o utility.o

main.o: main.c list.h utility.h
	gcc -c main.c

list.o: list.c list.h
	gcc -c list.c

utility.o: utility.c utility.h
	gcc -c utility.c

clean:
	rm *.o VMtranslator