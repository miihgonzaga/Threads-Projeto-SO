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
    //implementação recursiva do mergesort
    ThreadData tdata = *((ThreadData *)threaddata); //pego os valores necessários pra execução pra thread
    if (tdata.comeco < tdata.fim) {
        int meio = (tdata.fim+tdata.comeco)/2;
        //crio as estruturas que serão passadas para a esquerda e a direita e atribuo os valores
        ThreadData *tdata_left = malloc(sizeof(ThreadData)); 
        ThreadData *tdata_right = malloc(sizeof(ThreadData));
        tdata_left->vetor = tdata.vetor;
        tdata_left->comeco = tdata.comeco;
        tdata_left->fim = meio;
        tdata_right->vetor = tdata.vetor;
        tdata_right->comeco = meio+1;
        tdata_right->fim = tdata.fim;

        pthread_t threads[2];
        //crio 2 threads em que uma enxerga a direita do vetor, e a outra a esquerda
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
    int tam = 0;
    int entrada;
    int *valores;
    printf("insira o array que deseja ordernar(em uma linha só):");
    while(scanf("%d", &entrada)!= EOF) 
    {
        tam++;
        int *temp = realloc(valores, tam * sizeof(int));
        if (temp != NULL)
        {
            valores = temp;
        }
        valores[tam-1] = entrada;
    }
    //caso por algum motivo o vetor seja nulo (entrada inadequada)
    if (valores != NULL)
    {
        //crio a thread "pai" que "enxerga" o vetor inteiro
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
    }
    else
    {
        printf("\nvetor nulo, erro de execução\n");
    }
    pthread_exit(NULL);
    return 0;
}