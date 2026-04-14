#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_THREADS 4 //quantidade N=4 de threads

typedef struct ThreadArgs{
    int (*matriz)[50];
    int tamanho; //para checar a mesma quantidade de linhas e colunas
    int verifica;
    int valor;
}ThreadArgs;

void *thread_func(void *arg){ //funçao que executa a thread - checa cada parte da matriz de acordo com o args->verifica
    ThreadArgs *args = (ThreadArgs*) arg;
    if (args->verifica == 0){
        //deve checar todas as linhas
        int sum_linha=0, comparador=0, contagem=0;
        for(int i=0; i<args->tamanho; i++){
            for(int j=0; j<args->tamanho; j++){
                sum_linha += args->matriz[i][j];} //percorre toda a linha
            if (i==0) comparador = sum_linha;
            if (comparador == sum_linha) contagem++;
            sum_linha = 0;
            }
        if (contagem == args->tamanho)args->valor = comparador;
        else args->valor = -1;  //adimitindo apenas matriz de inteiros positivos
    }
    else if(args->verifica == 1){
        //deve checar todas as colunas
        int sum_coluna=0, comparador=0, contagem=0;
        for (int j=0; j<args->tamanho;j++){
            for (int i=0; i<args->tamanho; i++){
                sum_coluna += args->matriz[i][j]; } //percorre toda a coluna
            if (j==0) comparador = sum_coluna;
            if (comparador == sum_coluna) contagem++;
            sum_coluna = 0;
        }
        if (contagem == args->tamanho) args->valor = comparador;
        else args->valor = -1;  //adimitindo apenas matriz de inteiros positivos 
    }
    else if(args->verifica == 2){
        //checa a diagonal principal
        int sum_diagonal=0;
        for (int i=0; i<args->tamanho; i++){
            sum_diagonal += args->matriz[i][i];}
        args->valor = sum_diagonal;
    }
    else {
        //checa diagonal secundária
        int sum_diagonal=0;
        for (int i=0; i<args->tamanho; i++){
            sum_diagonal += args->matriz[i][args->tamanho-i-1];}
        args->valor = sum_diagonal;
    }
    return NULL;
}


int main(){
    ThreadArgs arg[4]; //sempre serão 4 threads independente do tamanho da matriz
    pthread_t thread[4];

    int tamanho, comparador=0, contagem=0;
    scanf("%d", &tamanho);
    int matriz[50][50];

    for (int i=0; i<tamanho; i++){
        for (int j=0; j<tamanho;j++){
            scanf("%d", &matriz[i][j]);}
    }

    for (int i=0; i<4; i++){
        arg[i].matriz = matriz;
        arg[i].tamanho = tamanho;
        arg[i].valor = 0;
        arg[i].verifica = i;
        
        pthread_create(&thread[i], NULL, thread_func, &arg[i]); //criando threads para cada verificação
    }

    for (int i=0; i<4; i++){ 
        pthread_join(thread[i],NULL);}  //esperar as threads encerrarem operação para dar continuidade ao código

    for (int i=0; i<4; i++){ //comparação para saber se cada verificação retornou o mesmo valor
        if(arg[i].valor != -1){
            if(i==0) comparador = arg[i].valor;
            if(comparador==arg[i].valor) contagem++;}
        else {
            printf("Não é um quadrado mágico\n");
            break;}
    }
    if (contagem == 4) printf("É um quadrado mágico\n");

}
