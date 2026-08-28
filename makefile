mandelbrot: main.o
	gcc main.o -o mandelbrot

main.o: main.c
	gcc -c main.c 

clean:
	rm -f *.o mandelbrot

.PHONY:clean