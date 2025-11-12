#include "reserva.h"

// Inicialização do contador estático
int Reserva::prox_ID = 0;

// Construtor da reserva
Reserva::Reserva(const std::string& a, const Cliente& c, Localidade l, TipoQuarto tipo,
                 const Data& data, int diarias, std::shared_ptr<Politica_de_Desconto> d, double valor)
    : ID(++prox_ID), atendente(a), cliente(c), localidade(l), tipo_quarto(tipo),
    check_In(data), num_Diarias(diarias), desconto(d), confirmada(false)
{
    valor_Total = desconto->aplicar_Desconto(valor); // Aplica desconto ao valor
    valor_Entrada = valor_Total / 3;                 // 1/3 do valor total
}

// Implementação dos métodos getters
int Reserva::getID() const
{
    return ID;
}

std::string Reserva::getAtendente() const
{
    return atendente;
}

Cliente Reserva::getCliente() const
{
    return cliente;
}

Localidade Reserva::getLocalidade() const
{
    return localidade;
}

TipoQuarto Reserva::getTipoQuarto() const
{
    return tipo_quarto;
}

Data Reserva::getCheckIn() const
{
    return check_In;
}

int Reserva::getNumDiarias() const
{
    return num_Diarias;
}

double Reserva::getValorTotal() const
{
    return valor_Total;
}

double Reserva::getValorEntrada() const
{
    return valor_Entrada;
}

bool Reserva::isConfirmada() const
{
    return confirmada;
}

std::string Reserva::getNomePolitica() const
{
    return desconto->getNome();
}

double Reserva::getPercentualDesconto() const
{
    return desconto->getPercentual();
}

// Confirma a reserva
void Reserva::confirmar()
{
    confirmada = true;
}

bool Reserva::isPendente() const
{
    return !confirmada;
}

// Operador para ordenação por data de check-in
bool Reserva::operator<(const Reserva& other) const
{
    return check_In < other.check_In;
}
