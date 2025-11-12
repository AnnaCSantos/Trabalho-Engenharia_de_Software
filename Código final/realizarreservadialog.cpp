#include "realizarreservadialog.h"
#include "ui_realizarreservadialog.h"
#include <QMessageBox>
#include <regex>
#include <QLineEdit>
#include <QLabel>
#include <QComboBox>
#include <QDateEdit>
#include <QSpinBox>

//Função auxiliar
std::shared_ptr<Politica_de_Desconto> criarPoliticaDesconto(int opcao)
{
    switch (opcao)
    {
    case 1: return std::make_shared<Sem_Desconto>();
    case 2: return std::make_shared<Cliente_VIP>();
    case 3: return std::make_shared<Baixa_Temporada>();
    case 4: return std::make_shared<Feriados_Especificos>();
    default: return std::make_shared<Sem_Desconto>();
    }
}

RealizarReservaDialog::RealizarReservaDialog(QWidget *parent, const QString& atendenteUsername)
    : QDialog(parent)
    , ui(new Ui::RealizarReservaDialog),
    gerenciador(Gerenciador_Reservas::getInstance()),
    atendenteLogado(atendenteUsername)
{
    ui->setupUi(this);
    setWindowTitle("Realizar Nova Reserva");

    this->setStyleSheet("QDialog {background-color: #F8F8F8;}");

    popularLocalidades();
    popularTiposQuarto();
    popularPoliticasDesconto();

    ui->checkInDateEdit->setDisplayFormat("dd/MM/yyyy");
    ui->checkInDateEdit->setDate(QDate::currentDate());
    ui->numDiariasSpinBox->setMinimum(1);

    atualizarValorTotalEstimado();  //Atualiza p valor estimado inicial verificando o tipo de desconto
}

RealizarReservaDialog::~RealizarReservaDialog()
{
    delete ui;
}

void RealizarReservaDialog::popularLocalidades()
{
    ui->localidadeComboBox->addItem("Jericoacoara", QVariant::fromValue(Localidade::JERICOACOARA));
    ui->localidadeComboBox->addItem("Canoa Quebrada",QVariant::fromValue(Localidade::CANOA_QUEBRADA));
    ui->localidadeComboBox->addItem("Cumbuco", QVariant::fromValue(Localidade::CUMBUCO));
 }

void RealizarReservaDialog::popularTiposQuarto()
{
    ui->tipoQuartocomboBox->addItem("Solteiro", QVariant::fromValue(TipoQuarto::S));
    ui->tipoQuartocomboBox->addItem("Duplo", QVariant::fromValue(TipoQuarto::D));
    ui->tipoQuartocomboBox->addItem("Casal",QVariant::fromValue(TipoQuarto::C));
    ui->tipoQuartocomboBox->addItem("Triplo", QVariant::fromValue(TipoQuarto::T));
    ui->tipoQuartocomboBox->addItem("Quádruplo", QVariant::fromValue(TipoQuarto::Q));
}

void RealizarReservaDialog::popularPoliticasDesconto()
{
    ui->descontoComboBox->addItem("Sem desconto - 0%", QVariant(1));
    ui->descontoComboBox->addItem("Cliente VIP - 10%", QVariant(2));
    ui->descontoComboBox->addItem("Baixa Temporada - 20%", QVariant(3));
    ui->descontoComboBox->addItem("Promoção feriados - 15%", QVariant(4));
}

double RealizarReservaDialog::calcularPrecoBase()
{
    TipoQuarto tipo = ui->tipoQuartocomboBox->currentData().value<TipoQuarto>();
    int diarias = ui->numDiariasSpinBox->value();
    return gerenciador->preco_Total(tipo, diarias);
}

void RealizarReservaDialog::atualizarValorTotalEstimado()
{
    double precoB = calcularPrecoBase();
    int opcaoD = ui->descontoComboBox->currentData().toInt();
    std::shared_ptr<Politica_de_Desconto> politica = criarPoliticaDesconto(opcaoD);
    double valorComD = politica->aplicar_Desconto(precoB);

    ui->valorEstimadoLabel->setText(QString("Valor Total: R$ %1").arg(valorComD, 0, 'f', 2)); // Adicionado formatação
}

void RealizarReservaDialog::on_localidadeComboBox_currentIndexChanged(int /*index*/)
{
    atualizarValorTotalEstimado();
}

void RealizarReservaDialog::on_tipoQuartocomboBox_currentIndexChanged(int /*index*/)
{
    atualizarValorTotalEstimado();
}

void RealizarReservaDialog::on_numDiariasSpinBox_valueChanged(int /*value*/)
{
    atualizarValorTotalEstimado();
}

void RealizarReservaDialog::on_descontoComboBox_currentIndexChanged(int /*index*/)
{
    atualizarValorTotalEstimado();
}

void RealizarReservaDialog::on_confirmarReservaButton_clicked()
{
    Localidade localidade = ui->localidadeComboBox->currentData().value<Localidade>();
    TipoQuarto tipoQuarto = ui->tipoQuartocomboBox->currentData().value<TipoQuarto>();
    Data checkIn = Data::fromQDate(ui->checkInDateEdit->date());
    int numDiarias = ui->numDiariasSpinBox->value();
    QString nomeCliente = ui->nomeClienteLineEdit->text();
    QString cpfCliente = ui->cpfClienteLineEdit->text();
    int opcaoDesconto = ui->descontoComboBox->currentData().toInt();

    // 2. Validações básicas
    if (nomeCliente.isEmpty() || cpfCliente.isEmpty())
    {
        QMessageBox::warning(this, "Erro de Entrada", "Nome e CPF do cliente são obrigatórios.");
        return;
    }
    if (numDiarias <= 0)
    {
        QMessageBox::warning(this, "Erro de Entrada", "Número de diárias deve ser maior que zero.");
        return;
    }
    if (!cpfCliente.toStdString().empty() && !std::regex_match(cpfCliente.toStdString(), std::regex("[0-9]+")))
    {
        QMessageBox::warning(this, "Erro de Entrada", "CPF deve conter apenas números.");
        return;
    }

    // 3. Verificar disponibilidade
    if (!gerenciador->verifica_Disponibilidade(localidade, tipoQuarto, checkIn, numDiarias))
    {
        QMessageBox::warning(this, "Indisponibilidade", "Quarto não disponível para as datas selecionadas!");
        return;
    }

    // 4. Criar objetos de backend e a reserva
    Cliente cliente(nomeCliente.toStdString(), cpfCliente.toStdString());
    std::shared_ptr<Politica_de_Desconto> politicaDesconto = criarPoliticaDesconto(opcaoDesconto);
    double precoBase = calcularPrecoBase(); // Usando o método corrigido

    Reserva novaReserva(atendenteLogado.toStdString(), cliente, localidade, tipoQuarto,
                        checkIn, numDiarias, politicaDesconto, precoBase);

    // 5. Adicionar a reserva ao gerenciador
    gerenciador->Nova_Reserva(novaReserva);

    // 6. Exibir resumo e sucesso
    QString resumo = "Reserva Realizada com Sucesso!\n\n";
    resumo += QString("ID: #%1\n").arg(novaReserva.getID());
    resumo += QString("Atendente: %1\n").arg(atendenteLogado); // Necessário se 'atendenteLogado' for usado
    resumo += QString("Cliente: %1 (CPF: %2)\n").arg(nomeCliente).arg(cpfCliente);
    resumo += QString("Localidade: %1\n").arg(ui->localidadeComboBox->currentText());
    resumo += QString("Quarto: %1\n").arg(ui->tipoQuartocomboBox->currentText());
    resumo += QString("Check-in: %1\n").arg(ui->checkInDateEdit->date().toString("dd/MM/yyyy"));
    resumo += QString("Diárias: %1\n").arg(numDiarias);
    resumo += QString("Desconto: %1 (%2%)\n").arg(QString::fromStdString(politicaDesconto->getNome())).arg(politicaDesconto->getPercentual());
    resumo += QString("Valor Total: R$ %1\n").arg(novaReserva.getValorTotal(), 0, 'f', 2);
    resumo += QString("Entrada (1/3): R$ %1\n").arg(novaReserva.getValorEntrada(), 0, 'f', 2);
    resumo += QString("Status: %1\n").arg(novaReserva.isConfirmada() ? "Confirmada" : "Pendente");


    QMessageBox::information(this, "Reserva Concluída", resumo);

    accept();
}
