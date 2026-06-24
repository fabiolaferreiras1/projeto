#   Sistema de Consulta com Tabela Hash e Filtro de Bloom

## 1. SOBRE O PROJETO

Este projeto foi desenvolvido como avaliação para a disciplina de **Laboratório de Algoritmos e Estruturas de Dados II**. O objetivo principal é construir um sistema de alta performance para a verificação de cadastro de usuários, mitigando acessos desnecessários à estrutura principal de armazenamento através do uso de uma estrutura probabilística complementar.

O sistema faz uso combinado de duas estruturas de dados implementadas manualmente:
1. **Filtro de Bloom:** Uma estrutura probabilística baseada em um vetor de bits alocado dinamicamente para acelerar consultas de existência.
2. **Tabela Hash:** Estrutura principal de armazenamento utilizada para a recuperação exata dos dados dos usuários.

---

## 2. INSTRUÇÕES DE COMPILAÇÃO

O projeto foi estruturado com os códigos-fonte na pasta `src/` e os arquivos gerados/testados na pasta `data/`. 

Para compilar o sistema, abra o terminal na pasta raiz do projeto e execute o comando abaixo utilizando o compilador GCC. 
*Atenção: A flag `-lm` é obrigatória para linkar a biblioteca matemática (`<math.h>`) utilizada para o dimensionamento dinâmico do Filtro de Bloom.*

```bash
gcc -Wall -Wextra -o sistema_usuarios src/main.c src/hash.c src/bloom.c -lm
```
---

## 3. FORMATO DE ENTRADA

O sistema permite inserir usuários individualmente através do menu interativo ou em lote carregando registros a partir de um arquivo de texto.

Para a massa de dados, os identificadores gerados e aceitos pelo sistema seguem o formato exato de 11 caracteres composto por:

* `[8caractere]` (letras minúsculas) seguidos de `[3numeros]`.

**Exemplos de registros válidos:**

* `islaifda122`
* `djskalsa297`
* `fjkldsaf881`

Para a importação em lote, o arquivo `.txt` (armazenado na pasta `data/`) deve conter um identificador por linha.

---

## 4. EXEMPLOS DE EXECUÇÃO

Após a compilação, o programa deve ser executado a partir da pasta raiz para que os caminhos relativos da pasta `data/` funcionem corretamente.

**Iniciando o programa no Linux / macOS:**

```bash
./sistema_usuarios
```

**Iniciando o programa no Windows:**

```cmd
sistema_usuarios.exe
```

**Exemplo de Fluxo de Execução no Menu:**

```text
=== MENU DE VERIFICAÇÃO DE USUÁRIOS ===
1 - (RF01) Inserir usuário
2 - (RF02) Consultar usuário
3 - (RF03) Exibir estatísticas
4 - (RF04) Inserir em lote (por meio de arquivo .txt)
5 - Gerar massa de dados (para testes)
6 - Executar experimentos
0 - Sair
Escolha uma opção: 4
Digite o nome do arquivo (ex: usuarios.txt): 1000.txt
1000 registros inseridos em lote a partir de 'data/1000.txt'.

Escolha uma opção: 2
Digite o nome do usuário para consulta: islaifda122
-> Usuário encontrado na Tabela Hash!

Escolha uma opção: 2
Digite o nome do usuário para consulta: djskalsa297
-> Usuário inexistente (bloqueado pelo Filtro de Bloom)
```
## 5. CONCLUSÃO

Este projeto demonstra na prática o impacto positivo do uso combinado do Filtro de Bloom e da Tabela Hash. Enquanto a Tabela Hash garante o armazenamento e a recuperação exata dos elementos , o Filtro de Bloom atua como uma barreira inicial altamente eficiente, evitando acessos desnecessários à memória principal em casos de usuários inexistentes.  

Através da execução da bateria de experimentos, o sistema permitiu avaliar quantitativamente o desempenho, o consumo de memória e a taxa de falsos positivos. Dessa forma, foi possível justificar as decisões de projeto, tais quais o tamanho da tabela, o fator de carga e a quantidade de funções hash, embasadas em análises experimentais concretas.