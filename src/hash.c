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

// Algoritmo de Hash DJB2 manual
unsigned int hash_funcao(const char* str, int tamanho_tabela) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; 
    }
    return hash % tamanho_tabela; // Garante que o índice está dentro do vetor
}

// Insere no início da lista encadeada daquela posição (complexidade O(1))
int hash_inserir(TabelaHash* h, const char* usuario) {
    if (h == NULL || usuario == NULL) return 0;
    
    unsigned int indice = hash_funcao(usuario, h->tamanho);
    
    // Criar o novo nó
    No* novo = (No*) malloc(sizeof(No));
    strcpy(novo->usuario, usuario);
    
    // Insere no início da lista encadeada (Colisão resolvida por encadeamento)
    novo->proximo = h->tabela[indice];
    h->tabela[indice] = novo;
    
    h->quantidade++;
    return 1; // Sucesso
}

// Busca linear apenas na lista encadeada do índice gerado
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

// Função para liberar a memória
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
