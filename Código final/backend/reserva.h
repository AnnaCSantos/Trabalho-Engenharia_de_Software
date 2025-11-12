// backend/reserva.h
#ifndef RESERVA_H
#define RESERVA_H

#include <string>
#include <memory>

#include "cliente.h"
#include "data.h"
#include "politica_desconto.h"
#include "tipo_hotel.h"

class Reserva
{
    private:
        static int prox_ID;     // Contador estático para IDs únicos
        int ID;                 // ID da reserva
        std::string atendente;  // Nome do atendente responsável
        Cliente cliente;        // Dados do cliente
        Localidade localidade;  // Local do hotel
        TipoQuarto tipo_quarto; // Tipo de quarto reservado
        Data check_In;          // Data de check-in
        int num_Diarias;        // Numero de diárias
        std::shared_ptr<Politica_de_Desconto> desconto; // Política de desconto aplicada
        double valor_Total;     // Valor total com desconto
        double valor_Entrada;   // Valor da entrada (1/3 do total)
        bool confirmada;        // Status da confirmação

    public:
        Reserva(const std::string& a, const Cliente& c, Localidade l, TipoQuarto tipo,
                const Data& data, int diarias, std::shared_ptr<Politica_de_Desconto> d, double valor);

        int getID() const;
        std::string getAtendente() const;
        Cliente getCliente() const;
        Localidade getLocalidade() const;
        TipoQuarto getTipoQuarto() const;
        Data getCheckIn() const;
        int getNumDiarias() const;
        double getValorTotal() const;
        double getValorEntrada() const;
        bool isConfirmada() const;
        std::string getNomePolitica() const;
        double getPercentualDesconto() const;
        void confirmar();
        bool isPendente() const;
        bool operator<(const Reserva& other) const;
};

#endif
