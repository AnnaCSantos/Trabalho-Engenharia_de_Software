#ifndef DATA_H
#define DATA_H

#include <string>
#include <sstream>
#include <iomanip>
#include <QDate>

class Data
{
public:
    int dia;
    int mes;
    int ano;

    Data(int d = 0, int m = 0, int a = 0);

    static Data fromString(const std::string& str);         // Converte string para Data
    std::string toString() const;                           // Converte Data para string
    static Data fromQDate(const QDate& qdate);              // Adicionado para integração com Qt
    QDate toQDate() const;                                  // Adicionado para integração com Qt

    // Operadores de comparação
    bool operator<(const Data& other) const;
    bool operator==(const Data& other) const;

    Data avancarDias(int dias) const;       // Soma dias
    static bool intervaloSobreposto(const Data& inicio1, int duracao1, const Data& inicio2, int duracao2);      // Verifica sobreposição de intervalo
};

#endif
