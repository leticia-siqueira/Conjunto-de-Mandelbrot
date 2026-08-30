#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <time.h>

#include <omp.h>

void parteReal_e_complexa(int largura, int altura, int coluna, int linha, double *pixelreal, double *pixelImaginario){

    double minimoReal = -2.0, maximoReal = 1.0;
    double minimoImaginario = -1.5, maximoimaginario = 1.5;

    *pixelreal = minimoReal + ((double)coluna / largura) * (maximoReal - minimoReal);

    *pixelImaginario = minimoImaginario + ((double) linha / altura) * (maximoimaginario - minimoImaginario);
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

long ler_entrada(char *argumento, char *nome){
    char *final_argumento;
    errno = 0;

    long numero = strtol(argumento, &final_argumento, 10);

    if (errno != 0 || *final_argumento != '\0'){
        fprintf(stderr, "%s invalido: %s\n", nome, argumento);
        exit(1);
    }

    return numero;
}

double saida_OpenMP(int numero_largura, int numero_altura, int numero_MAXinteracoes, int *pixel_da_imagem, int numero_threads){
    struct timespec inicio, fim;

    clock_gettime(CLOCK_MONOTONIC, &inicio);
    #pragma omp parallel for num_threads(numero_threads)
    for( int i = 0; i < numero_altura; i++){
        for (int j = 0; j < numero_largura; j++){
            double pixelreal, pixelImaginario;

            parteReal_e_complexa(numero_largura, numero_altura, j, i, &pixelreal, &pixelImaginario);

            int numero_interacoes = interacoes(numero_MAXinteracoes, pixelImaginario, pixelreal);

            pixel_da_imagem[(i * numero_largura) + j] = ((255 * (double)numero_interacoes) / numero_MAXinteracoes);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &fim);

    double tempo_OpenMP = (fim.tv_sec - inicio.tv_sec) + (fim.tv_nsec - inicio.tv_nsec) / 1e9;

    return tempo_OpenMP
}

int main(int argc, char *argv[]){
    
    if (argc < 5 || argc > 5){
        fprintf(stderr, "Quantidade de argumentos invalidos, Coloque apenas Altura Largura MAXinteracoes threads\n");
        exit(1);
    } 

    struct timespec inicio, fim;

    long numero_altura = ler_entrada(argv[1], "altura");

    long numero_largura = ler_entrada(argv[2], "largura");

    long numero_MAXinteracoes = ler_entrada(argv[3], "maximo de interacoes");

    long numero_threads = ler_entrada(argv[4], "threads");

    int *pixel_da_imagem = (int* )malloc((numero_altura * numero_largura)*sizeof(int)); 

    if (pixel_da_imagem == NULL){
        fprintf(stderr, "Erro ao alocar memoria\n");
        return 1;
    }

    clock_gettime(CLOCK_MONOTONIC, &inicio);
    for( int i = 0; i < numero_altura; i++){
        for (int j = 0; j < numero_largura; j++){
            double pixelreal, pixelImaginario;

            parteReal_e_complexa(numero_largura, numero_altura, j, i, &pixelreal, &pixelImaginario);

            int numero_interacoes = interacoes(numero_MAXinteracoes, pixelImaginario, pixelreal);

            pixel_da_imagem[(i * numero_largura) + j] = ((255 * (double)numero_interacoes) / numero_MAXinteracoes);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &fim);
    
    double tempo_Serial = (fim.tv_sec - inicio.tv_sec) + (fim.tv_nsec - inicio.tv_nsec) / 1e9;
    double tempo_OpenMP = saida_OpenMP(numero_largura, numero_largura, numero_MAXinteracoes, &pixel_da_imagem, numero_threads)
    
    FILE *arquivoSerial = fopen("mandelbrot_lmss4_serial.pgm", "w");

    if (arquivoSerial == NULL){
        fprintf(stderr, "Erro ao abrir o arquivo.\n");
        exit(1);
    }

    for (int i = 0; i < numero_altura; i++){
        for (int j = 0; j < numero_largura; j++){
            
            if (j == 0){

                fprintf(arquivoSerial, "%d", pixel_da_imagem[(i * numero_largura) + j]);
            } else{

                fprintf(arquivoSerial, " %d", pixel_da_imagem[(i * numero_largura) + j]);
            }
        }

        fprintf(arquivoSerial, "\n");
    }
    
    fclose(arquivoSerial);



    FILE *ArquivoTempo = fopen("times.txt", "w");

    if (ArquivoTempo == NULL){
        fprintf(stderr, "Erro ao abrir o arquivo\n");
        exit(1);
    }

    fprintf(ArquivoTempo, "Serial: %lfs\n", tempo_Serial);
    fprintf(ArquivoTempo, "OpenMP: %lfs\n", tempo_OpenMP);
    fprintf(ArquivoTempo, "Pthreads1: %lfs\n", tempo_Serial);
    fprintf(ArquivoTempo, "Pthreads2: %lfs", tempo_Serial);

    fclose(ArquivoTempo);
    free(pixel_da_imagem);
    return 0;
}