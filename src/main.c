#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <locale.h>
#include "hash.h"
#include "bloom.h"

// Estrutura para armazenar as métricas do sistema
typedef struct {
    int elementos_armazenados;
    int consultas_realizadas;
    int consultas_evitadas_bloom;
    int falsos_positivos;
    double tempo_total_consultas; // em milissegundos
} Estatisticas;

// Variável global de estatísticas
Estatisticas stats = {0, 0, 0, 0, 0.0};

// Função auxiliar para limpar o buffer do teclado
void limpar_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// RF01 - Inserção
void cadastrar_usuario(FiltroBloom* bloom, TabelaHash* hash, const char* usuario) {
    bloom_inserir(bloom, usuario);
    hash_inserir(hash, usuario);
    stats.elementos_armazenados++;
    printf("Usuário '%s' inserido com sucesso!\n", usuario);
}

// RF02 - Consulta (Fluxo Obrigatório com medição de tempo)
void consultar_usuario(FiltroBloom* bloom, TabelaHash* hash, const char* usuario) {
    clock_t inicio_tempo = clock();
    stats.consultas_realizadas++;

    // 1. Consultar o Filtro de Bloom
    int pertence_bloom = bloom_pertence(bloom, usuario);

    if (pertence_bloom == 0) {
        // 2. Filtro indicou "definitivamente não existe"
        stats.consultas_evitadas_bloom++;
        printf("-> Usuário inexistente (bloqueado pelo Filtro de Bloom)\n");
    } else {
        // 3. Indicou "possivelmente existe", consultar a Tabela Hash
        int pertence_hash = hash_buscar(hash, usuario);
        
        if (pertence_hash == 1) {
            // 4. Resultado final: Encontrado
            printf("-> Usuário encontrado na Tabela Hash!\n");
        } else {
            // 4. Resultado final: Falso Positivo
            stats.falsos_positivos++;
            printf("-> Usuário inexistente (falso positivo do Filtro de Bloom detectado)\n");
        }
    }

    clock_t fim_tempo = clock();
    double tempo_gasto = ((double)(fim_tempo - inicio_tempo) / CLOCKS_PER_SEC) * 1000.0; // Converter para ms
    stats.tempo_total_consultas += tempo_gasto;
}

// RF03 - Estatísticas
void exibir_estatisticas() {
    double taxa_falsos_positivos = 0.0;
    int consultas_que_passaram_pelo_bloom = stats.consultas_realizadas - stats.consultas_evitadas_bloom;

    if (consultas_que_passaram_pelo_bloom > 0) {
        taxa_falsos_positivos = ((double)stats.falsos_positivos / consultas_que_passaram_pelo_bloom) * 100.0;
    }

    double tempo_medio = 0.0;
    if (stats.consultas_realizadas > 0) {
        tempo_medio = stats.tempo_total_consultas / stats.consultas_realizadas;
    }

    printf("\n--- ESTATÍSTICAS DO SISTEMA ---\n");
    printf("Elementos armazenados: %d\n", stats.elementos_armazenados);
    printf("Consultas realizadas: %d\n", stats.consultas_realizadas);
    printf("Consultas evitadas pela Bloom: %d\n", stats.consultas_evitadas_bloom);
    printf("Número de falsos positivos: %d\n", stats.falsos_positivos);
    printf("Taxa de falsos positivos: %.2f%%\n", taxa_falsos_positivos);
    printf("Tempo médio de consulta: %.6f ms\n", tempo_medio);
    printf("-------------------------------\n");
}

// RF04 - Inserir em Lote
void inserir_em_lote(FiltroBloom* bloom, TabelaHash* hash, const char* nome_arquivo) {
    char caminho_completo[150];
    snprintf(caminho_completo, sizeof(caminho_completo), "data/%s", nome_arquivo);

    FILE* arquivo = fopen(caminho_completo, "r");

    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo '%s'.\n", nome_arquivo);
        return;
    }

    char usuario[20];
    int cont = 0;
    while (fscanf(arquivo, "%19s", usuario) == 1) {
        bloom_inserir(bloom, usuario);
        hash_inserir(hash, usuario);
        stats.elementos_armazenados++;
        cont++;
    }

    fclose(arquivo);
    printf("%d registros inseridos em lote a partir de '%s'.\n", cont, nome_arquivo);
}

// Função utilitária para gerar arquivo de testes (8 letras e 3 números)
void gerar_massa_dados(const char* nome_arquivo, int qtd) {
    char caminho_completo[150];
    snprintf(caminho_completo, sizeof(caminho_completo), "data/%s", nome_arquivo);

    FILE* arquivo = fopen(caminho_completo, "w");
    
    if (arquivo == NULL) {
        printf("Erro ao criar arquivo.\n");
        return;
    }

    srand(time(NULL));
    for (int i = 0; i < qtd; i++) {
        char usuario[12];
        for (int j = 0; j < 8; j++) {
            usuario[j] = 'a' + (rand() % 26); // 8 letras aleatórias
        }
        for (int j = 8; j < 11; j++) {
            usuario[j] = '0' + (rand() % 10); // 3 números aleatórios
        }
        usuario[11] = '\0';
        fprintf(arquivo, "%s\n", usuario);
    }
    fclose(arquivo);
    printf("Arquivo '%s' gerado com %d registros.\n", nome_arquivo, qtd);
}

// Função para automatizar os experimentos da Parte 3
void executar_experimentos(FiltroBloom* bloom, TabelaHash* hash, const char* nome_arquivo) {
    char caminho_completo[150];
    snprintf(caminho_completo, sizeof(caminho_completo), "data/%s", nome_arquivo);

    FILE* arquivo = fopen(caminho_completo, "r");
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo '%s' para os testes.\n", caminho_completo);
        return;
    }

    // Carregar todas as palavras do arquivo para um array na memória 
    // (para que o tempo de leitura do disco (I/O) não suje o tempo de busca da Hash)
    char usuarios_teste[100000][12];
    int qtd_testes = 0;
    while (fscanf(arquivo, "%11s", usuarios_teste[qtd_testes]) == 1 && qtd_testes < 100000) {
        qtd_testes++;
    }
    fclose(arquivo);

    printf("\n=== EXPERIMENTO COM %d REGISTROS ===\n", qtd_testes);

    // Busca SEM Bloom (Direto na Hash)
    clock_t inicio_sem = clock();
    for (int i = 0; i < qtd_testes; i++) {
        hash_buscar(hash, usuarios_teste[i]);
    }
    clock_t fim_sem = clock();
    double tempo_sem_bloom = ((double)(fim_sem - inicio_sem) / CLOCKS_PER_SEC) * 1000.0;

    // Busca COM Bloom (Fluxo completo)
    int falsos_positivos_teste = 0;
    clock_t inicio_com = clock();
    
    for (int i = 0; i < qtd_testes; i++) {
        if (bloom_pertence(bloom, usuarios_teste[i]) == 1) {
            // Bloom disse que possivelmente existe, verifica na Hash
            if (hash_buscar(hash, usuarios_teste[i]) == 0) {
                falsos_positivos_teste++;
            }
        }
    }
    
    clock_t fim_com = clock();
    double tempo_com_bloom = ((double)(fim_com - inicio_com) / CLOCKS_PER_SEC) * 1000.0;

    printf("Tempo SEM Filtro de Bloom: %.4f ms\n", tempo_sem_bloom);
    printf("Tempo COM Filtro de Bloom: %.4f ms\n", tempo_com_bloom);
    
    double percentual_fp = 0.0;
    if (qtd_testes > 0) {
        percentual_fp = ((double)falsos_positivos_teste / qtd_testes) * 100.0;
    }
    printf("Falsos Positivos: %d (%.4f%%)\n", falsos_positivos_teste, percentual_fp);
}

int main() {

    setlocale(LC_ALL, "Portuguese");

    // Inicialização das estruturas
    // O tamanho 100.000 é usado como estimativa inicial baseada nos experimentos exigidos
    FiltroBloom* bloom = bloom_criar(100000, 0.01); 
    TabelaHash* hash = hash_criar(100003);

    int opcao;
    char usuario[20];
    char nome_arquivo[50];

    do {
        printf("\n=== MENU DE VERIFICAÇÃO DE USUÁRIOS ===\n");
        printf("1 - (RF01) Inserir usuário\n");
        printf("2 - (RF02) Consultar usuário\n");
        printf("3 - (RF03) Exibir estatísticas\n");
        printf("4 - (RF04) Inserir em lote (por meio de arquivo .txt)\n");
        printf("5 - Gerar massa de dados (para testes)\n");
        printf("6 - Executar experimentos\n");
        printf("0 - Sair\n");
        printf("Escolha uma opção: ");
        
        if (scanf("%d", &opcao) != 1) {
            limpar_buffer();
            opcao = -1;
        }

        switch (opcao) {
            case 1:
                printf("Digite o nome do usuário (ex: joao123): ");
                scanf("%19s", usuario);
                cadastrar_usuario(bloom, hash, usuario);
                break;
            case 2:
                printf("Digite o nome do usuário para consulta: ");
                scanf("%19s", usuario);
                consultar_usuario(bloom, hash, usuario);
                break;
            case 3:
                exibir_estatisticas();
                break;
            case 4:
                printf("Digite o nome do arquivo (ex: usuarios.txt): ");
                scanf("%49s", nome_arquivo);
                inserir_em_lote(bloom, hash, nome_arquivo);
                break;
            case 5: {
                int qtd;
                printf("Digite a quantidade de registros (ex: 1000, 10000, 100000): ");
                scanf("%d", &qtd);
                printf("Digite o nome do arquivo de saida (ex: arquivo.txt): ");
                scanf("%49s", nome_arquivo);
                gerar_massa_dados(nome_arquivo, qtd);
                break;
            }
            case 6:
                printf("Digite o nome do arquivo para teste (ex: 1000.txt): ");
                scanf("%49s", nome_arquivo);
                executar_experimentos(bloom, hash, nome_arquivo);
                break;
            case 0:
                printf("Encerrando o sistema e liberando memória...\n");
                break;
            default:
                printf("Opção inválida. Tente novamente!\n");
        }
    } while (opcao != 0);

    // Liberação de memória exigida
    bloom_destruir(bloom);
    hash_destruir(hash);

    return 0;
}