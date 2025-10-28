# Trabalho-Engenharia_de_Software

O projeto está sendo desenvolvido, porém está maior do que o github aceita, por isso não foi possível adicionar aqui, mas foi enviado pelo email pra dar uma olhada (obs: Na minha opinião está lindo e funciona no windows) - Gabi

Aula 14 e 15 - plano de gerenciamento de qualidade de projeto ===============

Papéis e Responsabilidades:

Anna: Responsável pela implementação e manutenção do Banco de Dados (SQLite com Qt SQL). Inclui a implementação de create, update e delete para usuários e a garantia de segurança por meio do hashing nos dados de autenticação.

Bianca: Responsável pelo design e implementação das telas do aplicativo pós-login 

Gabrielle: Responsável pela criação e manutenção das páginas de Login e Principal, incluindo o desenvolvimento do código C++.

Padrões de Documentação:

A documentação do projeto foi dividida entre Documentação de Projeto e Documentação de Código. O repositório GitHub foi a fonte principal das alterações realizadas no projeto. O código foi documentado utilizando comentários que descrevem as classes e funcionalidades, permitindo com que a manutenção e entendimento sejam fluidas e entendíveis por todos os membros. O ambiente de desenvolvimento padrão é o Qt Creator, onde o sistema de build utilizado é o CMake.

Padrões não funcionais:

O padrão não funcional adotado (de segurança) para fazer a autentiação do usuário através do login e assim armazenando os dados dos usuários, é baseado em Hashing Criptográfico. A senha é armazenada por meio de hashing, não em texto puro, diminuindo assim o risco de comprometimento em casos de acesso não autorizado ao banco de dados.
  
Atividade e métricas de garantia:

O processo de Garantia de Qualidade (QA) do projeto se concentra em testes funcionais e de usabilidade. Testes de Integração de Ponta a Ponta são realizados nas funcionalidades críticas de Login e Criação de Conta para validar o fluxo de autenticação e a integridade dos dados no banco.

Aula 16 - arquitetura do projeto ===================================

análise de quisisto do projeto, escolher e justificar um padrão de arq base, conectar proposta com o projeto e especifficar e documentar a proposta

Aula 17 - padrões de projeto =====================================

escolher e justificar os padrões de projeto a serem aplicados, aplicar e documentar  o padrão no código da equipe
