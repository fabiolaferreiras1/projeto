#ifndef HASH_H
#define HASH_H

/*
   Tabela Hash com Encadeamento Externo - parte do Integrante 1

   A ideia aqui é criar uma estrutura de armazenamento rápida e segura.
   Diferente do Filtro de Bloom, a Tabela Hash guarda o nome real e completo 
   de cada usuário cadastrado. Ela funciona como um armário cheio de gavetas: 
   a gente passa o nome do usuário em uma função matemática (a função hash), 
   ela nos dá o número de uma gaveta, e a gente guarda o usuário lá dentro.

   Se dois usuários diferentes caírem na mesma gaveta, acontece o que chamamos 
   de "colisão". Para resolver isso, usamos o Encadeamento Externo: cada gaveta 
   é, na verdade, o início de uma lista encadeada. Os usuários que colidirem 
   vão sendo "pendurados" um atrás do outro nessa lista de forma organizada.
*/

/*
   Struct do Nó da lista encadeada.
   
   usuario -> guarda a string real do usuário (padrão de 11 caracteres + '\0')
   proximo -> ponteiro para o próximo nó da lista caso ocorra uma colisão
*/
typedef struct No {
    char usuario[12];     
    struct No* proximo;   
} No;

/* Struct principal da Tabela Hash.

   tabela     -> o vetor de ponteiros para Nós (as gavetas onde as listas começam)
   tamanho    -> o tamanho total do vetor (o "M" da nossa fórmula, definido dinamicamente)
   quantidade -> conta quantos usuários estão guardados (o "N" usado para calcular o fator de carga)
*/
typedef struct {
    No** tabela;          
    int tamanho;          
    int quantidade;       
} TabelaHash;

/* Cria a tabela alocando a estrutura principal.
   MODIFICAÇÃO: Agora recebe o 'tamanho' desejado como parâmetro para se adaptar 
   dinamicamente aos diferentes volumes de testes (1k, 10k, 100k).
*/
TabelaHash* hash_criar(int tamanho);  

/* A função cérebro do módulo. Transforma o texto do usuário em um índice 
   numérico válido dentro dos limites do nosso vetor usando o algoritmo DJB2.
*/
unsigned int hash_funcao(const char* str, int tamanho_tabela);  

/* Insere um novo usuário na tabela. Calcula o índice e coloca o elemento 
   sempre no início da lista encadeada daquela posição, garantindo velocidade O(1).
*/
int hash_inserir(TabelaHash* h, const char* usuario);  

/* Busca por um usuário. Vai direto na gaveta certa através do índice e 
   percorre apenas a pequena lista daquela posição usando strcmp. É o nosso 
   "segundo filtro" para checar falsos positivos do Bloom.
*/
int hash_buscar(TabelaHash* h, const char* usuario);  

/* Varre o vetor limpando nó por nó de todas as listas da memória com 'free' 
   para evitar qualquer vazamento de memória (memory leak).
*/
void hash_destruir(TabelaHash* h);  

#endif
