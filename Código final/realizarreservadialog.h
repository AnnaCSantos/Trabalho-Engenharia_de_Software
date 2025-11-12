#ifndef REALIZARRESERVADIALOG_H
#define REALIZARRESERVADIALOG_H

#include <QDialog>
#include <QString>
#include <QDate>
#include <QSpinBox>
#include <QObject>

//Armazenar os backend que iremos usar nessa janela
#include "backend/gerenciador_reservas.h"
#include "backend/cliente.h"
#include "backend/data.h"
#include "backend/politica_desconto.h"
#include "backend/reserva.h"
#include "backend/tipo_hotel.h"

namespace Ui {
class RealizarReservaDialog;
}

class RealizarReservaDialog : public QDialog
{
    Q_OBJECT

    public:
        explicit RealizarReservaDialog(QWidget *parent = nullptr, const QString& atendenteUsername = "");
        ~RealizarReservaDialog();

    private slots:
        //Para fazer a escolha de cada item
        void on_confirmarReservaButton_clicked();
        void on_localidadeComboBox_currentIndexChanged(int index);
        void on_tipoQuartocomboBox_currentIndexChanged(int index);
        void on_numDiariasSpinBox_valueChanged(int value);
        void on_descontoComboBox_currentIndexChanged(int index);

    private:
        Ui::RealizarReservaDialog *ui;
        Gerenciador_Reservas* gerenciador;
        QString atendenteLogado;

        //Auxiliares métodos
        void popularLocalidades();
        void popularTiposQuarto();
        void popularPoliticasDesconto();
        void atualizarValorTotalEstimado();     //Armazena o valor após o calculo com desconto
        double calcularPrecoBase();             //Preço base é o que esta com o desconto já
};

#endif // REALIZARRESERVADIALOG_H
