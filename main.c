#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>

#include <omp.h>

void parteReal_e_complexa(int largura, int altura, int coluna, int linha, double *pixelreal, double *pixelImaginario){

    double minimoReal = -2.0, maximoReal = 1.0;
    double minimoImaginario = -1.5, maximoimaginario = 1.5;

    *pixelreal = minimoReal + (coluna / (largura - 1)) * (maximoReal - minimoReal);

    *pixelImaginario = minimoImaginario + (linha / (altura - 1)) * (maximoimaginario - minimoImaginario);
}

int interacoes(int MAX_interacoes, double pixelImaginario, double pixelreal){

    int contador_interacoes = 0;

    double valorReal = 0, Valorimaginario = 0;
    double novo_valorImaginario = 0, novo_valorReal = 0;

    for (int i = 0; i < MAX_interacoes; i++){
        if ((valorReal*valorReal) + (Valorimaginario*Valorimaginario) > 4){
            return contador_interacoes;
        }

        novo_valorReal = (valorReal*valorReal) - (Valorimaginario*Valorimaginario) + pixelreal;
        novo_valorImaginario = (2*valorReal*Valorimaginario) + pixelImaginario;

        valorReal = novo_valorReal;
        Valorimaginario = novo_valorImaginario;

        contador_interacoes++;
    }

    return contador_interacoes;
}


int main(int argc, char *argv[]){
    
    double cr, ci;

    parteReal_e_complexa(800, 600, 200, 150, &cr, &ci);
    printf("cr = %f, ci = %f\n", cr, ci);

    int n = interacoes(1000, 0.0, 1.0);
    printf("iteracoes = %d\n", n);

    return 0;
}