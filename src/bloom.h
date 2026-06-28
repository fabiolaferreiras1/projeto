#ifndef BLOOM_H
#define BLOOM_H

/*
   A ideia do Filtro de Bloom é guardar só "bitzinhos" ligados (1) ou
   desligados (0) num vetor, em vez de guardar o texto inteiro de cada
   usuário. Pra inserir ou consultar um usuário, a gente calcula k
   posições diferentes nesse vetor (usando funções hash) e liga ou
   verifica os bits dessas posições.

   Se em uma consulta algum desses bits estiver desligado, é certeza
   absoluta que o usuário nunca foi inserido. Mas se todos os bits
   estiverem ligados, não dá pra ter 100% de certeza - pode ser que o
   usuário realmente exista, ou pode ser só uma coincidência de bits
   (o tal "falso positivo"). Por isso, depois de passar pelo Bloom, a
   gente ainda confirma na Tabela Hash de verdade.

   Diferente da versão anterior, aqui o tamanho do vetor (m) e a
   quantidade de funções hash (k) não são mais fixos: eles são
   calculados matematicamente, em função de quantos elementos a gente
   espera guardar e da taxa de falso positivo que a gente aceita ter.
   Essas fórmulas são as que aparecem nos livros e artigos sobre Filtro
   de Bloom, então dão pra justificar com base teórica na apresentação.
*/

/*
   Struct principal do filtro.

   vetor_bits     -> onde os bits ficam guardados (em bytes, 8 bits por byte)
   tamanho_bits   -> tamanho total do vetor em bits (esse é o "m" da fórmula)
   num_hashes     -> quantas funções hash a gente aplica em cada usuário
                      (esse é o "k" da fórmula)
   total_inserido -> contador de quantos usuários já foram inseridos,
                      só pra ajudar nas estatísticas do relatório depois
*/
typedef struct {
    unsigned char* vetor_bits;
    int tamanho_bits;
    int num_hashes;
    int total_inserido;
} FiltroBloom;

/*
   Cria o filtro já calculando m e k pelas fórmulas matemáticas oficiais,
   usando o número de elementos esperados e a taxa de falso positivo
   desejada. Olha em bloom.c pra ver a conta completa e os comentários
   explicando cada parte.
*/
FiltroBloom* bloom_criar(int num_elementos_esperados, double taxa_falso_positivo_desejada);

/* Insere um usuário no filtro (liga os k bits dele) */
void bloom_inserir(FiltroBloom* filtro, const char* usuario);

/*
   Verifica se o usuário possivelmente está no filtro.
   Retorna 1 se talvez exista (todos os k bits ligados).
   Retorna 0 se com certeza não existe (algum bit desligado).
*/
int bloom_pertence(FiltroBloom* filtro, const char* usuario);

/* Libera toda a memória usada pelo filtro */
void bloom_destruir(FiltroBloom* filtro);

#endif