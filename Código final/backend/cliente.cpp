#include "cliente.h"

// Construtor do cliente
Cliente::Cliente(const std::string &n, const std::string &c) : nome(n), CPF(c) {}

// Implementação do método getNome
std::string Cliente::getNome() const
{
    return nome;
}

// Implementação do método getCPF
std::string Cliente::getCPF() const
{
    return CPF;
}
