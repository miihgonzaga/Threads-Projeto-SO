/* QUESTÃO 4

    OBJETIVO:
    controlar o sistema de um pedágio duma rodovia
    cada veículo é uma thread
    as cabines são recursos compartilhados (e limitados), podem atender um carro por vez
    garantir que não ocorra conflitos entre os carros
    C cabines, T threads
    cada carro:
        espera uma cabine livre
        paga o pedágio
        libera a cabine
        segue viagem
    cada atendimento leva um pequeno tempo

    controle das cabines: mutex e varia. de condicao (suspender carros durante espera)
    ->NAO pode espera ocupada

    atualizações:
        "Carro X chegou ao pedágio", 
        "Carro X usando cabine 2", 
        "Carro Y aguardando cabine livre"
        "Carro X terminou pagamento e liberou cabine 2"
        "Carro Y usando cabine 2",

*/

// inclusão de bibliotecas:
#include <pthread.h> // biblioteca pra acessar as ferramentas que manipulas threads
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> //bib para controle do tempo na simulacao
#include <unistd.h> // bib para api do posix

#define C 5 //numero de cabines
#define T 12 //numero de carros (threads)

typedef struct {
    int lista_cabines[C]; //0 = livre e 1 = cabine ocuapad
    pthread_mutex_t mutex; //exclusão mútua
    pthread_cond_t var_condicao; //variavel de condicao (dorme quando nao tem cabine livre)
} praca_de_pedagio;

//inicializar praça de pedagio:
praca_de_pedagio pedagio = {
    .lista_cabines = {0}, //todas cabines inicialmente desocupadas
    .mutex = PTHREAD_MUTEX_INITIALIZER, //iniciailiza mutex e a var. de condicao
    .var_condicao = PTHREAD_COND_INITIALIZER
}; 

//funcao de busca pra cabines livres (0 no array)
int buscar_cabine_livre() {
    for (int i = 0; i < C; i++) { //percorre o array de cabines
        if (pedagio.lista_cabines[i] == 0) {
            return i; //retorna o número da primeira cabine livre que encontrar
        }
    }
    return -1; // nenhuma cabine livre
}

/* FUNCAO DE OCUPAR CABINE
    controla regioes críticas usando exclusao mutua e a var. de condicao
    pega uma cabine livre( se tiver)
    usa (tem um tempo de uso)
    libera a cabine
*/
void ocupar_cabine(int carro_id) {
    pthread_mutex_lock(&pedagio.mutex); //trava regiao crítica
    
    int cabine = buscar_cabine_livre(); //busca uma cabine livre (se tiver)
    
    //checa continuamente se tem cab livre
    while (cabine == -1) { // se nenhuma cabine estiver livre
        printf("Carro %d aguardando cabine livre\n", carro_id); //condicao atual do carro
        
        pthread_cond_wait(&pedagio.var_condicao, &pedagio.mutex); 
        //a variavel de condicao libera mutex pra outras threads, dorme a thread atual (o carro fica esperando)
        //quando acordar, já trava o mutex dnv 

        cabine = buscar_cabine_livre(); //verifica disponibilidade de cabine dnv
    }//fim do laço while

    //se saiu do loop, é pq encontrou uma cabine livre!
    //pega a cabine livre
    pedagio.lista_cabines[cabine] = 1;// marca a cabine como ocupada
    printf("Carro %d usando cabine %d\n", carro_id, cabine + 1); //o array começa em cabine 0
    
    pthread_mutex_unlock(&pedagio.mutex); //sai da regiao crítica (libera o mutex)
    
    sleep(1); // uso da cabine (1 seg): tempo de atendimento
    
    // liberar cabine: (voltar na regiao critica e atualizar o array)
    // cabine passa de 1 -> 0
    pthread_mutex_lock(&pedagio.mutex); //trava a regiao
    pedagio.lista_cabines[cabine] = 0; //indica que a cabine tá liberada
    printf("Carro %d terminou pagamento e liberou cabine %d\n", carro_id, cabine + 1); //imprime status
    
    //caso tenha alguma thread dormindo,
    //acorda uma única thread (apenas uma cabine liberada -> apenas uma thread desperta)
    pthread_cond_signal(&pedagio.var_condicao); //
    pthread_mutex_unlock(&pedagio.mutex); //libera a regiao crítica

}

// funcao executada por cada thread (cada carro é uma thread)
void *carro(void* arg) {
    int carro_id = *(int*)arg; // cast void -> int
    free(arg); //libera a memoria 
    
    printf("Carro %d chegou ao pedágio\n", carro_id); //imprime status
    ocupar_cabine(carro_id); //chama a funcao de ocupar cabine
    printf("Carro %d seguiu viagem\n", carro_id);
    
    return NULL;
}

int main() {
    srand(time(NULL));
        
    pthread_t threads[T];
    
    // criar threads dos carros
    for (int i = 0; i < T; i++) {
        int* id = malloc(sizeof(int)); //cria memoria separada pra cada thread
        *id = i + 1;
        
        if (pthread_create(&threads[i], NULL, carro, id) != 0) {
            fprintf(stderr, "erro, nao foi possivel criar thread para o carro %d\n", i + 1);
            exit(1); //encerra
        }
        
        // delay aleatorio e pequeno pra controlar chegada dos carros em momentos diferentes
        usleep(rand() % 100000); //entre 0 e 100ms
    }
    
    // main espera todas threads concluirem
    for (int i = 0; i < T; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // destroi mutex e var de condicao
    pthread_mutex_destroy(&pedagio.mutex);
    pthread_cond_destroy(&pedagio.var_condicao);
    
    printf("\nSimulação concluída!\n");
    
    return 0;
}