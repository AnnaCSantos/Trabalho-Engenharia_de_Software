#ifndef CONSULTARDISPONIBILIDADEDIALOG_H
#define CONSULTARDISPONIBILIDADEDIALOG_H

#include <QDate>
#include <QString>
#include <QDialog>
#include <QObject>

//Adicionar os backends necessários
#include "backend/gerenciador_reservas.h"
#include "backend/tipo_hotel.h"
#include "backend/data.h"

namespace Ui
{
class ConsultarDisponibilidadeDialog;
}

class ConsultarDisponibilidadeDialog : public QDialog
{
    Q_OBJECT

    public:
        explicit ConsultarDisponibilidadeDialog(QWidget *parent = nullptr);
        ~ConsultarDisponibilidadeDialog();

    private slots:
        //Para conectar com o pushButton
        void on_consultarButton_clicked();

    private:
        Ui::ConsultarDisponibilidadeDialog *ui;
        Gerenciador_Reservas* gerenciador;

        //Auxiliares para os QComboBoxs
        void popularLocalidades();
        void popularTipoQuarto();
};

#endif // CONSULTARDISPONIBILIDADEDIALOG_H
