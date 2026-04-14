/* QUESTÃO 2
    OBJETIVO:
    tela com L linhas, cada linha contem informacoes
    N arquivos são fornecidos como entrada contendo  as informações de modificações da tela
    T threads para ler os arquivos e atualizar a tela 1 < T <= N
    cada linha deve ser alterada de forma independente (exclusão mútua)
        *travamento de mutex por linha 
        *espera de 2 segundos
        *atualiza
        *destrava o mutex
    (a linha não pode mudar de posicao)

    a tela é o recurso compartilhado a ser modificado (precisa proteger)
    os arquivos são recursos de acesso compartilhado (as threads precisam ler a info)
    *pegar todos os arquivos e armazenar numa lista
    *distribuir os arquivos entre as threads
        cada thread
            faz as atualizações nas linahs correspondentes
        e toda vez que a thread for modificar a tela:
            trava o mutex
            espera 2sgs
            atualiza a informacao
            destrava

    definindo L = 7, N = 3, T = 3
    arquivo1.txt, arquivo2.txt e arquivo3.txt -> arquivos que as threads devem acessar

*/

// inclusão de bibliotecas:
#include <pthread.h> // biblioteca pra acessar as ferramentas que manipulas threads
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // bib para api do posix

//constantes:
#define L 7 // número de linhas da tela 
#define TAM_MAX_STRING 100 // tamanho máximo das strings 

// struct (cada atualicacao tem esse formato)
typedef struct {
    int linha; // começa do 0
    char info[TAM_MAX_STRING]; //informacao de atualizacao
} info_atualizacao; 

//(variaveis GLOBAIS)
//controlar os conteúdos de cada arquivo:
info_atualizacao *lista_atualizacoes = NULL; // lista de atualizações
int qtd_atualizacoes = 0, proxima_atualizacao = 0; 
// variável de controle pra garantir que todas atualizações sejam feita sem repetir
pthread_mutex_t mutex_lista = PTHREAD_MUTEX_INITIALIZER;

//controlar os conteúdos de cada linhas:
pthread_mutex_t mutex_linha[L]; // array de mutex (um pra cada linha da tela
// o array de mutex não está iniciado, precisa iniciar antes de usar!!

//tela inicial
char *tela_inicial[L] = {
    "AQT123 Istambul 17:45",
    "XYZ001 Nice 17:50",
    "ABC789 Moscou 18:20",
    "FFF305 Estocolmo 18:35",
    "QRS111 Madri 18:45",
    "DEF321 Berlim 19:00",
    "GHI456 St. Petsbu. 19:15"
    };

//funcao pra leitura de arquivos e colocar o arquivo na lista
//ler e adicionar na lista, sem modificar o arquivo
void leitura_arquivos(const char *arquivo_nome) {
    FILE *f = fopen(arquivo_nome, "r"); //abrir para leitura
    
    // VERIFICAÇÃO DE ERRO:
    if (!f){  // se der erro pra abrir o arq  
        printf("erro, não está sendo possivel abrir o arquivo %s", arquivo_nome);   // print de erro
        return; 
    } 

    //conseguiu abrir o arquivo:
    int num_linha; 
    char info[TAM_MAX_STRING];

    while (fscanf(f, "%d ", &num_linha) == 1) { // condicao de leitura até o fim do arquivo
        fgets(info, TAM_MAX_STRING, f); //guarda a informacao na variavel info
        
        //retirar \n do final da string:
        int tamanho = strlen(info); //calcula o tamanho
        if (tamanho > 0 && info[tamanho-1] == '\n'){ //se tiver \n no final
            info[tamanho-1] = '\0'; // tira o \n e marca o final da string
        }
        
        // ADICIONANDO DADOS NA LISTA (NUMERO DA LINHA E INFO. DE ATUALIZACAO):
        num_linha--; // a lista começa começa com 0
        
        pthread_mutex_lock(&mutex_lista); // bloqueia (só uma thread atualiza essa parte por vez)
        
        // redimensionar o espaço da mem
        lista_atualizacoes = realloc(lista_atualizacoes, (qtd_atualizacoes + 1) * sizeof(info_atualizacao));
        lista_atualizacoes[qtd_atualizacoes].linha = num_linha; 
        strcpy(lista_atualizacoes[qtd_atualizacoes].info, info); // copia a informacao pra lista
        
        qtd_atualizacoes++; //incrementa a quantidade de atualizações 
        
        pthread_mutex_unlock(&mutex_lista); // libera

    } //fim do laço while

    fclose(f); //fecha o arq

}// fim da função de leitura de arquivos

// função de thread (cada thread executa)
void *funcao_thread(void *arg){
    int id = *(int*)arg; // cast de void pra inteiro (id da thread)

    // variavel de condição do laço
    int atualizando = 1; // enquanto tiver atualizações a serem feitas

    while(atualizando){
        // modificando a região crítica das listas de atualizações
        pthread_mutex_lock(&mutex_lista); // protege as variáveis globais de controle da lista de arquivo
        // protege a variável proxima_atualizacao e o acesso ao indice

        // verificar se já chegou ao final da lista de atualizações
        if (proxima_atualizacao >= qtd_atualizacoes){ 
            atualizando = 0; // não há mais atualizações a serem feitas
            pthread_mutex_unlock(&mutex_lista); // destravar o mutex
        }
        
        else{
            //o indice da lista indica qual arquivo a thread vai usar
            int indice = proxima_atualizacao++; // o índice é a posição da lista que a thread vai pegar pra atualizar
            // libera o recurso -> outra thread pode acessar a lista de atualizações e pegar um índice
            pthread_mutex_unlock(&mutex_lista); 

            info_atualizacao *informacao = &lista_atualizacoes[indice];

            //atualização da linha (regiao critica)
            pthread_mutex_lock(&mutex_linha[informacao->linha]); //bloqueia a linha
            
            printf("\033[%d;1H", informacao->linha + 1); //posiciona o cursor
            printf("\033[K"); // limpa a linha
            printf("\033[1;32m%s\033[0m", informacao->info); //imprime verde
            fflush(stdout); //forçar a saida no terminal 
            
            sleep(2); // espera 2 segs
            pthread_mutex_unlock(&mutex_linha[informacao->linha]); // libera a linha
        
        } // fim do else

    } //fim do laço while

    pthread_exit(NULL); //encerra a thread secundária

} // fim da funcao de execucao das threads

int main(){ // thread principal

    //inicializar mutexes do array de mutex pras linhas:
    for (int i = 0; i < L; i++) {
        pthread_mutex_init(&mutex_linha[i], NULL);
    }

    printf("\033[2J"); // limpar a  tela
    printf("\033[H"); // colocar o cursor no início

    for (int i = 0; i < L; i++) { //imprimir a tela  inicila
        printf("\033[31m%s\n\033[0m", tela_inicial[i]);
    }
    fflush(stdout); ///forçar a saida imediata no terminal 

    //lê os três arquivos usando a funcao de lieutra
    leitura_arquivos("arquivo1.txt");
    leitura_arquivos("arquivo2.txt");
    leitura_arquivos("arquivo3.txt");

    //criar threads secundárias: (tres threasd)
    int T = 3; 
    pthread_t threads[T];
    int ids_threads[T]; 

    for (int i = 0; i<T; i++){
        ids_threads[i] = i; // determina o id da thread e cria a a thread
        pthread_create(&threads[i], NULL, funcao_thread, &ids_threads[i]);
    }

    //fazer a main esperar as threads terminarem
    for (int i = 0; i < T; i++) {
        pthread_join(threads[i], NULL); // faz a main esperar a execução das threads
    }

    //liberar ponteiros e destroir mutexes:
    free(lista_atualizacoes);
    for (int i = 0; i < L; i++) {
        pthread_mutex_destroy(&mutex_linha[i]);
    }
    printf("\033[%d;1H", L+1); //posiciona o cursor depois da tabela
    pthread_exit(NULL); // encerra a main
    
}