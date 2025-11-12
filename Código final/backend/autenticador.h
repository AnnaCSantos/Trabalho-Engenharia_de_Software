#ifndef AUTENTICADOR_H
#define AUTENTICADOR_H

#include <vector>
#include "atendente.h"

class Autenticador
{
    private:
        std::vector<Atendente> atendentes;  // Lista de atendentes cadastrados


    public:
        Autenticador();     //Cadastra atendentes padrão do sistema
        bool autenticar(const std::string& usuario, const std::string& senha) const;  //Verifica se as credenciais fornecidas são válidas
};

#endif
