#ifndef BLOOM_H
#define BLOOM_H
typedef struct {
    unsigned char* vetor_bits;   
    int tamanho_bits;            
    int num_hashes;              
    int total_inserido;          
} FiltroBloom;

FiltroBloom* bloom_criar(int num_elementos_esperados, double taxa_falso_positivo_desejada);

void bloom_inserir(FiltroBloom* filtro, const char* usuario);

int bloom_pertence(FiltroBloom* filtro, const char* usuario);

void bloom_destruir(FiltroBloom* filtro);

#endif