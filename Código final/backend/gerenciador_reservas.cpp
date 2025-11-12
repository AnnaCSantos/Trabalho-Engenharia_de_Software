#include "gerenciador_reservas.h"

// Inicialização da instância estática do Singleton
Gerenciador_Reservas* Gerenciador_Reservas::instancia = nullptr;

// Construtor privado para implementar Singleton
Gerenciador_Reservas::Gerenciador_Reservas()
{
    // Inicializa tabela de preços por tipo de quarto
    precos[TipoQuarto::S] = 200.0;
    precos[TipoQuarto::D] = 300.0;
    precos[TipoQuarto::C] = 350.0;
    precos[TipoQuarto::T] = 450.0;
    precos[TipoQuarto::Q] = 550.0;
}

// Método para obter a instância única (Singleton)
Gerenciador_Reservas* Gerenciador_Reservas::getInstance()
{
    if (instancia == nullptr)
    {
        instancia = new Gerenciador_Reservas();
    }
    return instancia;
}

// Converte enum TipoQuarto para string
std::string Gerenciador_Reservas::tipoQuartoString(TipoQuarto tipo) const
{
    switch (tipo)
    {
        case TipoQuarto::S: return "Solteiro";
        case TipoQuarto::D: return "Duplo";
        case TipoQuarto::C: return "Casal";
        case TipoQuarto::T: return "Triplo";
        case TipoQuarto::Q: return "Quádruplo";
        default: return "Desconhecido";
    }
}

// Converte enum Localidade para string
std::string Gerenciador_Reservas::localidadeString(Localidade l) const
{
    switch (l)
    {
        case Localidade::JERICOACOARA: return "Jericoacoara";
        case Localidade::CANOA_QUEBRADA: return "Canoa Quebrada";
        case Localidade::CUMBUCO: return "Cumbuco";
        default: return "Desconhecida";
    }
}

// Verifica se um quarto está disponível para as datas especificadas
bool Gerenciador_Reservas::verifica_Disponibilidade(Localidade l, TipoQuarto t, const Data& check, int d) const
{
    for (const auto& reserva : reservas)
    {
        // Verifica conflitos com reservas já confirmadas
        if (reserva.getLocalidade() == l && reserva.getTipoQuarto() == t && reserva.isConfirmada())
        {
            Data checkExistente = reserva.getCheckIn();
            int duracaoExistente = reserva.getNumDiarias();

            if (Data::intervaloSobreposto(check, d, checkExistente, duracaoExistente))
            {
                return false; // Conflito encontrado na data
            }
        }
    }
    return true; // Disponível
}

// Adiciona uma nova reserva ao sistema
void Gerenciador_Reservas::Nova_Reserva(const Reserva& reserva)
{
    reservas.push_back(reserva);
}

// Confirma uma reserva pelo ID somente se estiver pendente
bool Gerenciador_Reservas::confirma_Reserva(int id)
{
    for (auto& reserva : reservas)
    {
        if (reserva.getID() == id)
        {
            if (reserva.isPendente())
            {
                reserva.confirmar(); // Confirma a reserva
                return true;
            }
            else {
                // Não imprime aqui, a GUI fará isso
                return false; // Reserva já está confirmada.
            }
        }
    }
    // Não imprime aqui, a GUI fará isso
    return false; // Reserva não encontrada
}

// Calcula preço total baseado no tipo de quarto e Numero de diárias
double Gerenciador_Reservas::preco_Total(TipoQuarto t, int d) const
{
    auto it = precos.find(t);
    if (it != precos.end())
    {
        return it->second * d; // Retorna o preço total
    }
    return 0.0; // Tipo de quarto desconhecido
}

// Gera relatório completo das reservas em arquivo texto
void Gerenciador_Reservas::gerar_Relatorio(const std::string& nome_Arq) const
{
    std::vector<Reserva> r_Ordenadas = reservas;
    std::sort(r_Ordenadas.begin(), r_Ordenadas.end());

    std::ofstream arquivo(nome_Arq);
    if (arquivo.is_open())
    {
        arquivo << "---------------------------------------Relatorio de Reservas - Rede Paradise----------------------------------------\n\n";
        for (const auto& reserva : r_Ordenadas)
        {
            arquivo << "\tID: " << reserva.getID() << std::endl;
            arquivo << "\tAtendente: " << reserva.getAtendente() << std::endl;
            arquivo << "\tCliente: " << reserva.getCliente().getNome()
                    << "\tCPF: " << reserva.getCliente().getCPF() << std::endl;
            arquivo << "\tLocalidade: " << localidadeString(reserva.getLocalidade()) << std::endl;
            arquivo << "\tQuarto: " << tipoQuartoString(reserva.getTipoQuarto()) << std::endl;
            arquivo << "\tCheck-In: " << reserva.getCheckIn().toString() << std::endl;
            arquivo << "\tDiarias: " << reserva.getNumDiarias() << std::endl;
            arquivo << "\tDesconto: " << reserva.getNomePolitica();

            if (reserva.getPercentualDesconto() > 0)
            {
                arquivo << " (" << reserva.getPercentualDesconto() << "%)";
            }
            arquivo << std::endl;

            arquivo << "\tValor Total: R$ " << std::fixed << std::setprecision(2) << reserva.getValorTotal() << std::endl;
            arquivo << "\tEntrada: R$ " << reserva.getValorEntrada() << std::endl;
            arquivo << "\tStatus: " << (reserva.isConfirmada() ? "Confirmada" : "Pendente") << std::endl;
            arquivo << "---------------------------------------------------------------------------------------------------------------------" << std::endl;
        }
        arquivo.close();
    }
    else
    {
        // A GUI vai lidar com a mensagem de erro
    }
}

// Implementação do método buscarReservaPorId
Reserva* Gerenciador_Reservas::buscarReservaPorId(int id)
{
    for (auto& reserva : reservas)
    {
        if (reserva.getID() == id)
        {
            return &reserva;
        }
    }
    return nullptr;
}
