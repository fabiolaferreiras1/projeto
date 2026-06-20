#ifndef HASH_H
#define HASH_H

// Definição do tamanho inicial da tabela (deve ser ajustado/justificado nos experimentos)
#define TAMANHO_HASH 10009 // Usar um número primo ajuda a reduzir colisões

// Estrutura do nó para encadeamento externo (Lista Encadeada)
typedef struct No {
    char usuario[12];     // Armazena o nome do usuário (formato de 11 caracteres + '\0')
    struct No* proximo;   // Ponteiro para o próximo nó em caso de colisão
} No;

// Estrutura da Tabela Hash
typedef struct {
    No** tabela;          // Vetor de ponteiros para Nós (as "gavetas" da tabela)
    int tamanho;          // Tamanho total da tabela
    int quantidade;       // Contador de elementos cadastrados (para métricas e fator de carga)
} TabelaHash;

// Funções obrigatórias e utilitárias
TabelaHash* hash_criar();  // Define a função que cria a tabela, aloca memória e retorna o ponteiro dela
unsigned int hash_funcao(const char* str, int tamanho_tabela);  // Define a função matemática que transforma a string do usuário num índice (inteiro)
int hash_inserir(TabelaHash* h, const char* usuario);  // Define a função que insere um novo usuário na tabela e retorna 1 (sucesso) ou 0 (falha)
int hash_buscar(TabelaHash* h, const char* usuario);  // Define a função que procura por um usuário na tabela e retorna 1 (achou) ou 0 (não achou)
void hash_destruir(TabelaHash* h);  // Define a função que limpa todas as listas da memória e desaloca a tabela inteira

#endif
