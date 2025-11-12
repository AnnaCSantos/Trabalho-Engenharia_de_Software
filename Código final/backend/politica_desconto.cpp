#include "politica_desconto.h"


// Implementação da política Sem_Desconto
double Sem_Desconto::aplicar_Desconto(double valor) const
{
    return valor; // Retorna o valor original
}

std::string Sem_Desconto::getNome() const
{
    return "Sem desconto"; // Nome da política
}

double Sem_Desconto::getPercentual() const
{
    return 0.0; // Percentual de desconto
}

// Implementação da política Cliente_VIP
double Cliente_VIP::aplicar_Desconto(double valor) const
{
    return valor * 0.9; // Aplica 10% de desconto
}

std::string Cliente_VIP::getNome() const
{
    return "Cliente VIP"; // Nome da política
}

double Cliente_VIP::getPercentual() const
{
    return 10.0; // Percentual de desconto
}

// Implementação da política Baixa_Temporada
double Baixa_Temporada::aplicar_Desconto(double valor) const
{
    return valor * 0.8; // Aplica 20% de desconto
}

std::string Baixa_Temporada::getNome() const
{
    return "Baixa Temporada"; // Nome da política
}

double Baixa_Temporada::getPercentual() const
{
    return 20.0; // Percentual de desconto
}

// Implementação da política Feriados_Especificos
double Feriados_Especificos::aplicar_Desconto(double valor) const
{
    return valor * 0.85; // Aplica 15% de desconto
}

std::string Feriados_Especificos::getNome() const
{
    return "Promocao de Feriados"; // Nome da política
}

double Feriados_Especificos::getPercentual() const
{
    return 15.0; // Percentual de desconto
}
