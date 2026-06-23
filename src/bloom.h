#ifndef BLOOM_H
#define BLOOM_H

/*
   Filtro de Bloom - parte do Integrante 2

   A ideia aqui é bem simples: em vez de guardar o nome de cada usuário
   inteiro, a gente guarda só uns "bitzinhos" ligados ou desligados em um
   vetor. Quando alguém pergunta se um usuário existe, a gente calcula umas
   posições nesse vetor e olha se os bits estão todos em 1.

   Se algum bit estiver em 0, é 100% certeza que o usuário nunca foi
   cadastrado. Mas se todos estiverem em 1, não dá pra ter certeza total -
   pode ser que o usuário realmente exista, ou pode ser coincidência (isso
   é o que o trabalho chama de "falso positivo"). Por isso, depois de
   passar pelo Bloom, a gente ainda confirma na Tabela Hash de verdade.
*/

/*
   Struct principal do filtro.

   vetor_bits     -> onde os bits ficam guardados (em bytes, 8 bits por byte)
   tamanho_bits   -> tamanho total do vetor em bits (isso é o "m")
   num_hashes     -> quantas vezes a gente marca um bit pra cada usuário (o "k")
   total_inserido -> só pra contar quantos usuários já foram inseridos,
                      ajuda nas estatísticas depois
*/
typedef struct {
    unsigned char* vetor_bits;
    int tamanho_bits;
    int num_hashes;
    int total_inserido;
} FiltroBloom;

/*
   Cria o filtro já calculando o tamanho do vetor e a quantidade de hashes.
   O segundo parâmetro (taxa de falso positivo) ficou na assinatura porque
   o enunciado pede, mas a gente não usa ele numa fórmula complicada - o
   tamanho do vetor é definido de um jeito mais direto, dá pra ver melhor
   como em bloom.c.
*/
FiltroBloom* bloom_criar(int num_elementos_esperados, double taxa_falso_positivo_desejada);

/* Insere um usuário no filtro (liga os bits dele) */
void bloom_inserir(FiltroBloom* filtro, const char* usuario);

/*
   Verifica se o usuário possivelmente está no filtro.
   Retorna 1 se talvez exista, 0 se com certeza não existe.
*/
int bloom_pertence(FiltroBloom* filtro, const char* usuario);

/* Libera a memória usada pelo filtro */
void bloom_destruir(FiltroBloom* filtro);

#endif