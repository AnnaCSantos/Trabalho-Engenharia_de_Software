#ifndef CONSULTARRESERVADIALOG_H
#define CONSULTARRESERVADIALOG_H

#include <QDialog>

namespace Ui {
class ConsultarReservaDialog;
}

class ConsultarReservaDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConsultarReservaDialog(QWidget *parent = nullptr);
    ~ConsultarReservaDialog();

private:
    Ui::ConsultarReservaDialog *ui;
};

#endif // CONSULTARRESERVADIALOG_H
