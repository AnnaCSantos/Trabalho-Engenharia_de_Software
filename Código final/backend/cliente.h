#ifndef CLIENTE_H
#define CLIENTE_H

#include <string>

class Cliente
{
    private:
        std::string nome; // nome do cliente
        std::string CPF;  // CPF do cliente

    public:
        Cliente(const std::string &n, const std::string &c);
        std::string getNome() const;
        std::string getCPF() const;
};

#endif
