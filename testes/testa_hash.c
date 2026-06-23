#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "hash.h"

// Função auxiliar para gerar nomes aleatórios no formato exigido: [8 caracteres][3 numeros]
void gerar_nome_aleatorio(char* buffer) {
    char letras[] = "abcdefghijklmnopqrstuvwxyz";
    char numeros[] = "0123456789";
    
    // Gera os 8 caracteres alfanuméricos/letras
    for (int i = 0; i < 8; i++) {
        buffer[i] = letras[rand() % 26];
    }
    // Gera os 3 números
    for (int i = 8; i < 11; i++) {
        buffer[i] = numeros[rand() % 10];
    }
    buffer[11] = '\0'; // Fim da string
}

// Função para analisar a distribuição e colisões na tabela
void analisar_tabela_hash(TabelaHash* h) {
    int indices_ocupados = 0;
    int max_colisoes_num_index = 0;

    for (int i = 0; i < h->tamanho; i++) {
        if (h->tabela[i] != NULL) {
            indices_ocupados++;
            int colisoes_aqui = 0;
            No* atual = h->tabela[i]->proximo; // O primeiro nó não é colisão, os seguintes sim
            
            while (atual != NULL) {
                colisoes_aqui++;
                atual = atual->proximo;
            }
            if (colisoes_aqui > max_colisoes_num_index) {
                max_colisoes_num_index = colisoes_aqui;
            }
        }
    }

    double fator_carga = (double)h->quantidade / h->tamanho;

    printf("\n=== ESTATISTICAS DA TABELA HASH ===\n");
    printf("Tamanho total da tabela (M): %d\n", h->tamanho);
    printf("Elementos inseridos (N): %d\n", h->quantidade);
    printf("Fator de Carga (N/M): %.2f\n", fator_carga);
    printf("Índices ocupados do vetor: %d (%.2f%%)\n", indices_ocupados, ((double)indices_ocupados / h->tamanho) * 100);
    printf("Maior numero de colisoes em uma unica lista: %d\n", max_colisoes_num_index);
    printf("===================================\n\n");
}

int main() {
    // Vetor contendo os três volumes de dados exigidos no projeto
    int volumes_testes[] = {1000, 10000, 100000};
    
    // Um número primo grande para aguentar os testes sem estourar a memória
    int tamanho_tabela = 100009; 

    printf("=== INICIANDO EXPERIMENTOS DA PARTE 3 ===\n");

    // Loop que vai rodar 3 vezes (uma para cada volume de dados)
    for (int t = 0; t < 3; t++) {
        int limite_usuarios = volumes_testes[t];

        // 1. Cria uma tabela limpa para o teste atual
        TabelaHash* h = hash_criar(tamanho_tabela);

        // 2. Inserção em lote dos N usuários aleatórios
        for (int i = 0; i < limite_usuarios; i++) {
            char usuario_gerado[12];
            gerar_nome_aleatorio(usuario_gerado); // Sua função de gerar nomes
            hash_inserir(h, usuario_gerado);
        }

        // 3. Executa a análise de colisões que criamos para este volume
        printf("\n=========================================");
        printf("\n=> RESULTADOS PARA %d USUARIOS:", limite_usuarios);
        analisar_tabela_hash(h);; 
        printf("=========================================\n");

        // 4. Limpa totalmente a memória antes de começar o próximo volume
        hash_destruir(h);
    }

    printf("\n=== EXPERIMENTOS CONCLUIDOS COM SUCESSO ===\n");
    return 0;
}
