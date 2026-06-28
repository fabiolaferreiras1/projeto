#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "bloom.h"

// Implementação do Filtro de Bloom.
// math.h é usado pra calcular m e k pelas fórmulas oficiais (log, pow, ceil).


// DJB2 - mesma função usada no hash.c do Integrante 1, pra manter padrão.
unsigned int hash_djb2(const char* str) {
    unsigned long hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }

    return hash;
}

// SDBM - segunda função hash, com espalhamento diferente da DJB2.
unsigned int hash_sdbm(const char* str) {
    unsigned long hash = 0;
    int c;

    while ((c = *str++)) {
        hash = (hash << 6) + (hash << 16) - hash + c;
    }

    return hash;
}


// Cria o filtro e dimensiona m e k de acordo com n e p.
FiltroBloom* bloom_criar(int num_elementos_esperados, double taxa_falso_positivo_desejada) {

    FiltroBloom* filtro = (FiltroBloom*) malloc(sizeof(FiltroBloom));

    // m = -(n * ln p) / (ln 2)^2
    double m_calculado = -(num_elementos_esperados * log(taxa_falso_positivo_desejada))
                          / pow(log(2), 2);

    filtro->tamanho_bits = (int) ceil(m_calculado);

    if (filtro->tamanho_bits < 80) {
        filtro->tamanho_bits = 80;
    }

    // k = (m/n) * ln 2
    double k_calculado = ((double) filtro->tamanho_bits / num_elementos_esperados) * log(2);

    filtro->num_hashes = (int) ceil(k_calculado);

    if (filtro->num_hashes < 1) {
        filtro->num_hashes = 1;
    }

    filtro->total_inserido = 0;

    // bits -> bytes, arredondando pra cima
    int num_bytes = (filtro->tamanho_bits + 7) / 8;

    // calloc já zera tudo (todos os bits começam desligados)
    filtro->vetor_bits = (unsigned char*) calloc(num_bytes, sizeof(unsigned char));

    printf("[Bloom] Filtro criado: n=%d, p=%.4f -> m=%d bits (%d bytes), k=%d funcoes hash\n",
           num_elementos_esperados, taxa_falso_positivo_desejada,
           filtro->tamanho_bits, num_bytes, filtro->num_hashes);

    return filtro;
}


// Insere o usuário ligando k bits no vetor (Double Hashing).
void bloom_inserir(FiltroBloom* filtro, const char* usuario) {
    if (filtro == NULL || usuario == NULL) {
        return;
    }

    unsigned int hash1 = hash_djb2(usuario);
    unsigned int hash2 = hash_sdbm(usuario);

    for (int i = 0; i < filtro->num_hashes; i++) {

        // posicao_i = (hash1 + i*hash2) % m
        unsigned int combinado = hash1 + (i * hash2);
        int posicao = combinado % filtro->tamanho_bits;

        int byte_index = posicao / 8;
        int bit_index = posicao % 8;

        // liga o bit sem afetar os outros do mesmo byte
        filtro->vetor_bits[byte_index] |= (1 << bit_index);
    }

    filtro->total_inserido++;
}


// Verifica se os k bits do usuário estão todos ligados.
int bloom_pertence(FiltroBloom* filtro, const char* usuario) {
    if (filtro == NULL || usuario == NULL) {
        return 0;
    }

    unsigned int hash1 = hash_djb2(usuario);
    unsigned int hash2 = hash_sdbm(usuario);

    for (int i = 0; i < filtro->num_hashes; i++) {
        unsigned int combinado = hash1 + (i * hash2);
        int posicao = combinado % filtro->tamanho_bits;

        int byte_index = posicao / 8;
        int bit_index = posicao % 8;

        // testa se aquele bit específico está ligado
        int bit_ligado = (filtro->vetor_bits[byte_index] & (1 << bit_index)) != 0;

        // achou bit desligado -> com certeza não existe
        if (bit_ligado == 0) {
            return 0;
        }
    }

    // todos os bits ligados -> possivelmente existe
    return 1;
}


// Libera a memória do filtro (vetor de bits primeiro, depois a struct).
void bloom_destruir(FiltroBloom* filtro) {
    if (filtro == NULL) {
        return;
    }

    free(filtro->vetor_bits);
    free(filtro);
}