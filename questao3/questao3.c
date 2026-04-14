#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_BUCKETS 8  //define o tamanho do hashmap


//ESTRUTRA DO HASHMAP//
typedef struct Node{
    char *chave;   
    struct Node *proximo; //lista encadeada

}Node;

typedef struct Bucket{
    Node *head; //lista encadeada de dados -> o próximo a entrar é sempre a nova cabeça
    pthread_mutex_t mutex; //UM MUTEX PARA CADA BUCKET (para travar a região cŕitica e impedir modificações no mesmo bucket ao mesmo tempo)
} Bucket;

typedef struct HashMap{
    Bucket buckets[NUM_BUCKETS]; //array de buckets
} HashMap;


//OPERAÇÕES DO HASHMAP (inserir, remover, buscar)

void hashmap_init (HashMap *map) { //cria  mapa "map"
    for (int i=0; i< NUM_BUCKETS; i++){
        map->buckets[i].head = NULL;  //inicia lista vazia (cabeça=NULL)
        pthread_mutex_init(&map->buckets[i].mutex, NULL); //inicializa o mutex
    }
}

int hash(const char *chave){ //organização da hash - onde inserir cada chave
    int soma = 0;
    int pos = 0;
    while (chave[pos] != '\0'){
        soma += chave[pos]; //somatorio do valor de cada elemento da chave
        pos++;
    }
    return soma%NUM_BUCKETS; //mod do tamanho do buckeet para definir posição
}

void hashmap_insert(HashMap *map, const char *chave){
    int indice = hash(chave); //recebe a posição da chave
    pthread_mutex_lock(&map->buckets[indice].mutex); //trava qualquer operação dentro do bucket
    Node *no = map->buckets[indice].head;
    while (no != NULL) { //checa se já pertence ao hashmap
        if (strcmp(no->chave, chave) == 0) {
            pthread_mutex_unlock(&map->buckets[indice].mutex);
            printf("Chave já inserida anteriormente\n");
            return;
        }
        no = no->proximo;
    }
    Node *novo = (Node*) malloc(sizeof(Node)); //libera espaço para armazenar novo nó
    if (novo==NULL){ //malloc falhou
        pthread_mutex_unlock(&map->buckets[indice].mutex); //encerra operação no bucket e destrava mutex
        return;
    }
    novo->chave = strdup(chave);
    novo->proximo = map->buckets[indice].head; //a antiga cabeça
    map->buckets[indice].head = novo; //o novo nó é a nova cabeça
    pthread_mutex_unlock(&map->buckets[indice].mutex); //destrava para realizar operações novamente
    printf("Chave inserida\n");
    return;
}


int hashmap_search(HashMap *map, const char *chave){
    int busca = hash(chave); //para saber em qual bucket buscar
    pthread_mutex_lock(&map->buckets[busca].mutex); //bloqueia o acesso ao bucket
    Node *no = map->buckets[busca].head; //começa a busca pelo primeiro elemento (cabeça)
    while (no!=NULL){
        if (strcmp(no->chave, chave) == 0){ //achou a chave
            pthread_mutex_unlock(&map->buckets[busca].mutex); //destrava o bucket
            printf("Chave Encontrada\n");
            return 1;}
        else{
            no = no->proximo;} //vai checando linearmente
    }
    pthread_mutex_unlock(&map->buckets[busca].mutex); //caso não tenha achado o bucket também é desbloqueado
    printf("Chave não pertence ao HashMap\n");
    return 0;
}

void hashmap_remove(HashMap *map, const char *chave){
    int remove = hash(chave); 
    pthread_mutex_lock(&map->buckets[remove].mutex); //trava o bucket
    //buscar a chave para saber se ela existe - se existe nós removemos (apontamos o nó anterior para o próximo a ela!)
    Node *no = map->buckets[remove].head; //seguindo a mesma estrutra da hashmap_search()
    Node *anterior = NULL;
    while (no != NULL) {
        if (strcmp(no->chave, chave)==0){ //achou o no
            if (anterior == NULL){ //indica que a chave que vamor remover é a cabeça
                map->buckets[remove].head = no->proximo; //define a nova cabeça do bucket como o próximo
                pthread_mutex_unlock(&map->buckets[remove].mutex); //desbloqueia o bucket
                free(no); //libera o espaço que o no removido ocupava
                printf("Chave removida\n");
                return;
            }
            anterior->proximo = no->proximo; //organiza a lista encadeada para poder remover o no
            pthread_mutex_unlock(&map->buckets[remove].mutex); //desbloqueia o bucket
            free(no->chave); //libera o espaço 
            free(no); //libera o espaço que o no removido ocupava
            printf("Chave removida\n");
            return;
        }
        //não achou a chave ainda, define o último lido como anterior e vai pro próximo
        anterior = no;
        no = no->proximo;    
    }
    pthread_mutex_unlock(&map->buckets[remove].mutex); 
    printf("Chave não pertence ao HashMap\n");
    return ;
}


//ORGANIZAÇÃO DA THREAD
typedef struct ThreadArgs{  //argumentos que a thread recebe
    HashMap *map;
    char *chave;
    int operacao;
} ThreadArgs;

void *thread_func(void *arg){ //a thread realiza todas as operações de acordo com a entrada recebida
    ThreadArgs *args = (ThreadArgs*) arg;

    if (args->operacao == 0){
        hashmap_insert(args->map, args->chave);
    }
    else if(args->operacao == 1){
        hashmap_search(args->map, args->chave);
    }
    else{
        hashmap_remove(args->map, args->chave);
    }
    return NULL;
}

void hashmap_destroy(HashMap *map){ //liberar o espaço que foi alocado para a hash
    for (int i = 0; i<NUM_BUCKETS; i++){ //acessar cada linha da hash
        Node *no = map->buckets[i].head;
        while (no!=NULL){
            Node *armazena = no;
            no = no->proximo;
            free(armazena->chave);
            free(armazena);
        }
        pthread_mutex_destroy(&map->buckets[i].mutex); }
}

void hashmap_print(HashMap *map){ 
    for (int i = 0; i<NUM_BUCKETS; i++){ //acessar cada linha da hash
        Node *no = map->buckets[i].head;
        printf("%d:", i);
        while (no!=NULL){
            printf("%s ", no->chave);
            no = no->proximo;
        }
        printf("\n");
}}

int main(){
    HashMap map;
    hashmap_init(&map); //inicializa o hashmap

    int qnt_op, operacao;
    char entrada[50];
    scanf("%d", &qnt_op); //recebe a quantidade de operações que serão realizadas na hash

    pthread_t threads[qnt_op]; //threads - uma para cada operação
    ThreadArgs args[qnt_op]; //organização dos argumentos que a threads recebe

    for(int vez=0; vez<qnt_op; vez++){  //loop para receber cada operação
        scanf("%s %d", entrada, &operacao);

        args[vez].map = &map;
        args[vez].chave = strdup(entrada);
        args[vez].operacao = operacao;

        pthread_create(&threads[vez], NULL, thread_func, &args[vez]); //criação das threads e execuçãoem concorrência (com lock dos mutex)
        }
    for(int vez=0; vez<qnt_op; vez++){
        pthread_join(threads[vez], NULL);
        free(args[vez].chave);}
    hashmap_print(&map);
    hashmap_destroy(&map);
    
}