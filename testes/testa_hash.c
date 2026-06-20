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

    printf("\n=== ESTATÍSTICAS DA TABELA HASH ===\n");
    printf("Tamanho total da tabela (M): %d\n", h->tamanho);
    printf("Elementos inseridos (N): %d\n", h->quantidade);
    printf("Fator de Carga (N/M): %.2f\n", fator_carga);
    printf("Índices ocupados do vetor: %d (%.2f%%)\n", indices_ocupados, ((double)indices_ocupados / h->tamanho) * 100);
    printf("Maior número de colisões em uma única lista: %d\n", max_colisoes_num_index);
    printf("===================================\n\n");
}

int main() {
    // Inicializa o gerador de números aleatórios
    srand(time(NULL));

    printf("Criando a Tabela Hash...\n");
    TabelaHash* minha_hash = hash_criar();

    // 1. Teste de Inserção Individual Manual
    printf("\n[Teste 1] Inserindo usuários fixos...\n");
    hash_inserir(minha_hash, "joao123");
    hash_inserir(minha_hash, "maria098");
    hash_inserir(minha_hash, "pedro456");

    // 2. Teste de Busca Individual Manual
    printf("\n[Teste 2] Buscando usuarios fixos...\n");
    if (hash_buscar(minha_hash, "joao123")) {
        printf("-> 'joao123' encontrado! (Sucesso)\n");
    } else {
        printf("-> Erro: 'joao123' não foi encontrado.\n");
    }

    if (!hash_buscar(minha_hash, "inexistente777")) {
        printf("-> 'inexistente777' nao encontrado! (Sucesso)\n");
    } else {
        printf("-> Erro: Encontrou um usuario que nao foi inserido.\n");
    }

    // 3. Teste de Carga (Simulando o cenário do Experimento de 1.000 registros)
    printf("\n[Teste 3] Inserindo 1.000 usuarios aleatarios no formato do projeto...\n");
    char usuario_teste[12];
    char guardar_um_usuario_para_teste[12];

    for (int i = 0; i < 1000; i++) {
        gerar_nome_aleatorio(usuario_teste);
        hash_inserir(minha_hash, usuario_teste);
        
        // Salva o último elemento gerado para testarmos a busca depois
        if (i == 500) {
            strcpy(guardar_um_usuario_para_teste, usuario_teste);
        }
    }

    // Verificar se o elemento sorteado no meio do lote é encontrado com sucesso
    printf("Buscando usuario aleatorio gerado no lote (%s): ", guardar_um_usuario_para_teste);
    if (hash_buscar(minha_hash, guardar_um_usuario_para_teste)) {
        printf("Encontrado!\n");
    } else {
        printf("Erro ao encontrar elemento do lote.\n");
    }

    // 4. Análise de Métricas para o Relatório
    analisar_tabela_hash(minha_hash);

    // 5. Destruir a tabela e liberar memória
    printf("Limpando a memoria...\n");
    hash_destruir(minha_hash);
    printf("Teste finalizado com sucesso!\n");

    return 0;
}
