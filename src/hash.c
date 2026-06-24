#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hash.h"

// Inicializa a tabela alocando memória para a estrutura e para as listas
TabelaHash* hash_criar() {
  // 1. Aloca a estrutura principal da Tabela Hash
    TabelaHash* h = (TabelaHash*) malloc(sizeof(TabelaHash));
    h->tamanho = TAMANHO_HASH;
    h->quantidade = 0;
    
 // 2. Cria o vetor de listas encadeadas
    h->tabela = (No**) calloc(h->tamanho, sizeof(No*));
    return h;
}

/*
  ALGORITMO DE HASH DJB2 (Criado por Dan Bernstein)
  É considerado um dos melhores algoritmos de espalhamento para strings.
  Como ele funciona:
  Inicia com a constante prima 5381 (semente matemática testada empiricamente).
  O loop processa caractere por caractere (c).
  A expressão ((hash << 5) + hash) multiplica o valor acumulado por 33 de forma ultra-rápida. Deslocar 5 bits para a esquerda multiplica por 32,
  somando com o próprio hash resulta em 33.
  O uso do multiplicador 33 e de números primos gera um "Efeito Avalanche".
  Isso garante que strings muito parecidas (como nosso padrão de 8 letras e 3 números) 
  resultem em índices completamente distantes no vetor, reduzindo drasticamente as colisões.
  */

unsigned int hash_funcao(const char* str, int tamanho_tabela) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; 
    }
    return hash % tamanho_tabela; // Garante que o índice está dentro do vetor
}

/* Essa função insere no início da lista encadeada daquela posição (complexidade O(1))
  Esse processo garante que a inserção aconteça de forma instantânea
  em tempo constante O(1), pois manipulamos os ponteiros diretamente na cabeça da 
  lista sem precisar varrer todos os elementos anteriores até o final.
*/

int hash_inserir(TabelaHash* h, const char* usuario) {
    if (h == NULL || usuario == NULL) return 0;
    
    unsigned int indice = hash_funcao(usuario, h->tamanho);
    
    // Criar o novo nó
    No* novo = (No*) malloc(sizeof(No));
    strcpy(novo->usuario, usuario);
    
  // Insere no início da lista encadeada (Colisão resolvida por encadeamento)
    novo->proximo = h->tabela[indice];
  //O vetor da tabela irá apontar diretamente para este novo nó. Com isso, o novo elemento se torna oficialmente a nova "cabeça" da lista.
    h->tabela[indice] = novo;
  
    h->quantidade++;
    return 1; // Sucesso
}

/* Busca linear apenas na lista encadeada do índice gerado
  A função calcula o índice exato da string, indo direto para a posição 
  correta do vetor em vez de varrer a tabela inteira.

  O laço percorre apenas a pequena lista encadeada daquele índice específico, 
  comparando os nomes com strcmp até encontrar o usuário ou chegar ao fim (NULL).
  Graças ao bom espalhamento da função hash, as listas de colisão 
  são minúsculas. Isso faz com que a busca rode de forma quase instantânea, 
  mantendo a performance em tempo constante O(1).
*/

int hash_buscar(TabelaHash* h, const char* usuario) {
    if (h == NULL || usuario == NULL) return 0;
    
    unsigned int indice = hash_funcao(usuario, h->tamanho);
    No* atual = h->tabela[indice];
    
    // Percorre a lista encadeada daquela posição específica
    while (atual != NULL) {
        if (strcmp(atual->usuario, usuario) == 0) {
            return 1; // (Usuário encontrado)
        }
        atual = atual->proximo;
    }
    return 0; // (Usuário não encontrado)
}

/* Função para liberar a memória
  O laço 'for' percorre cada uma das posições do vetor principal da tabela.
  
  O laço 'while' varre a lista encadeada de cada posição usando o ponteiro 'aux' 
  para segurar o nó atual e liberá-lo com 'free' somente após o ponteiro 'atual' 
  ter avançado com segurança para o próximo nó.
  Ao final, a função libera o vetor de ponteiros e a estrutura da tabela em si, 
  evitando qualquer vazamento de memória (memory leak) no sistema.
*/

void hash_destruir(TabelaHash* h) {
    if (h == NULL) return;
    for (int i = 0; i < h->tamanho; i++) {
        No* atual = h->tabela[i];
        while (atual != NULL) {
            No* aux = atual;
            atual = atual->proximo;
            free(aux);
        }
    }
    free(h->tabela);
    free(h);
}
