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
    int *iteracoes_da_imagem; 
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
    FILE *Erros_ler_entrada = fopen("erros.txt", "w");

    if (Erros_ler_entrada == NULL){
        exit(1);
    }

    char *final_argumento;
    errno = 0;

    long numero = strtol(argumento, &final_argumento, 10);

    if (errno != 0 || *final_argumento != '\0'){
        fprintf(Erros_ler_entrada, "%s invalido(a): %s\n", nome, argumento);
        fclose(Erros_ler_entrada);
        exit(1);
    }

    fclose(Erros_ler_entrada);
    return numero;
}

double saida_Serial(int numero_largura, int numero_altura, int numero_MAXinteracoes, int *pixel_da_imagem){
    
    struct timespec inicio, fim;

    FILE *Erros_serial = fopen("erros.txt", "w");

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
        fclose(Erros_serial); 
        free(pixel_da_imagem);
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
    fclose(Erros_serial);

    return tempo_Serial;
}

double saida_OpenMP(int numero_largura, int numero_altura, int numero_MAXinteracoes, int *pixel_da_imagem, int numero_threads){
    
    struct timespec inicio, fim;
    
    FILE *Erros_OpenMP = fopen("erros.txt", "w");
    
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
        fclose(Erros_OpenMP); 
        free(pixel_da_imagem);
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
    fclose(Erros_OpenMP);  

    return tempo_OpenMP;
}

void *calcula_pixel_PThreads(void *informacoes_pthreads){
    Estrutura_pthread *dados = (Estrutura_pthread *)informacoes_pthreads;

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

    FILE *Erros_pthreads1 = fopen("erros.txt", "w");

    if (Erros_pthreads1 == NULL){
        exit(1);
    }

    pthread_t *threads = malloc(numero_threads * sizeof(pthread_t));
    Estrutura_pthread *dados_da_thread = malloc(numero_threads * sizeof(Estrutura_pthread));

    if (threads == NULL){
        fprintf(Erros_pthreads1, "Erro ao alocar as threads\n");
        free(threads);
        free(dados_da_thread);
        free(pixel_da_imagem);
        fclose(Erros_pthreads1);
        exit(1);
    }

    if (dados_da_thread == NULL){
        fprintf(Erros_pthreads1, "Erro ao alocar dados da threads\n");
        free(threads);
        free(dados_da_thread);
        free(pixel_da_imagem);
        fclose(Erros_pthreads1);
        exit(1);
    }

    int linhas_por_thread = numero_altura / numero_threads;
    int resto = numero_altura % numero_threads;

    clock_gettime(CLOCK_MONOTONIC, &inicio);
    int linha_atual = 0;
    
    for (int i = 0; i < numero_threads; i++){

        dados_da_thread[i].largura = numero_largura;

        dados_da_thread[i].altura = numero_altura;

        dados_da_thread[i].max_iteracoes = numero_MAXinteracoes;

        dados_da_thread[i].pixel_da_imagem = pixel_da_imagem;

        dados_da_thread[i].linha_inicial = linha_atual;

        int linhas_da_thread = linhas_por_thread;
        if (i == numero_threads - 1){
            linhas_da_thread += resto;
        }

        dados_da_thread[i].linha_final = linha_atual + linhas_da_thread;
        linha_atual = dados_da_thread[i].linha_final;

        int criar_thread = pthread_create(&threads[i], NULL, calcula_pixel_PThreads, &dados_da_thread[i]);

        if (criar_thread != 0){
            fprintf(Erros_pthreads1, "Erro ao criar thread %d\n", i);
            for (int k = 0; k < i; k++){
                pthread_join(threads[k], NULL);
            }
            free(threads);
            free(dados_da_thread);
            free(pixel_da_imagem);
            fclose(Erros_pthreads1);
            exit(1);
        }
    }

    for (int i = 0; i < numero_threads; i++){
        pthread_join(threads[i], NULL);
    }
    clock_gettime(CLOCK_MONOTONIC, &fim);

    
    double tempo_Pthreads1 = (fim.tv_sec - inicio.tv_sec) + (fim.tv_nsec - inicio.tv_nsec) / 1e9;

    FILE *arquivoPThreads1 = fopen("mandelbrot_lmss4_pthreads1.pgm", "w");

    if (arquivoPThreads1 == NULL){
        fprintf(Erros_pthreads1, "Erro ao abrir o arquivo Pthreads 1.\n");
        free(threads);
        free(dados_da_thread);
        free(pixel_da_imagem);
        fclose(Erros_pthreads1);
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
    fclose(Erros_pthreads1);
    free(threads);
    free(dados_da_thread);

    return tempo_Pthreads1;
}

void preenche_iteracoes(int numero_largura, int numero_altura, int numero_MAXinteracoes, int *iteracoes_da_imagem){
    for (int i = 0; i < numero_altura; i++){
        for (int j = 0; j < numero_largura; j++){
            double pixelreal, pixelImaginario;

            parteReal_e_complexa(numero_largura, numero_altura, j, i, &pixelreal, &pixelImaginario);

            int numero_interacoes = interacoes(numero_MAXinteracoes, pixelImaginario, pixelreal);

            iteracoes_da_imagem[(i * numero_largura) + j] = numero_interacoes;
        }
    }
}

void *normaliza_bloco_pthreads2(void *informacoes_pthreads){
    Estrutura_pthread *dados = (Estrutura_pthread *)informacoes_pthreads;

    for (int i = dados->linha_inicial; i < dados->linha_final; i++){
        for (int j = 0; j < dados->largura; j++){
            
            int indice = (i * dados->largura) + j;
            int numero_interacoes = dados->iteracoes_da_imagem[indice];

            dados->pixel_da_imagem[indice] = ((255 * (double)numero_interacoes) / dados->max_iteracoes);
        }
    }

    return NULL;
}

double saida_pthreads2(int numero_largura, int numero_altura, int numero_MAXinteracoes, int *pixel_da_imagem, int *iteracoes_da_imagem, int numero_threads){

    struct timespec inicio, fim;

    FILE *Erros_pthreads2 = fopen("erros.txt", "w");

    if (Erros_pthreads2 == NULL){
        exit(1);
    }

    pthread_t *threads = malloc(numero_threads * sizeof(pthread_t));
    Estrutura_pthread *dados_da_thread = malloc(numero_threads * sizeof(Estrutura_pthread));

    if (threads == NULL || dados_da_thread == NULL){
        fprintf(Erros_pthreads2, "Erro ao alocar as threads (Pthreads2)\n");
        free(threads);
        free(dados_da_thread);
        free(pixel_da_imagem);
        free(iteracoes_da_imagem);
        fclose(Erros_pthreads2);
        exit(1);
    }

    preenche_iteracoes(numero_largura, numero_altura, numero_MAXinteracoes, iteracoes_da_imagem);

    int linhas_por_thread = numero_altura / numero_threads;
    int resto = numero_altura % numero_threads;
    int linha_atual = 0;

    clock_gettime(CLOCK_MONOTONIC, &inicio);

    for (int i = 0; i < numero_threads; i++){

        int resultado;
        if (i < resto) {
            resultado = 1;
        } else {
            resultado = 0;
        }

        int linhas_da_thread = linhas_por_thread + resultado;

        dados_da_thread[i].largura = numero_largura;

        dados_da_thread[i].altura = numero_altura;

        dados_da_thread[i].max_iteracoes = numero_MAXinteracoes;

        dados_da_thread[i].pixel_da_imagem = pixel_da_imagem;

        dados_da_thread[i].iteracoes_da_imagem = iteracoes_da_imagem;

        dados_da_thread[i].linha_inicial = linha_atual;

        dados_da_thread[i].linha_final = linha_atual + linhas_da_thread;
        linha_atual = dados_da_thread[i].linha_final;

        int criar_thread = pthread_create(&threads[i], NULL, normaliza_bloco_pthreads2, &dados_da_thread[i]);

        if (criar_thread != 0){
            fprintf(Erros_pthreads2, "Erro ao criar thread %d (Pthreads2)\n", i);
            for (int k = 0; k < i; k++){
                pthread_join(threads[k], NULL);
            }
            free(threads);
            free(dados_da_thread);
            free(pixel_da_imagem);
            free(iteracoes_da_imagem);
            fclose(Erros_pthreads2);
            exit(1);
        }
    }

    for (int i = 0; i < numero_threads; i++){
        pthread_join(threads[i], NULL);
    }
    clock_gettime(CLOCK_MONOTONIC, &fim);

    double tempo_Pthreads2 = (fim.tv_sec - inicio.tv_sec) + (fim.tv_nsec - inicio.tv_nsec) / 1e9;

    FILE *arquivoPThreads2 = fopen("mandelbrot_lmss4_pthreads2.pgm", "w");

    if (arquivoPThreads2 == NULL){
        fprintf(Erros_pthreads2, "Erro ao abrir o arquivo Pthreads 2.\n");
        free(threads);
        free(dados_da_thread);
        free(pixel_da_imagem);
        free(iteracoes_da_imagem);
        fclose(Erros_pthreads2);
        exit(1);
    }

    for (int i = 0; i < numero_altura; i++){
        for (int j = 0; j < numero_largura; j++){

            if (j == 0){
                fprintf(arquivoPThreads2, "%d", pixel_da_imagem[(i * numero_largura) + j]);
            } else{
                fprintf(arquivoPThreads2, " %d", pixel_da_imagem[(i * numero_largura) + j]);
            }
        }

        if (i != numero_altura - 1){
            fprintf(arquivoPThreads2, "\n");
        }
    }

    fclose(arquivoPThreads2);
    free(threads);
    free(dados_da_thread);
    fclose(Erros_pthreads2);

    return tempo_Pthreads2;
}

int main(int argc, char *argv[]){
    
    FILE *Erros_main = fopen("erros.txt", "w");

    if (Erros_main == NULL){
        exit(1);
    }

    if (argc < 5 || argc > 5){
        fprintf(Erros_main, "Quantidade de argumentos invalidos, Coloque: Altura Largura MAXinteracoes threads\n");
        fclose(Erros_main); 
        exit(1);
    } 
    
    long int numero_largura = ler_entrada(argv[1], "largura");
    if (numero_largura < 1){
        fprintf(Erros_main, "Valor invalido, a Largura precisa ser maior ou pelo menos igual a 1\n");
        fclose(Erros_main);  
        exit(1);
    }

    long int numero_altura = ler_entrada(argv[2], "altura");
    if (numero_altura < 1){
        fprintf(Erros_main, "Valor invalido, a Altura precisa ser maior ou pelo menos igual a 1\n");
        fclose(Erros_main);  
        exit(1);
    }


    long int numero_MAXinteracoes = ler_entrada(argv[3], "maximo de interacoes");
    if (numero_MAXinteracoes < 1){
        fprintf(Erros_main, "Valor invalido, O numero maximo de interacoes precisa ser maior ou igual a 1\n");
        fclose(Erros_main);  
        exit(1);
    }

    long int numero_threads = ler_entrada(argv[4], "threads");
    if (numero_threads < 1){
        fprintf(Erros_main, "Valor invalido, o numero de Threads precisa ser maior ou igual a 1\n");
        fclose(Erros_main);  
        exit(1);
    }

    if (numero_threads > numero_altura) {
        numero_threads = numero_altura;
    }
 
    int *pixel_da_imagem = (int* )malloc((numero_altura * numero_largura)*sizeof(int)); 
    if (pixel_da_imagem == NULL){
        fprintf(Erros_main, "Erro na alocacao de memoria do pixel da imagem\n");
        fclose(Erros_main);  
        exit(1);
    }

    int *iteracoes_da_imagem = (int *)malloc((numero_altura * numero_largura) * sizeof(int));
    if (iteracoes_da_imagem == NULL){
        fprintf(Erros_main, "Erro na alocacao de memoria das iteracoes da imagem\n");
        fclose(Erros_main);  
        free(pixel_da_imagem);
        exit(1);
    }

    double tempo_Serial = saida_Serial(numero_largura, numero_altura, numero_MAXinteracoes, pixel_da_imagem);
    double tempo_OpenMP = saida_OpenMP(numero_largura, numero_altura, numero_MAXinteracoes, pixel_da_imagem, numero_threads);
    double tempo_Pthreads1 = saida_Pthreads1(numero_largura, numero_altura, numero_MAXinteracoes, pixel_da_imagem, numero_threads);
    double tempo_Pthreads2 = saida_pthreads2(numero_largura, numero_altura, numero_MAXinteracoes, pixel_da_imagem, iteracoes_da_imagem, numero_threads);

    FILE *ArquivoTempo = fopen("times.txt", "w");

    if (ArquivoTempo == NULL){
        fprintf(Erros_main, "Erro ao abrir o arquivo de tempo\n");
        free(pixel_da_imagem);
        free(iteracoes_da_imagem);
        exit(1);
    }

    fprintf(ArquivoTempo, "Serial: %lfs\n", tempo_Serial);
    fprintf(ArquivoTempo, "OpenMP: %lfs\n", tempo_OpenMP);
    fprintf(ArquivoTempo, "Pthreads1: %lfs\n", tempo_Pthreads1);
    fprintf(ArquivoTempo, "Pthreads2: %lfs", tempo_Pthreads2);

    fclose(ArquivoTempo);

    fclose(Erros_main);  
    free(pixel_da_imagem);
    free(iteracoes_da_imagem);

    return 0;
}