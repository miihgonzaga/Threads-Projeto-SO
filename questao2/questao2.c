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
#define COMPRIMENTO_TABELA 30 // tamanho da tabela (alinhar)

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

// FUNÇÃO PRA IMPRIMIR A TELA INICIAL
void imprimir_tela_inicial() {
    printf("\033[2J");  // limpa tela
    printf("\033[H");   // ajjusta cursor
    
    for (int i = 0; i < L; i++) {
        printf("\033[%d;1H", i + 1);  // posiciona linha
        printf("\033[K");  // limpa linha completamente antes de escrever

        char linha_informacao[COMPRIMENTO_TABELA+2]; //por segurança, +2
        strcpy(linha_informacao, tela_inicial[i]); //faz uma copia da string da tela_inicial[i] pra linha_informacao

        //deixar todas as linhas com o mesmo tamanho (alinhar tabela)
        // nenhum dos arquivos contém infos com qtd de caracteres >= 30
        int tamanho_info = strlen(linha_informacao); 

        if (tamanho_info < COMPRIMENTO_TABELA){ // se for menor do que 30:
            
            // adicionar espaços pra que todas as linhas tenham memsa quantidade de caracteres:
            for (int j = tamanho_info; j < COMPRIMENTO_TABELA; j++) {
                linha_informacao[j] = ' '; //adiciona espaço
            }
            linha_informacao[COMPRIMENTO_TABELA] = '\0'; //finaliza string

        }

        // aplicar a cor
        switch(i) {
            case 0: printf("\033[30;41m%s\033[0m", linha_informacao); break; // fundo vermelho
            case 1: printf("\033[30;42m%s\033[0m", linha_informacao); break; // fundo verde
            case 2: printf("\033[30;43m%s\033[0m", linha_informacao); break; // fundo amarelo
            case 3: printf("\033[30;44m%s\033[0m", linha_informacao); break; // fundo azul
            case 4: printf("\033[30;45m%s\033[0m", linha_informacao); break; // fundo magenta
            case 5: printf("\033[30;46m%s\033[0m", linha_informacao); break; // fundo ciano
            case 6: printf("\033[30;47m%s\033[0m", linha_informacao); break; // fundo branco
            }

    }// fim do laço for

    fflush(stdout); //forçar a saida imediata no terminal 

} // fim da função que imprime a tela inicial

//funcao pra leitura de arquivos e colocar o arquivo na lista
//ler e adicionar na lista, sem modificar o arquivo
void *leitura_arquivos(void *arg) {
    char *arquivo_nome = (char*)arg; //cast de void pra char
    FILE *f = fopen(arquivo_nome, "r"); //abrir para leitura
    
    // VERIFICAÇÃO DE ERRO:
    if (!f){  // se der erro pra abrir o arq  
        printf("erro, não está sendo possivel abrir o arquivo %s", arquivo_nome);   // print de erro
        pthread_exit(NULL); 
    } 

    //conseguiu abrir o arquivo:
    int num_linha; 
    char info[TAM_MAX_STRING];
    char linha_entrada[TAM_MAX_STRING]; //receber entradas

    while (fgets(linha_entrada, sizeof(linha_entrada), f) != NULL) { // condicao de leitura até o fim do arquivo
        //retirar \n do final da string de entrada:
        linha_entrada[strcspn(linha_entrada, "\n")] = '\0'; 
        
        if (strlen(linha_entrada) == 0) continue; //pular linha vazia

        if (sscanf(linha_entrada, "%d", &num_linha) == 1) { // verifica se a 1a entrada realmente é um número 
            
            // se for numero, a proxima linha de entrada deve ser a informação:
            if (fgets(info, TAM_MAX_STRING, f) != NULL) {
                info[strcspn(info, "\n")] = '\0'; //retirar \n
                
            // ADICIONANDO DADOS NA LISTA (NUMERO DA LINHA E INFO. DE ATUALIZACAO):
            num_linha--; // a lista começa começa com 0
            
            pthread_mutex_lock(&mutex_lista); // bloqueia (só uma thread atualiza essa parte por vez)
            
            // redimensionar o espaço da mem
            lista_atualizacoes = realloc(lista_atualizacoes, (qtd_atualizacoes + 1) * sizeof(info_atualizacao));
            lista_atualizacoes[qtd_atualizacoes].linha = num_linha; 
            strcpy(lista_atualizacoes[qtd_atualizacoes].info, info); // copia a informacao pra lista
            
            qtd_atualizacoes++; //incrementa a quantidade de atualizações 
            
            pthread_mutex_unlock(&mutex_lista); // libera

            } 
        } 
    
    }//fim do laço while

    fclose(f); //fecha o arq
    pthread_exit(NULL); 

}// fim da função de leitura de arquivos

// função de thread p/ atualização (cada thread executa)
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
            
            //criar uma cópia da informação pra modificar -> alinhar na tabela
            char info_formatada[COMPRIMENTO_TABELA + 2];
            strcpy(info_formatada, informacao->info);

            int tamanho_info = strlen(info_formatada);
            if (tamanho_info < COMPRIMENTO_TABELA) {
                // adicionar espaços para que todas as linhas tenham mesma quantidade de caracteres
                for (int j = tamanho_info; j < COMPRIMENTO_TABELA; j++) {
                    info_formatada[j] = ' '; // add espaço
                }
                info_formatada[COMPRIMENTO_TABELA] = '\0'; //finaliza str
            }

            //atualização da linha (regiao critica)
            pthread_mutex_lock(&mutex_linha[informacao->linha]); //bloqueia a linha
            
            printf("\033[%d;1H", informacao->linha + 1); //posiciona o cursor
            printf("\033[K"); // limpa a linha
            
            // switch pra determinar a cor da linha:
            // aplicar a cor
            switch(informacao->linha) {
                case 0: printf("\033[30;41m%s\033[0m", info_formatada); break; // fundo vermelho
                case 1: printf("\033[30;42m%s\033[0m", info_formatada); break; // fundo verde
                case 2: printf("\033[30;43m%s\033[0m", info_formatada); break; // fundo amarelo
                case 3: printf("\033[30;44m%s\033[0m", info_formatada); break; // fundo azul
                case 4: printf("\033[30;45m%s\033[0m", info_formatada); break; // fundo magenta
                case 5: printf("\033[30;46m%s\033[0m", info_formatada); break; // fundo ciano
                case 6: printf("\033[30;47m%s\033[0m", info_formatada); break; // fundo branco
                }


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

    system("clear"); //limpeza (evitar concatenacao entre as informacoes atualizadas)
    imprimir_tela_inicial(); //chamada de função pra impressão da tela inicial

    //lê os três arquivos usando a funcao de lieutra
    pthread_t thread_leitura[3];
    char arquivos[3][100] = {
        "arquivo1.txt", 
        "arquivo2.txt", 
        "arquivo3.txt"
    };
    
    // threads vao realizar a leitura dos arquivos:
    for (int i = 0; i< 3; i++){
        pthread_create(&thread_leitura[i], NULL, leitura_arquivos, arquivos[i]);
    }    

    //fazer a main esperar as threads terminarem a leitura
    for (int i = 0; i < 3; i++) {
        pthread_join(thread_leitura[i], NULL); // faz a main esperar a execução das threads
    }

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

    //liberar ponteiros e destruir mutexes:
    free(lista_atualizacoes);
    for (int i = 0; i < L; i++) {
        pthread_mutex_destroy(&mutex_linha[i]);
    }
    printf("\033[%d;1H", L+1); //posiciona o cursor depois da tabela
    pthread_exit(NULL); // encerra a main
    
}