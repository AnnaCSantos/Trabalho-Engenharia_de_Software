#include "consultarreservadialog.h"
#include "ui_consultarreservadialog.h"

ConsultarReservaDialog::ConsultarReservaDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ConsultarReservaDialog)
{
    ui->setupUi(this);
}

ConsultarReservaDialog::~ConsultarReservaDialog()
{
    delete ui;
}
