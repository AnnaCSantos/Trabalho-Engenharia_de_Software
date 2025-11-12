#ifndef LISTARRESERVADIALOG_H
#define LISTARRESERVADIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QHeaderView>
#include <QObject>

//Os backend
#include "backend/gerenciador_reservas.h"
#include "backend/reserva.h"
#include "backend/tipo_hotel.h"

namespace Ui {
class ListarReservaDialog;
}

class ListarReservaDialog : public QDialog
{
    Q_OBJECT

    public:
        explicit ListarReservaDialog(QWidget *parent = nullptr);
        ~ListarReservaDialog();

    private slots:
        void on_confirmarSelecionadaButton_clicked();   //Esse botão serve para confirmar uma reserva que está como pendente
        void on_refreshButton_clicked();                //Atualizar a lista

    private:
        Ui::ListarReservaDialog *ui;
        Gerenciador_Reservas* gerenciador;

        void carregaReservasNaTabela();                 //Função para que quando adicionado nova reserva seja adicionado na tabela
        QString tipoQuartoToString(TipoQuarto tipo) const;
        QString localidadeToString(Localidade local) const;

};

#endif // LISTARRESERVADIALOG_H
