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
    printf("Indices ocupados do vetor: %d (%.2f%%)\n", indices_ocupados, ((double)indices_ocupados / h->tamanho) * 100);
    printf("Maior numero de colisoes em uma unica lista: %d\n", max_colisoes_num_index);
    printf("===================================\n\n");
}

int main() {
    // Altera a semente do rand para garantir nomes diferentes a cada execução
    srand(time(NULL));

    printf("==================================================\n");
    printf("   PASSO 1: EXECUTANDO TESTES UNITARIOS ISOLADOS   \n");
    printf("==================================================\n");

    // 1. Cria uma tabela de testes
    TabelaHash* h_teste = hash_criar(10009);    if (h_teste == NULL) {
        printf("[ERRO] Falha ao criar a tabela hash para os testes.\n");
        return 1;
    }
    printf("[OK] hash_criar: Tabela alocada com sucesso.\n");
    printf("[OK] Tamanho verificado: %d\n", h_teste->tamanho);

    // 2. Testa a Inserção de unidades controladas
    int ins1 = hash_inserir(h_teste, "useralex123");
    int ins2 = hash_inserir(h_teste, "userjoao456");
    
    if (ins1 == 1 && ins2 == 1 && h_teste->quantidade == 2) {
        printf("[OK] hash_inserir: Elementos inseridos e contadores atualizados.\n");
    } else {
        printf("[ERRO] hash_inserir: Falha na contagem ou no retorno de insercao.\n");
    }

    // 3. Testa a Busca (Unidade de Sucesso e Unidade de Falha)
    int busca_sucesso = hash_buscar(h_teste, "useralex123");
    int busca_falha = hash_buscar(h_teste, "naoexiste00");

    if (busca_sucesso == 1) {
        printf("[OK] hash_buscar: Encontrou corretamente um usuario cadastrado.\n");
    } else {
        printf("[ERRO] hash_buscar: Nao encontrou um usuario que deveria existir.\n");
    }

    if (busca_falha == 0) {
        printf("[OK] hash_buscar: Retornou 0 corretamente para usuario inexistente.\n");
    } else {
        printf("[ERRO] hash_buscar: Encontrou um fantasma (usuario nao cadastrado).\n");
    }

    // 4. Limpa a tabela de teste unitário
    hash_destruir(h_teste);
    printf("[OK] hash_destruir: Memoria dos testes unitarios liberada.\n");


    printf("\n==================================================\n");
    printf("   PASSO 2: EXECUTANDO EXPERIMENTOS DE ESTRESSE   \n");
    printf("==================================================\n");

    // Vetor contendo os três volumes de dados exigidos no projeto
    int volumes_testes[] = {1000, 10000, 100000};
    int tamanhos_primos[] = {1009, 10009, 100009};

    // Loop que vai rodar 3 vezes (uma para cada volume de dados)
    for (int t = 0; t < 3; t++) {
        int limite_usuarios = volumes_testes[t];
        int tamanho_atual = tamanhos_primos[t];

        // 1. Cria uma tabela limpa para o teste de volume atual
        TabelaHash* h = hash_criar(tamanho_atual);
        
        // 2. Inserção em lote dos N usuários aleatórios
        for (int i = 0; i < limite_usuarios; i++) {
            char usuario_gerado[12];
            gerar_nome_aleatorio(usuario_gerado); 
            hash_inserir(h, usuario_gerado);
        }

        // 3. Inserção de uma chave conhecida com o formato correto (8 letras + 3 números)
        char chave_controle[] = "testepre123";
        hash_inserir(h, chave_controle);

        // 4. Executa a análise estatística de colisões para este volume
        printf("\n=> ANALISE QUANTITATIVA PARA %d ELEMENTOS:", limite_usuarios);
        analisar_tabela_hash(h);

        // 5. Teste unitário da busca atuando dentro de uma tabela massiva
        int achou_controle = hash_buscar(h, chave_controle);
        if (achou_controle == 1) {
            printf("Validacao da Busca no Volume de %d: SUCESSO (Chave encontrada)\n", limite_usuarios);
        } else {
            printf("Validacao da Busca no Volume de %d: FALHA (Chave de controle sumiu)\n", limite_usuarios);
        }
        printf("--------------------------------------------------\n");

        // 6. Limpa totalmente a memória antes de começar o próximo volume
        hash_destruir(h);
    }

    printf("\n=== TODOS OS TESTES E EXPERIMENTOS CONCLUIDOS ===\n");
    return 0;
}
