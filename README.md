#   Sistema de Consulta com Tabela Hash e Filtro de Bloom
# 📌 Sobre o Projeto
Este projeto foi desenvolvido como avaliação para a disciplina de **Laboratório de Algoritmos e Estruturas de Dados II**. O objetivo principal é construir um sistema de alta performance para a verificação de cadastro de usuários, mitigando acessos desnecessários à estrutura principal de armazenamento através do uso de uma estrutura probabilística complementar.

O sistema faz uso combinado de duas estruturas de dados implementadas manualmente:
1. **Filtro de Bloom:** Uma estrutura probabilística baseada em um vetor de bits alocado dinamicamente para acelerar consultas de existência.
2. **Tabela Hash:** Estrutura principal de armazenamento utilizada para a recuperação exata dos dados dos usuários.
