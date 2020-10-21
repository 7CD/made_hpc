all: 
	gcc -c matpower.c -O3
	ar rc matpower.a matpower.o
	gcc test.c matpower.a -o test.exe
clean:
	rm *.o *.a *.exe
test:
	./test.exe

