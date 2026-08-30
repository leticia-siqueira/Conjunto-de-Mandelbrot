mandelbrot: main.o
	gcc main.o -o mandelbrot -fopenmp

main.o: main.c
	gcc -c main.c -fopenmp

clean:
	rm -f *.o mandelbrot

.PHONY:clean