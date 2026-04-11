#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct ThreadData
{
    int *vetor;
    int comeco;
    int fim;
} ThreadData;


void merge(int vetor[], int comeco, int meio, int fim) {
    int com1 = comeco, com2 = meio+1, comAux = 0, tam = fim-comeco+1;
    int *vetAux;
    vetAux = (int*)malloc(tam * sizeof(int));
    printf("entrei no merge, comeco: %d fim: %d\n\n", comeco, fim);
    while(com1 <= meio && com2 <= fim){
        if(vetor[com1] < vetor[com2]) {
            vetAux[comAux] = vetor[com1];
            com1++;
        } else {
            vetAux[comAux] = vetor[com2];
            com2++;
        }
        comAux++;
    }

    while(com1 <= meio){  //Caso ainda haja elementos na primeira metade
        vetAux[comAux] = vetor[com1];
        comAux++;
        com1++;
    }

    while(com2 <= fim) {   //Caso ainda haja elementos na segunda metade
        vetAux[comAux] = vetor[com2];
        comAux++;
        com2++;
    }

    for(comAux = comeco; comAux <= fim; comAux++){    //Move os elementos de volta para o vetor original
        vetor[comAux] = vetAux[comAux-comeco];
    }
    
    free(vetAux);
}

void *ThreadMergeSort(void *threaddata){
    ThreadData tdata = *((ThreadData *)threaddata); //pego os valores necessários pra execução pra thread
    if (tdata.comeco < tdata.fim) {
        int meio = (tdata.fim+tdata.comeco)/2;
        //crio as estruturas que serão passadas para a esquerda e a direita
        ThreadData *tdata_left = malloc(sizeof(ThreadData)); 
        ThreadData *tdata_right = malloc(sizeof(ThreadData));
        tdata_left->vetor = tdata.vetor;
        tdata_left->comeco = tdata.comeco;
        tdata_left->fim = meio;
        tdata_right->vetor = tdata.vetor;
        tdata_right->comeco = meio+1;
        tdata_right->fim = tdata.fim;

        pthread_t threads[2];
        
        pthread_create(&threads[0], NULL, ThreadMergeSort, (void *) tdata_left);
        pthread_create(&threads[1], NULL, ThreadMergeSort, (void *) tdata_right);
        //join para a thread pai esperar os filhos terminarem
        pthread_join(threads[0], NULL);
        pthread_join(threads[1], NULL);
        //libero os espaços que reservei já que não serão mais usados
        free(tdata_left);
        free(tdata_right);
        merge(tdata.vetor, tdata.comeco, meio, tdata.fim);
    }
}

int main()
{   
    int tam;
    printf("insira o tamanho do vetor que deseja ordenar:");
    scanf("%d", &tam);
    int valores[tam];
    printf("insira o array que deseja ordernar (separando os números por espaços e sem repetições):");
    for (int i = 0; i < tam; i++)
    {
        scanf("%d", &valores[i]);
    }
    ThreadData data = {valores, 0, tam-1};
    pthread_t thread[0];
    pthread_create(&thread[0], NULL, ThreadMergeSort, (void *) &data);
    pthread_join(thread[0], NULL);
    printf("lista ordenada: ");
    for (int i = 0; i < tam; i++)
    {
        printf("%d ", data.vetor[i]);
    }
    printf("\n");
    pthread_exit(NULL);
    return 0;
}