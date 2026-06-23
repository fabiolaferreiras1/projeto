#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bloom.h"

unsigned int hash_djb2(const char* str) {
    unsigned long hash = 5381; 
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; 
    }

    return hash; 
}

unsigned int hash_sdbm(const char* str) {
    unsigned long hash = 0; 
    int c;

    while ((c = *str++)) {
        hash = (hash << 6) + (hash << 16) - hash + c;
    }

    return hash;
}

FiltroBloom* bloom_criar(int num_elementos_esperados, double taxa_falso_positivo_desejada) {
    (void) taxa_falso_positivo_desejada;

    FiltroBloom* filtro = (FiltroBloom*) malloc(sizeof(FiltroBloom));

    filtro->tamanho_bits = num_elementos_esperados * 10;

    if (filtro->tamanho_bits < 80) {
        filtro->tamanho_bits = 80;
    }

    filtro->num_hashes = 7;

    filtro->total_inserido = 0;

    int num_bytes = (filtro->tamanho_bits + 7) / 8;

    filtro->vetor_bits = (unsigned char*) calloc(num_bytes, sizeof(unsigned char));

    printf("[Bloom] Filtro criado: n=%d -> m=%d bits (%d bytes), k=%d funcoes hash\n",
           num_elementos_esperados, filtro->tamanho_bits, num_bytes, filtro->num_hashes);

    return filtro;
}

void bloom_inserir(FiltroBloom* filtro, const char* usuario) {
    if (filtro == NULL || usuario == NULL) {
        return; 
    }

    unsigned int hash1 = hash_djb2(usuario);
    unsigned int hash2 = hash_sdbm(usuario);

    for (int i = 0; i < filtro->num_hashes; i++) {

        unsigned int combinado = hash1 + (i * hash2);

        int posicao = combinado % filtro->tamanho_bits;

        int byte_index = posicao / 8;
        int bit_index = posicao % 8;

        filtro->vetor_bits[byte_index] |= (1 << bit_index);
    }

    filtro->total_inserido++;
}

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

        int bit_ligado = (filtro->vetor_bits[byte_index] & (1 << bit_index)) != 0;

        if (bit_ligado == 0) {
            return 0; 
        }
    }

    return 1; 
}

void bloom_destruir(FiltroBloom* filtro) {
    if (filtro == NULL) {
        return;
    }

    free(filtro->vetor_bits); 
    free(filtro);             
}