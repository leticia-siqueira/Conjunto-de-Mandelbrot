#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <time.h>

#include <omp.h>
#include <pthread.h>

typedef struct Estrutura_pthread{
    int altura;
    int largura;
    int max_iteracoes;
    int linha_inicial;
    int linha_final;
    int *pixel_da_imagem;
} Estrutura_pthread;

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
    FILE *Erros_ler_entrada = fopen("erros.txt", "a");

    if (Erros_ler_entrada == NULL){
        exit(1);
    }

    char *final_argumento;
    errno = 0;

    long numero = strtol(argumento, &final_argumento, 10);

    if (errno != 0 || *final_argumento != '\0'){
        fprintf(Erros_ler_entrada, "%s invalido(a): %s\n", nome, argumento);
        exit(1);
    }

    return numero;
}

double saida_Serial(int numero_largura, int numero_altura, int numero_MAXinteracoes, int *pixel_da_imagem){
    
    struct timespec inicio, fim;

    FILE *Erros_serial = fopen("erros.txt", "a");

    if (Erros_serial == NULL){
        exit(1);
    }

    clock_gettime(CLOCK_MONOTONIC, &inicio);
    for( int i = 0; i < numero_altura; i++){
        for (int j = 0; j < numero_largura; j++){
            double pixelreal, pixelImaginario;

            parteReal_e_complexa(numero_largura, numero_altura, j, i, &pixelreal, &pixelImaginario);

            int numero_interacoes = interacoes((int)numero_MAXinteracoes, pixelImaginario, pixelreal);

            pixel_da_imagem[(i * numero_largura) + j] = ((255 * (double)numero_interacoes) / numero_MAXinteracoes);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &fim);
    
    double tempo_Serial = (fim.tv_sec - inicio.tv_sec) + (fim.tv_nsec - inicio.tv_nsec) / 1e9;

    FILE *arquivoSerial = fopen("mandelbrot_lmss4_serial.pgm", "w");

    if (arquivoSerial == NULL){
        fprintf(Erros_serial, "Erro ao abrir o arquivo Serial.\n");
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

        if (i != numero_altura - 1){
            fprintf(arquivoSerial, "\n");
        }
    }
    
    fclose(arquivoSerial);

    return tempo_Serial;
}

double saida_OpenMP(int numero_largura, int numero_altura, int numero_MAXinteracoes, int *pixel_da_imagem, int numero_threads){
    
    struct timespec inicio, fim;

    FILE *Erros_OpenMP = fopen("erros.txt", "a");

    if (Erros_OpenMP == NULL){
        exit(1);
    }

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

    FILE *arquivoOpenMP = fopen("mandelbrot_lmss4_openmp.pgm", "w");

    if (arquivoOpenMP == NULL){
        fprintf(Erros_OpenMP, "Erro ao abrir o arquivo OpenMP.\n");
        exit(1);
    }

    for (int i = 0; i < numero_altura; i++){
        for (int j = 0; j < numero_largura; j++){
            
            if (j == 0){

                fprintf(arquivoOpenMP, "%d", pixel_da_imagem[(i * numero_largura) + j]);
            } else{

                fprintf(arquivoOpenMP, " %d", pixel_da_imagem[(i * numero_largura) + j]);
            }
        }

        if (i != numero_altura - 1){
            fprintf(arquivoOpenMP, "\n");
        }
    }
    
    fclose(arquivoOpenMP);

    return tempo_OpenMP;
}

void *calcula_pixel_PThreads(void *struct_pthreads){
    Estrutura_pthread *dados = (Estrutura_pthread *)struct_pthreads;

    for (int i = dados->linha_inicial; i < dados->linha_final; i++){
        for (int j = 0; j < dados->largura; j++){
            double pixelreal, pixelImaginario;

            parteReal_e_complexa(dados->largura, dados->altura, j, i, &pixelreal, &pixelImaginario);

            int numero_interacoes = interacoes(dados->max_iteracoes, pixelImaginario, pixelreal);

            dados->pixel_da_imagem[(i * dados->largura) + j] = ((255 * (double)numero_interacoes) / dados->max_iteracoes);
        }
    }

    return NULL;
}

double saida_Pthreads1(int numero_largura, int numero_altura, int numero_MAXinteracoes, int *pixel_da_imagem, int numero_threads){
    struct timespec inicio, fim;

    FILE *Erros_pthreads1 = fopen("erros.txt", "a");

    if (Erros_pthreads1 == NULL){
        exit(1);
    }

    pthread_t threads[numero_threads];
    Estrutura_pthread dados_da_thread[numero_threads];

    int linhas_por_thread = numero_altura / numero_threads;
    int resto = numero_altura % numero_threads;

    clock_gettime(CLOCK_MONOTONIC, &inicio);

    clock_gettime(CLOCK_MONOTONIC, &fim);

    double tempo_Pthreads1 = (fim.tv_sec - inicio.tv_sec) + (fim.tv_nsec - inicio.tv_nsec) / 1e9;

    FILE *arquivoPThreads1 = fopen("mandelbrot_lmss4_openmp.pgm", "w");

    if (arquivoPThreads1 == NULL){
        fprintf(stderr, "Erro ao abrir o arquivo Pthreads 1.\n");
        exit(1);
    }

    for (int i = 0; i < numero_altura; i++){
        for (int j = 0; j < numero_largura; j++){
            
            if (j == 0){

                fprintf(arquivoPThreads1, "%d", pixel_da_imagem[(i * numero_largura) + j]);
            } else{

                fprintf(arquivoPThreads1, " %d", pixel_da_imagem[(i * numero_largura) + j]);
            }
        }

        if (i != numero_altura - 1){
            fprintf(arquivoPThreads1, "\n");
        }
    }
    
    fclose(arquivoPThreads1);

    return tempo_Pthreads1;
}

int main(int argc, char *argv[]){
    
    FILE *Erros_main = fopen("erros.txt", "a");

    if (Erros_main == NULL){
        exit(1);
    }

    if (argc < 5 || argc > 5){
        fprintf(Erros_main, "Quantidade de argumentos invalidos, Coloque: Altura Largura MAXinteracoes threads\n");
        exit(1);
    } 

    long numero_altura = ler_entrada(argv[1], "altura");
    if (numero_altura < 1){
        fprintf(Erros_main, "Valor invalido, a Altura precisa ser maior ou pelo menos igual a 1\n");
        exit(1);
    }

    long numero_largura = ler_entrada(argv[2], "largura");
    if (numero_largura < 1){
        fprintf(Erros_main, "Valor invalido, a Largura precisa ser maior ou pelo menos igual a 1\n");
        exit(1);
    }

    long numero_MAXinteracoes = ler_entrada(argv[3], "maximo de interacoes");
    if (numero_MAXinteracoes < 1){
        fprintf(Erros_main, "Valor invalido, O numero maximo de interacoes precisa ser maior ou igual a 1\n");
        exit(1);
    }

    long numero_threads = ler_entrada(argv[4], "threads");
    if (numero_threads < 1){
        fprintf(Erros_main, "Valor invalido, o numero de Threads precisa ser maior ou igual a 1\n");
        exit(1);
    }

    
    int *pixel_da_imagem = (int* )malloc((numero_altura * numero_largura)*sizeof(int)); 
    if (pixel_da_imagem == NULL){
        fprintf(Erros_main, "Erro ao alocar memoria do pixel da imagem\n");
        exit(1);
    }
    
    double tempo_Serial = saida_Serial(numero_largura, numero_altura, numero_MAXinteracoes, pixel_da_imagem);
    double tempo_OpenMP = saida_OpenMP(numero_largura, numero_altura, numero_MAXinteracoes, pixel_da_imagem, numero_threads);
    double tempo_Pthreads1 = saida_Pthreads1(numero_largura, numero_altura, numero_MAXinteracoes, pixel_da_imagem, numero_threads);
    
    
    FILE *ArquivoTempo = fopen("times.txt", "w");

    if (ArquivoTempo == NULL){
        fprintf(Erros_main, "Erro ao abrir o arquivo de tempo\n");
        exit(1);
    }

    fprintf(ArquivoTempo, "Serial: %lfs\n", tempo_Serial);
    fprintf(ArquivoTempo, "OpenMP: %lfs\n", tempo_OpenMP);
    fprintf(ArquivoTempo, "Pthreads1: %lfs\n", tempo_Pthreads1);
    fprintf(ArquivoTempo, "Pthreads2: %lfs", tempo_Serial);

    fclose(ArquivoTempo);
    
    free(pixel_da_imagem);
    
    return 0;
}