#include "listarreservadialog.h"
#include "ui_listarreservadialog.h"
#include <QMessageBox>
#include <QDebug>
#include <QTableWidgetItem>
#include <QBrush>

ListarReservaDialog::ListarReservaDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ListarReservaDialog),
    gerenciador(Gerenciador_Reservas::getInstance())
{
    ui->setupUi(this);
    setWindowTitle("Listagem de Reservas");

    ui->reservasTableWidget->setColumnCount(12);
    QStringList headers;
    headers <<"ID"<<"Atendente"<<"Cliente"<<"CPF"<< "Localidade"<< "Quarto" << "Check_In"<< "Diárias"<<"Desconto"<<"Total (R$)"<<"Entrada(R$"<<"Status";
    ui->reservasTableWidget->setHorizontalHeaderLabels(headers);
    ui->reservasTableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->reservasTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->reservasTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    carregaReservasNaTabela();

}

ListarReservaDialog::~ListarReservaDialog()
{
    delete ui;
}

void ListarReservaDialog::on_confirmarSelecionadaButton_clicked()
{
    // Obtém as linhas selecionadas na tabela.
    QModelIndexList selectedRows = ui->reservasTableWidget->selectionModel()->selectedRows();
    if (selectedRows.empty())
    {
        QMessageBox::warning(this, "Nenhuma Seleção", "Por favor, selecione uma reserva para confirmar.");
        return; // Sai se nenhuma linha estiver selecionada.
    }

    // Pega o ID da primeira linha selecionada (assumindo que o ID está na primeira coluna, índice 0).
    int selectedRow = selectedRows.first().row();
    int reservaId = ui->reservasTableWidget->item(selectedRow, 0)->text().toInt();

    // Tenta confirmar a reserva no backend.
    if (gerenciador->confirma_Reserva(reservaId))
    {
        QMessageBox::information(this, "Confirmação", QString("Reserva #%1 confirmada com sucesso!").arg(reservaId));
        carregaReservasNaTabela(); // Recarrega a tabela para que o status visual seja atualizado.
    }
    else
    {
        // Se a confirmação falhou, verifica se a reserva já estava confirmada para dar um feedback mais específico.
        Reserva* reserva = gerenciador->buscarReservaPorId(reservaId);
        if (reserva && reserva->isConfirmada())
        {
            QMessageBox::warning(this, "Confirmação", QString("Reserva #%1 já está confirmada.").arg(reservaId));
        }
        else
        {
            QMessageBox::critical(this, "Erro", QString("Não foi possível confirmar a reserva #%1. Não encontrada ou erro inesperado.").arg(reservaId));
        }
    }
}

void ListarReservaDialog::on_refreshButton_clicked()
{
    carregaReservasNaTabela();
    QMessageBox::information(this, "Atualização", "Lista de reservas atualizada.");
}

void ListarReservaDialog::carregaReservasNaTabela()
{
    ui->reservasTableWidget->setRowCount(0);
    const auto& reservas = gerenciador->getTodasReservas();

    ui->reservasTableWidget->setRowCount(reservas.size());
    int row = 0;

    for(const auto& reserva : reservas)
    {
        ui->reservasTableWidget->setItem(row, 0, new QTableWidgetItem(QString::number(reserva.getID())));
        ui->reservasTableWidget->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(reserva.getAtendente())));
        ui->reservasTableWidget->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(reserva.getCliente().getNome())));
        ui->reservasTableWidget->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(reserva.getCliente().getCPF())));
        ui->reservasTableWidget->setItem(row, 4, new QTableWidgetItem(localidadeToString(reserva.getLocalidade()))); // Converte enum para string
        ui->reservasTableWidget->setItem(row, 5, new QTableWidgetItem(tipoQuartoToString(reserva.getTipoQuarto()))); // Converte enum para string
        ui->reservasTableWidget->setItem(row, 6, new QTableWidgetItem(QString::fromStdString(reserva.getCheckIn().toString())));
        ui->reservasTableWidget->setItem(row, 7, new QTableWidgetItem(QString::number(reserva.getNumDiarias())));
        ui->reservasTableWidget->setItem(row, 8, new QTableWidgetItem(QString("%1 (%2%)").arg(QString::fromStdString(reserva.getNomePolitica())).arg(reserva.getPercentualDesconto(), 0, 'f', 0)));
        ui->reservasTableWidget->setItem(row, 9, new QTableWidgetItem(QString::number(reserva.getValorTotal(), 'f', 2)));
        ui->reservasTableWidget->setItem(row, 10, new QTableWidgetItem(QString::number(reserva.getValorEntrada(), 'f', 2)));
        QString status = reserva.isConfirmada() ? "Confirmada" : "Pendente";
        QTableWidgetItem* statusItem = new QTableWidgetItem(status);
        if (reserva.isConfirmada())
        {
            statusItem->setForeground(QBrush(Qt::darkGreen)); // Cor verde escura para status Confirmada
        }
        else
        {
            statusItem->setForeground(QBrush(Qt::darkRed)); // Cor vermelha escura para status Pendente
        }
        ui->reservasTableWidget->setItem(row, 11, statusItem);

        row++; // Move para a próxima linha
    }

    // Ajusta a largura das colunas para se adequar ao conteúdo
    ui->reservasTableWidget->resizeColumnsToContents();
}

QString ListarReservaDialog::tipoQuartoToString(TipoQuarto tipo) const
{
    switch(tipo)
    {
        case TipoQuarto::S: return "Solteiro";
        case TipoQuarto::D: return "Duplo";
        case TipoQuarto::C: return "Casal";
        case TipoQuarto::T: return "Triplo";
        case TipoQuarto::Q: return "Quádruplo";
        default: return "Desconhecido";
    }
}

QString ListarReservaDialog::localidadeToString(Localidade local) const
{
    switch (local)
    {
        case Localidade::JERICOACOARA: return "Jericoacoara";
        case Localidade::CANOA_QUEBRADA: return "Canoa_Quebrada";
        case Localidade::CUMBUCO: return "Cumbuco";
        default: return "Desconhecida";
    }
}
