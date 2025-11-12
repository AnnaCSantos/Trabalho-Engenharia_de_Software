#include "data.h"

// Construtor
Data::Data(int d, int m, int a) : dia(d), mes(m), ano(a) {}

// Implementação do método fromString
Data Data::fromString(const std::string& str)
{
    Data data_obj;
    char um, dois; // Variáveis dummy para consumir as barras
    std::istringstream iss(str);
    iss >> data_obj.dia >> um >> data_obj.mes >> dois >> data_obj.ano;
    return data_obj;
}

// Implementação do método toString
std::string Data::toString() const
{
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(2) << dia << "/"
       << std::setw(2) << mes << "/" << ano;
    return ss.str();
}

// Converte QDate para Data
Data Data::fromQDate(const QDate& qdate) {
    return Data(qdate.day(), qdate.month(), qdate.year());
}

// Converte Data para QDate
QDate Data::toQDate() const {
    return QDate(ano, mes, dia);
}

// Implementação do operador <
bool Data::operator<(const Data& other) const
{
    if (ano != other.ano)
        return ano < other.ano;
    if (mes != other.mes)
        return mes < other.mes;
    return dia < other.dia;
}

// Implementação do operador ==
bool Data::operator==(const Data& other) const
{
    return dia == other.dia && mes == other.mes && ano == other.ano;
}

// Implementação do método avancarDias (agora usando QDate para robustez)
Data Data::avancarDias(int dias) const {
    QDate qdate = this->toQDate();
    qdate = qdate.addDays(dias);
    return Data::fromQDate(qdate);
}

// Implementação do método intervaloSobreposto
bool Data::intervaloSobreposto(const Data& inicio1, int duracao1, const Data& inicio2, int duracao2)
{
    Data fim1 = inicio1.avancarDias(duracao1);
    Data fim2 = inicio2.avancarDias(duracao2);

    return !(fim1 < inicio2 || fim2 < inicio1);
}
