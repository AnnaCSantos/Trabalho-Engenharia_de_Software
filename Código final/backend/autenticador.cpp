#include "autenticador.h"

// Construtor que cadastra atendentes padrão do sistema
Autenticador::Autenticador()
{
    atendentes.push_back(Atendente("atendente1", "senha1"));
    atendentes.push_back(Atendente("atendente2", "senha2"));
    atendentes.push_back(Atendente("atendente3", "senha3"));
    atendentes.push_back(Atendente("atendente4", "senha4"));
}

// Implementação do método autenticar
bool Autenticador::autenticar(const std::string &usuario, const std::string &senha) const
{
    for(const auto& atendente: atendentes)
    {
        if(atendente.getUsuario() == usuario && atendente.getSenha() == senha)
        {
            return true; // Credenciais válidas
        }
    }
    return false; // Credenciais inválidas
}
