#include "atendente.h"

Atendente::Atendente(const std::string &u, const std::string &s): usuario(u), senha(s){}

std::string Atendente::getUsuario() const
{
    return usuario;
}

std::string Atendente::getSenha() const
{
    return senha;
}

bool Atendente::validarSenha(const std::string &s) const
{
    return senha == s;
}
