# Conjunto-de-Mandelbrot

Objetivo 
A implementação consiste em gerar uma imagem do conjunto de mandelbrot, essa imagem é gerada num plano complexo. Meu código entrega o cálculo para a conversão dos valores na posição do pixel, execução em série, utilizando openmp e pthreads (uma fazendo os cálculos do mandelbrot e a outra na normalização dos números apenas).

Compilar:

make clean

make

Depois rode:
    
    ./mandelbrot [largura] [altura] [MAX_interações] [qtd_threads]
