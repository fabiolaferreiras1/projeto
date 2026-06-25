#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "bloom.h"

/*
   Implementação do Filtro de Bloom.

   Aqui sim usamos a <math.h>, porque o professor pediu que o
   dimensionamento do filtro (ou seja, o cálculo de m e k) seja feito
   pelas fórmulas matemáticas oficiais, e não por valores fixos
   "chutados" como na nossa versão mais simples anterior.

   As três funções de math.h que vamos usar são:

   log(x)   -> calcula o logaritmo NATURAL de x (logaritmo na base "e",
               o número de Euler, ~2,71828). É diferente do log na base 10
               que a gente usa na escola. Em matemática isso costuma ser
               chamado de "ln(x)", mas em C a função se chama só log().

   pow(x,y) -> calcula x elevado a y (x^y). A gente usa pow(log(2), 2) pra
               calcular o logaritmo natural de 2 elevado ao quadrado, que
               aparece no denominador da fórmula do m.

   ceil(x)  -> arredonda x SEMPRE pra cima, pro próximo número inteiro.
               Por exemplo, ceil(7.1) = 8.0 e ceil(7.9) = 8.0 também.
               Usamos isso porque m e k representam quantidade de bits e
               de funções hash, e não faz sentido ter "7,3 bits" ou
               "4,2 funções hash" - sempre arredondamos pra cima pra
               garantir que o filtro não fique menor (e pior) do que o
               necessário.
*/


/*
   Primeira função hash: DJB2.
   Copiei essa função exatamente igual à que o Integrante 1 usou no
   hash.c, só pra manter o mesmo padrão no projeto todo.

   Ela passa por cada letra da string e, a cada letra, multiplica o hash
   acumulado por 33 e soma o valor da letra. A conta
   "hash = (hash << 5) + hash + c" é só um jeito mais rápido de fazer
   "hash = hash * 33 + c", porque deslocar 5 bits pra esquerda (<< 5)
   equivale a multiplicar por 32, e somando mais um "hash" dá os 33 no
   total. O número inicial 5381 e o multiplicador 33 são valores que o
   criador do DJB2 testou bastante e viu que davam uma boa distribuição.
*/
unsigned int hash_djb2(const char* str) {
    unsigned long hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }

    return hash;
}

/*
   Segunda função hash: SDBM.
   Usei essa pra ter uma segunda função diferente da DJB2, pra combinar
   as duas no Double Hashing. Ela segue a mesma ideia de ir passando
   letra por letra e embaralhando o número, só que com outra combinação
   de deslocamentos de bits, então ela espalha os valores de um jeito
   diferente da DJB2 (o que é justamente o que a gente quer: duas
   funções que não tenham padrões de colisão parecidos).
*/
unsigned int hash_sdbm(const char* str) {
    unsigned long hash = 0;
    int c;

    while ((c = *str++)) {
        hash = (hash << 6) + (hash << 16) - hash + c;
    }

    return hash;
}


/*
   Cria o filtro de Bloom, calculando m (tamanho do vetor em bits) e
   k (quantidade de funções hash) pelas fórmulas matemáticas oficiais.
*/
FiltroBloom* bloom_criar(int num_elementos_esperados, double taxa_falso_positivo_desejada) {

    FiltroBloom* filtro = (FiltroBloom*) malloc(sizeof(FiltroBloom));

    /*
       Fórmula do tamanho do vetor de bits (m):

           m = -(n * log(p)) / (log(2))^2

       Onde:
         n = num_elementos_esperados (quantos usuários esperamos guardar)
         p = taxa_falso_positivo_desejada (ex.: 0.01 pra 1% de erro)

       Por que essa fórmula faz sentido: quanto MAIS elementos a gente
       espera guardar (n maior), mais espalhados os bits vão precisar
       estar, então m precisa crescer. E quanto MENOR a taxa de falso
       positivo que a gente aceita (p menor), também precisamos de um
       vetor maior, porque um vetor pequeno satura rápido e começa a dar
       bit ligado pra tudo (gerando mais falso positivo).

       Sobre o log(p): como p é uma probabilidade, ela está sempre entre
       0 e 1 (por exemplo, 0.01). O logaritmo natural de um número entre
       0 e 1 é sempre NEGATIVO. Por isso a fórmula começa com um sinal de
       menos na frente: -(n * log(p)) vira positivo, porque estamos
       multiplicando um número negativo (log(p)) por -1.

       O denominador, pow(log(2), 2), é o logaritmo natural de 2 elevado
       ao quadrado - uma constante que vem da derivação matemática da
       fórmula (não precisa decorar o "porquê" disso, só saber que é uma
       constante fixa que ajusta a escala do resultado).
    */
    double m_calculado = -(num_elementos_esperados * log(taxa_falso_positivo_desejada))
                          / pow(log(2), 2);

    /*
       m_calculado é um número double (com casas decimais), mas a gente
       precisa de um número inteiro de bits. Usamos ceil() pra arredondar
       sempre pra CIMA - assim garantimos que o vetor nunca fique menor
       do que o matematicamente necessário (arredondar pra baixo
       deixaria a taxa de falso positivo real um pouco PIOR do que a
       desejada).
    */
    filtro->tamanho_bits = (int) ceil(m_calculado);

    /* garantia mínima pra não correr risco de um vetor pequeno demais ou
       até negativo, caso alguém chame a função com valores estranhos de
       n ou p */
    if (filtro->tamanho_bits < 80) {
        filtro->tamanho_bits = 80;
    }

    /*
       Fórmula da quantidade de funções hash (k):

           k = (m / n) * log(2)

       Onde:
         m = filtro->tamanho_bits (o tamanho que a gente acabou de calcular)
         n = num_elementos_esperados

       A razão m/n representa quantos bits do vetor "sobram", em média,
       pra cada elemento guardado. Multiplicando isso por log(2)
       (o logaritmo natural de 2, um valor fixo de aproximadamente
       0,693), chegamos no número de funções hash que minimiza a taxa de
       falso positivo PARA aquele m e aquele n específicos. Usar menos
       funções hash do que o ideal deixa o filtro "frouxo demais"
       (mais falso positivo); usar mais do que o ideal enche o vetor de
       bits ligados rápido demais (também aumentando o falso positivo).
    */
    double k_calculado = ((double) filtro->tamanho_bits / num_elementos_esperados) * log(2);

    /* de novo usamos ceil() pra arredondar k pra cima, garantindo que a
       gente nunca use MENOS funções hash do que o cálculo indicou */
    filtro->num_hashes = (int) ceil(k_calculado);

    /* garantia mínima: usar pelo menos 1 função hash, senão o filtro não
       funciona */
    if (filtro->num_hashes < 1) {
        filtro->num_hashes = 1;
    }

    filtro->total_inserido = 0;

    /*
       Como cada byte do vetor guarda 8 bits, pra saber quantos bytes
       precisamos alocar a gente divide o tamanho em bits por 8 e
       arredonda pra cima. A conta (tamanho_bits + 7) / 8 faz esse
       arredondamento pra cima usando só números inteiros, sem precisar
       chamar ceil() de novo (esse é um truque comum quando se trabalha
       com bits em C).
    */
    int num_bytes = (filtro->tamanho_bits + 7) / 8;

    /* calloc já entrega o vetor com todos os bytes zerados, ou seja,
       todos os bits começam desligados - que é exatamente o estado
       inicial certo, já que nenhum usuário foi inserido ainda */
    filtro->vetor_bits = (unsigned char*) calloc(num_bytes, sizeof(unsigned char));

    printf("[Bloom] Filtro criado: n=%d, p=%.4f -> m=%d bits (%d bytes), k=%d funcoes hash\n",
           num_elementos_esperados, taxa_falso_positivo_desejada,
           filtro->tamanho_bits, num_bytes, filtro->num_hashes);

    return filtro;
}


/*
   Insere um usuário no filtro.
   Calcula k posições diferentes pro usuário (usando Double Hashing) e
   liga (seta pra 1) o bit em cada uma dessas posições.
*/
void bloom_inserir(FiltroBloom* filtro, const char* usuario) {
    if (filtro == NULL || usuario == NULL) {
        return;
    }

    /* calculamos as duas funções hash de base uma vez só, antes do
       laço, porque o usuário não muda durante as k voltas - não faz
       sentido recalcular DJB2 e SDBM de novo a cada iteração */
    unsigned int hash1 = hash_djb2(usuario);
    unsigned int hash2 = hash_sdbm(usuario);

    /*
       Esse laço gera as k posições usando a técnica de Double Hashing,
       que combina nossas duas funções hash de base numa fórmula só:

           posicao_i = (hash1 + i * hash2) % tamanho_bits

       Repare que "i" vai de 0 até k-1, e como ele muda a cada volta do
       laço, a posição calculada também muda - mesmo usando sempre os
       mesmos hash1 e hash2 de base. É assim que conseguimos gerar k
       posições "independentes" usando só duas funções hash, em vez de
       precisar escrever k funções hash diferentes.
    */
    for (int i = 0; i < filtro->num_hashes; i++) {

        unsigned int combinado = hash1 + (i * hash2);

        /* o operador % (resto da divisão) garante que a posição final
           sempre caia dentro dos limites do vetor, entre 0 e
           tamanho_bits - 1 */
        int posicao = combinado % filtro->tamanho_bits;

        /* agora descobrimos em qual byte do vetor essa posição cai
           (cada byte guarda 8 bits, então dividimos por 8) e qual bit,
           dentro desse byte, queremos ligar (o resto da divisão por 8,
           que vai de 0 a 7) */
        int byte_index = posicao / 8;
        int bit_index = posicao % 8;

        /*
           Essa linha é o coração da inserção:

               vetor_bits[byte_index] |= (1 << bit_index);

           O "1 << bit_index" desloca o número 1 pra esquerda
           "bit_index" vezes, criando um byte com um único bit ligado
           bem na posição que queremos (todos os outros bits ficam em
           0). Por exemplo, se bit_index for 3, isso dá 00001000.

           O operador "|=" faz um OR bit a bit entre o byte que já
           estava no vetor e esse número novo, guardando o resultado de
           volta no mesmo lugar. O OR funciona assim: se QUALQUER um dos
           dois bits comparados for 1, o resultado é 1. Então, ao fazer
           OR com 00001000, a gente garante que o bit da posição 3 fique
           ligado (ou continue ligado, se já estivesse), sem bagunçar os
           outros bits que já existiam naquele byte - porque fazer OR
           com 0 nunca muda o valor original do bit.
        */
        filtro->vetor_bits[byte_index] |= (1 << bit_index);
    }

    filtro->total_inserido++;
}


/*
   Verifica se um usuário possivelmente está no filtro.
   Calcula as mesmas k posições usadas na inserção (com a mesma fórmula
   de Double Hashing) e checa se todos os bits estão ligados. Se um
   único bit estiver desligado, já dá pra afirmar com certeza que o
   usuário nunca foi inserido (essa é a principal garantia do Filtro de
   Bloom: ele nunca dá falso negativo).
*/
int bloom_pertence(FiltroBloom* filtro, const char* usuario) {
    if (filtro == NULL || usuario == NULL) {
        return 0;
    }

    unsigned int hash1 = hash_djb2(usuario);
    unsigned int hash2 = hash_sdbm(usuario);

    for (int i = 0; i < filtro->num_hashes; i++) {
        /* mesma conta usada na inserção, pra cair exatamente nas mesmas
           posições que foram (ou não) ligadas quando esse usuário foi
           cadastrado */
        unsigned int combinado = hash1 + (i * hash2);
        int posicao = combinado % filtro->tamanho_bits;

        int byte_index = posicao / 8;
        int bit_index = posicao % 8;

        /*
           Aqui é o teste do bit:

               (vetor_bits[byte_index] & (1 << bit_index)) != 0

           De novo criamos aquela "máscara" com um único bit 1 na
           posição que queremos checar. Só que agora, em vez de OR,
           usamos AND (&). O AND bit a bit só resulta em 1 quando OS
           DOIS lados comparados são 1 - como a máscara só tem um bit
           ligado, o resultado da operação só vai ser diferente de zero
           SE aquele bit específico também estiver ligado no vetor.

           Comparamos o resultado com "!= 0": se for diferente de zero,
           o bit estava ligado (1); se for igual a zero, o bit estava
           desligado (0).
        */
        int bit_ligado = (filtro->vetor_bits[byte_index] & (1 << bit_index)) != 0;

        /* achamos um bit desligado: já podemos parar e responder que o
           usuário com certeza não existe, sem precisar checar os bits
           restantes (propriedade do Filtro de Bloom: sem falso negativo) */
        if (bit_ligado == 0) {
            return 0;
        }
    }

    /* passamos pelos k bits e todos estavam ligados, então o usuário
       possivelmente existe (pode ser um falso positivo também, por
       isso a confirmação na Tabela Hash continua sendo necessária) */
    return 1;
}


/*
   Libera toda a memória que foi alocada com malloc/calloc.
   Primeiro libera o vetor de bits (que está "dentro" da struct, mas foi
   alocado separadamente), depois libera a struct principal.
*/
void bloom_destruir(FiltroBloom* filtro) {
    if (filtro == NULL) {
        return;
    }

    free(filtro->vetor_bits);
    free(filtro);
}