#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bloom.h"

/*
   Implementação do Filtro de Bloom.

   Não usei math.h nem nada de logaritmo aqui pra não complicar a
   explicação na apresentação. O tamanho do vetor e a quantidade de
   funções hash são definidos de um jeito mais direto, baseado nos
   cenários do trabalho (1.000, 10.000 e 100.000 registros).
*/

/*
   Primeira função hash: DJB2.
   Copiei essa daqui exatamente igual à que o Integrante 1 usou no hash.c,
   só pra manter o mesmo padrão no projeto todo.

   Basicamente ela vai passando por cada letra da string e a cada letra
   multiplica o hash por 33 e soma o valor da letra. O "(hash << 5) + hash"
   é só um jeito mais rápido de fazer essa multiplicação por 33 (porque
   deslocar 5 bits pra esquerda equivale a multiplicar por 32, e somando
   mais um hash dá os 33). Esse 33 e o número inicial 5381 são só valores
   que o cara que criou o DJB2 testou e viu que funcionavam bem.
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
   as duas depois. Ela segue a mesma lógica de ir passando letra por letra
   e embaralhando o número, só que com outra combinação de deslocamentos,
   então ela espalha os valores de um jeito um pouco diferente.
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
   Cria o filtro de Bloom.
   Aqui é onde decido o tamanho do vetor de bits e quantas funções hash
   usar, baseado em quantos elementos a gente espera guardar.
*/
FiltroBloom* bloom_criar(int num_elementos_esperados, double taxa_falso_positivo_desejada) {
    /* não uso esse parâmetro em conta nenhuma, só deixei na assinatura
       porque o enunciado pedia esse formato */
    (void) taxa_falso_positivo_desejada;

    FiltroBloom* filtro = (FiltroBloom*) malloc(sizeof(FiltroBloom));

    /*
       Pra decidir o tamanho do vetor (m), usei uma regra simples: 10 bits
       pra cada elemento esperado. Essa proporção é bem usada em exemplos
       de Filtro de Bloom e funciona bem combinada com 7 funções hash,
       dando uma taxa de falso positivo baixa (perto de 1%), que é o
       suficiente pros testes do trabalho.

       Nos cenários pedidos isso dá:
         1.000 registros   -> 10.000 bits
         10.000 registros  -> 100.000 bits
         100.000 registros -> 1.000.000 bits
    */
    filtro->tamanho_bits = num_elementos_esperados * 10;

    /* só uma garantia boba pra não criar um vetor de tamanho 0 se alguém
       passar 0 como número de elementos esperados */
    if (filtro->tamanho_bits < 80) {
        filtro->tamanho_bits = 80;
    }

    /* número de funções hash fixo em 7 - é um valor clássico quando se
       usa essa proporção de 10 bits por elemento */
    filtro->num_hashes = 7;

    filtro->total_inserido = 0;

    /*
       Como cada byte guarda 8 bits, pra saber quantos bytes preciso
       alocar eu divido o tamanho em bits por 8 e arredondo pra cima.
       A conta (tamanho_bits + 7) / 8 faz esse arredondamento sem
       precisar de função de matemática nenhuma, só com inteiro mesmo.
    */
    int num_bytes = (filtro->tamanho_bits + 7) / 8;

    /* uso calloc porque ele já zera tudo - assim todos os bits começam
       desligados, que é o estado inicial certo (nenhum usuário inserido ainda) */
    filtro->vetor_bits = (unsigned char*) calloc(num_bytes, sizeof(unsigned char));

    printf("[Bloom] Filtro criado: n=%d -> m=%d bits (%d bytes), k=%d funcoes hash\n",
           num_elementos_esperados, filtro->tamanho_bits, num_bytes, filtro->num_hashes);

    return filtro;
}

/*
   Insere um usuário no filtro.
   A ideia é: calcula k posições diferentes pro usuário e liga (seta pra 1)
   o bit em cada uma dessas posições.
*/
void bloom_inserir(FiltroBloom* filtro, const char* usuario) {
    if (filtro == NULL || usuario == NULL) {
        return;
    }

    /* calculo as duas funções hash uma vez só, antes do laço, já que o
       usuário não muda durante as 7 voltas - não tem motivo pra ficar
       recalculando isso de novo a cada iteração */
    unsigned int hash1 = hash_djb2(usuario);
    unsigned int hash2 = hash_sdbm(usuario);

    /* aqui é o laço que gera as k posições. a fórmula usada é a clássica
       de combinar duas funções hash: pego o hash1 e somo com i vezes o
       hash2. como i muda a cada volta (0, 1, 2...), a posição calculada
       também muda, mesmo usando os mesmos dois hashes de base */
    for (int i = 0; i < filtro->num_hashes; i++) {

        unsigned int combinado = hash1 + (i * hash2);

        /* o % garante que a posição final caia dentro do vetor (entre 0
           e tamanho_bits - 1), senão a gente ia tentar acessar fora do
           vetor e dar problema */
        int posicao = combinado % filtro->tamanho_bits;

        /* agora descubro em qual byte do vetor essa posição cai (cada
           byte tem 8 bits, então divido por 8) e qual bit dentro desse
           byte (o resto da divisão por 8, vai de 0 a 7) */
        int byte_index = posicao / 8;
        int bit_index = posicao % 8;

        /*
           Essa linha aqui é o coração da inserção:
             vetor_bits[byte_index] |= (1 << bit_index);

           O "1 << bit_index" cria um número com só um bit ligado, na
           posição que eu quero. Por exemplo, se bit_index for 3, isso
           dá 00001000.

           O "|=" faz um OR entre o byte que já tá lá e esse número novo,
           e guarda o resultado de volta no mesmo lugar. O OR funciona
           assim: se qualquer um dos dois bits for 1, o resultado é 1.
           Então, ao fazer OR com 00001000, eu garanto que o bit da
           posição 3 fique ligado, sem bagunçar os outros bits que já
           estavam naquele byte (porque OR com 0 não muda nada).
        */
        filtro->vetor_bits[byte_index] |= (1 << bit_index);
    }

    filtro->total_inserido++;
}

/*
   Verifica se um usuário possivelmente está no filtro.
   Calcula as mesmas k posições da inserção e checa se todos os bits
   estão ligados. Se um único bit estiver desligado, já dá pra afirmar
   com certeza que o usuário nunca foi inserido.
*/
int bloom_pertence(FiltroBloom* filtro, const char* usuario) {
    if (filtro == NULL || usuario == NULL) {
        return 0;
    }

    unsigned int hash1 = hash_djb2(usuario);
    unsigned int hash2 = hash_sdbm(usuario);

    for (int i = 0; i < filtro->num_hashes; i++) {
        /* mesma conta da inserção, pra cair exatamente nas mesmas
           posições que foram usadas (ou não) quando esse usuário foi
           cadastrado */
        unsigned int combinado = hash1 + (i * hash2);
        int posicao = combinado % filtro->tamanho_bits;

        int byte_index = posicao / 8;
        int bit_index = posicao % 8;

        /*
           Aqui é o teste do bit:
             (vetor_bits[byte_index] & (1 << bit_index)) != 0

           De novo crio aquela "máscara" com um único bit 1 na posição
           que quero checar. Só que agora, em vez de OR, uso AND (&).
           O AND só dá 1 quando os dois lados são 1 - como a máscara só
           tem um bit ligado, o resultado só vai ser diferente de zero se
           aquele bit específico também estiver ligado no vetor.

           Daí comparo com != 0: se for diferente de zero, o bit estava
           ligado; se for zero, o bit estava desligado.
        */
        int bit_ligado = (filtro->vetor_bits[byte_index] & (1 << bit_index)) != 0;

        /* achei um bit desligado, já posso parar e dizer que não existe,
           não precisa nem checar o resto */
        if (bit_ligado == 0) {
            return 0;
        }
    }

    /* passou pelos k bits e todos estavam ligados, então possivelmente
       o usuário existe (pode ser um falso positivo também) */
    return 1;
}

/*
   Libera tudo que foi alocado com malloc/calloc.
   Primeiro libera o vetor de bits (que tá "dentro" da struct), depois
   a struct em si.
*/
void bloom_destruir(FiltroBloom* filtro) {
    if (filtro == NULL) {
        return;
    }

    free(filtro->vetor_bits);
    free(filtro);
}