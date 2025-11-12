#ifndef ATENDENTE_H
#define ATENDENTE_H

#include <string>

class Atendente
{
    private:
        std::string usuario; // Nome do Atendente
        std::string senha;   // Senha do atendente

    public:
        Atendente(const std::string& u, const std::string& s);
        std::string getUsuario() const;
        std::string getSenha() const;
        bool validarSenha(const std::string& s) const;
};

#endif
