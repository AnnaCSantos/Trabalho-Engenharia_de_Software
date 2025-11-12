#include "consultardisponibilidadedialog.h"
#include "ui_consultardisponibilidadedialog.h"
#include <QMessageBox>
#include <QDate>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>

ConsultarDisponibilidadeDialog::ConsultarDisponibilidadeDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ConsultarDisponibilidadeDialog),
    gerenciador(Gerenciador_Reservas::getInstance())
{
    ui->setupUi(this);
    setWindowTitle("Consultar Disponibilidade");
    QString estiloForcado = "background-color: #F4B315; color: black; border: 1px solid transparent; border-radius: 10px;";
    this->setStyleSheet("QDialog {background-color: #F8F8F8; }");

    popularLocalidades();
    popularTipoQuarto();

    ui->checkInDateEdit->setDisplayFormat("dd/MM/yyyy");
    ui->checkInDateEdit->setDate(QDate::currentDate());
    ui->numDiariasSpinBox->setMinimum(1);
}

ConsultarDisponibilidadeDialog::~ConsultarDisponibilidadeDialog()
{
    delete ui;
}

void ConsultarDisponibilidadeDialog::popularLocalidades()
{
    ui->LocalidadecomboBox->addItem("Jericoacoara", QVariant::fromValue(Localidade::JERICOACOARA));
    ui->LocalidadecomboBox->addItem("Canoa_Quebrada", QVariant::fromValue(Localidade::CANOA_QUEBRADA));
    ui->LocalidadecomboBox->addItem("Cumbuco", QVariant::fromValue(Localidade::CUMBUCO));
}

void ConsultarDisponibilidadeDialog::popularTipoQuarto()
{
    ui->tipoQuartocomboBox->addItem("Solteiro (R$200/dia)", QVariant::fromValue(TipoQuarto::S));
    ui->tipoQuartocomboBox->addItem("Duplo (R$300/dia)", QVariant::fromValue(TipoQuarto::D));
    ui->tipoQuartocomboBox->addItem("Casal (R$350/dia)", QVariant::fromValue(TipoQuarto::C));
    ui->tipoQuartocomboBox->addItem("Triplo (R$450/dia)", QVariant::fromValue(TipoQuarto::T));
    ui->tipoQuartocomboBox->addItem("Quádruplo (R$550/dia)", QVariant::fromValue(TipoQuarto::Q));
}

void ConsultarDisponibilidadeDialog::on_consultarButton_clicked()
{
    Localidade localidade = ui->LocalidadecomboBox->currentData().value<Localidade>();
    TipoQuarto tipoQuarto = ui->tipoQuartocomboBox->currentData().value<TipoQuarto>();
    Data checkIn = Data::fromQDate(ui->checkInDateEdit->date());
    int numDiarias = ui->numDiariasSpinBox->value();

    if(numDiarias <= 0)
    {
        QMessageBox::warning(this,"Erro de Entrada", "Número de diárias deve ser maior que 0");
        return;
    }
    if(gerenciador->verifica_Disponibilidade(localidade, tipoQuarto, checkIn, numDiarias))
    {
        double preco = gerenciador->preco_Total(tipoQuarto, numDiarias);
        QMessageBox::information(this,"disponibilidade", QString("Quarto DISPONÍVEL!\nValor total estimado: R$%1").arg(preco,0,'f',2));
    }
    else
    {
        QMessageBox::warning(this,"Disponibilidade", "Quarto NÃO DISPONÍVEL para as datas selecionadas, tente em outras datas ou outro lugar");
    }
}
