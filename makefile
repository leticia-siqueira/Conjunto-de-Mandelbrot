mandelbrot: main.o
	gcc main.o -o mandelbrot -fopenmp -pthread

main.o: main.c
	gcc -c main.c -fopenmp -pthread

clean:
	rm -f *.o mandelbrot

.PHONY:clean