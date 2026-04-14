#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//para testar o código com outra imagem, coloque o ppm na pasta da questão e o nome do arquivo na linha 23

typedef struct {
    int R, G, B;
    int x, y;
    int **Imagem;
} ThreadData;

void *MakePixel (void *threaddata)
{
    //thread pega as informações do pixel e faz a conversão 
    ThreadData tdata = *((ThreadData *)threaddata); 
    tdata.Imagem[tdata.y][tdata.x] =  tdata.R*0.30 + tdata.G*0.59 + tdata.B*0.11;
    return NULL;
}


int main(int argc, char *argv[]) {
    FILE *arqin = fopen("questao1_SO.ppm", "r");
    if (arqin == NULL) {
        printf("Erro ao abrir o arquivo de entrada!\n");
        return 1;
    }
    int x;
    int y;
    int cor;
    char letra_magica[3];

    FILE *fp = fopen("preto_e_branco.ppm", "w"); // Cria o arquivo
    
    //scan dos atributos iniciais da imagem

    fscanf(arqin,"%s", letra_magica);
    fscanf(arqin,"%d %d", &x, &y);
    fscanf(arqin,"%d", &cor);

    //uma thread por pixel, então o número de threads é x*y
    #define NUMBER_THREAD x*y
    pthread_t threads[NUMBER_THREAD];
    ThreadData *task_datas[NUMBER_THREAD];
    
    // Cabeçalho: P3, Largura, Altura, Max Color
    fprintf(fp, "%s\n%d %d\n%d\n", letra_magica, x, y, cor);
   
    //arrayy onde vai ficar os a informação de cada pixel
    int **imagem = (int **)malloc(y * sizeof(int *));
    for (int i = 0; i < y; i++)
    {
        imagem[i] = (int *)malloc(x * sizeof(int));
    }
    //crio as variáveis de cor para então ir para as threads
    int R, G, B;
    for (int i = 0; i < NUMBER_THREAD; i++)
    {        
        //passo todas as informações necessárias para a thread para um ponteiro com a estrutura usada
        task_datas[i] = (ThreadData *) malloc(sizeof(ThreadData)); 
        task_datas[i]->y = i/x;
        task_datas[i]->x = i%x;
        fscanf(arqin,"%d %d %d", &R, &G, &B);
        task_datas[i]->R = R;
        task_datas[i]->G = G;
        task_datas[i]->B = B;
        task_datas[i]->Imagem = imagem;
        int rc = pthread_create(&threads[i], NULL, MakePixel, (void *) task_datas[i]);      
        if (rc)
        {         
            printf("ERRO; codigo de retorno %d\n", rc);         
            exit(-1);      
        }   
  
    }
    for (int i = 0; i < NUMBER_THREAD; i++)    
    {
        //join para esperar todas as conversões acabarem
        pthread_join(threads[i], NULL);
    }

    for (int i = 0; i < y; i++) 
    {
        for (int j = 0; j < x; j++) 
        {
            int cinza = imagem[i][j];    
            fprintf(fp, "%d %d %d ", cinza, cinza, cinza);
        }
    fprintf(fp, "\n");
}        
    fclose(fp);
    fclose(arqin);
    printf("Imagem PPM criada com sucesso!\n");
    free(*task_datas);
    for (int i = 0; i < y; i++)
    {
        free(imagem[i]);
    }
    free(imagem);
    pthread_exit(NULL);
    return 0;
}