// backend/gerenciador_reservas.h
#ifndef GERENCIADOR_RESERVAS_H
#define GERENCIADOR_RESERVAS_H

#include <vector>    // Para std::vector
#include <map>       // Para std::map
#include <string>    // Para std::string
#include <algorithm> // Para std::sort
#include <fstream>
#include <iomanip>
#include <sstream>

#include "reserva.h"
#include "tipo_hotel.h"
#include "data.h"

class Gerenciador_Reservas
{
    private:
        static Gerenciador_Reservas* instancia;               // Instância única (Singleton)
        std::vector<Reserva> reservas;                        // Lista de todas as reservas
        std::map<TipoQuarto, double> precos;                  // Tabela de preços por tipo de quarto
        Gerenciador_Reservas();                               // Construtor privado para implementar Singleton

        std::string tipoQuartoString(TipoQuarto tipo) const;   // Converte enum TipoQuarto para string
        std::string localidadeString(Localidade l) const;      // Converte enum Localidade para string

        public:
            static Gerenciador_Reservas* getInstance();         // Método para obter a instância única (Singleton)

            // Impede cópia e atribuição
            Gerenciador_Reservas(Gerenciador_Reservas const&) = delete;
            void operator=(Gerenciador_Reservas const&) = delete;
            bool verifica_Disponibilidade(Localidade l, TipoQuarto t, const Data& check, int d) const;  // Verifica se um quarto está disponível para as datas especificadas
            void Nova_Reserva(const Reserva& reserva);       // Adiciona uma nova reserva ao sistema
            bool confirma_Reserva(int id);                   // Confirma uma reserva pelo ID somente se estiver pendente
            double preco_Total(TipoQuarto t, int d) const;   // Calcula preço total baseado no tipo de quarto e Numero de diárias
            void gerar_Relatorio(const std::string& nome_Arq) const;  // Gera relatório completo das reservas em arquivo texto

            const std::vector<Reserva>& getTodasReservas() const { return reservas; }       // Método para a GUI obter a lista de reservas

            //Método para buscar uma reserva específica pelo ID
            Reserva* buscarReservaPorId(int id);
};

#endif
