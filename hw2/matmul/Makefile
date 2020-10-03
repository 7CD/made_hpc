all: 
	gcc -c mult.c -O3
	gcc -c random.c
	gcc -c main.c
	ar rc lib.a mult.o random.o
	gcc main.c lib.a -o run.exe
clean:
	rm *.o lib.a *.exe
run:
	./run.exe

