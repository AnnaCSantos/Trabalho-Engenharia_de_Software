#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QMessageBox>
#include <QFile>    //verifica se o relatorio foi salvo
#include <QApplication>

//Outras telas pra quando a pessoa clicar nos botões
#include "consultardisponibilidadedialog.h"
#include "realizarreservadialog.h"
#include "listarreservadialog.h"
#include "forumdialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),
    gerenciador(Gerenciador_Reservas::getInstance())
{
    ui->setupUi(this);
    setWindowTitle("Rede Paradise Hotéis - Sistema de Reserva");


    hide();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setLoggedInUser(const QString& username)
{
    loggedInUsername = username;
    //Para colocar o nome do atendente no Bem vindos
    ui->atendentelabel->setText("Bem-vindo, " + username + "!");
}

void MainWindow::on_forumButton_clicked()
{
    ForumDialog dialog(this);
    dialog.exec();
}

void MainWindow::on_realizarReservaButton_clicked()
{
    RealizarReservaDialog dialog(this, loggedInUsername);
    dialog.exec();
}

void MainWindow::on_grupoButton_clicked()
{
    ListarReservaDialog dialog(this);
    dialog.exec();
}

void MainWindow::on_gerarRelatorioButton_clicked()
{
    QString fileName = "relatorio_reservas_paradise.txt";
    gerenciador->gerar_Relatorio(fileName.toStdString());

    if(QFile::exists(fileName))
    {
        QMessageBox::information(this, "Relatório", QString("Relatório salvo com sucesso em: %1").arg(fileName));
    }
    else
    {
        QMessageBox::critical(this,"Erro", "Erro ao gerar o relatório. Verifique as permissões ou espaço ");
    }
}

void MainWindow::on_sairButton_clicked()
{
    QMessageBox::information(this,"Sair", "Saindo do sistema, até logoo....");
    QApplication::quit();
}
