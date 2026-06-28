#ifndef BLOOM_H
#define BLOOM_H

// Filtro de Bloom - Integrante 2
// Estrutura probabilística: guarda só bits, não as strings dos usuários.
// Se um bit estiver desligado, o usuário com certeza não foi inserido.
// Se todos os bits estiverem ligados, pode ser que exista (ou falso positivo).

// Dados do filtro: vetor de bits, tamanho do vetor (m), quantidade de
// funções hash (k) e um contador de quantos usuários já foram inseridos.
typedef struct {
    unsigned char* vetor_bits;
    int tamanho_bits;
    int num_hashes;
    int total_inserido;
} FiltroBloom;

// Cria o filtro calculando m e k pelas fórmulas matemáticas do Bloom,
// a partir do número de elementos esperados (n) e da taxa de falso
// positivo desejada (p).
FiltroBloom* bloom_criar(int num_elementos_esperados, double taxa_falso_positivo_desejada);

// Insere um usuário, ligando os bits dele no vetor.
void bloom_inserir(FiltroBloom* filtro, const char* usuario);

// Consulta se o usuário possivelmente está no filtro (1) ou com
// certeza não está (0).
int bloom_pertence(FiltroBloom* filtro, const char* usuario);

// Libera a memória do filtro.
void bloom_destruir(FiltroBloom* filtro);

#endif