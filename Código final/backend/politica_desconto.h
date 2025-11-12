#ifndef POLITICA_DESCONTO_H
#define POLITICA_DESCONTO_H

#include <string>

class Politica_de_Desconto
{
    public:
        virtual double aplicar_Desconto(double valor) const = 0;    // Aplica o desconto ao valor
        virtual std::string getNome() const = 0;                    // Retorna o nome da política de desconto
        virtual double getPercentual() const = 0;                   // Retorna o percentual de desconto
        virtual ~Politica_de_Desconto() = default;                  // Destrutor padrão
};

// Implementação da política de desconto sem desconto
class Sem_Desconto : public Politica_de_Desconto
{
    public:
        double aplicar_Desconto(double valor) const override;
        std::string getNome() const override;
        double getPercentual() const override;
};

// Implementação da política de desconto para clientes VIP (10% de desconto)
class Cliente_VIP : public Politica_de_Desconto
{
    public:
        double aplicar_Desconto(double valor) const override;
            std::string getNome() const override;
            double getPercentual() const override;
};

// Implementação da política de desconto para baixa temporada (20% de desconto).
class Baixa_Temporada : public Politica_de_Desconto
{
    public:
        double aplicar_Desconto(double valor) const override;
            std::string getNome() const override;
        double getPercentual() const override;
};

// Implementação da política de desconto para feriados específicos (15% de desconto)
class Feriados_Especificos : public Politica_de_Desconto
{
    public:
        double aplicar_Desconto(double valor) const override;
        std::string getNome() const override;
        double getPercentual() const override;
};

#endif
