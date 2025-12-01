#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QMessageBox>
#include <QFile>
#include <QApplication>

// ============================================================================
// INCLUDES DAS JANELAS DO SISTEMA
// ============================================================================
#include "forumdialog.h"
#include "agendaacademicadialog.h"
#include "duvidasdialog.h"
#include "grupoestudodialog.h"  // ✅ NOVO: Sistema de Grupos de Estudo

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("EducaUTFPR - Home");
    hide();
}

// DESTRUTOR - Libera memória
MainWindow::~MainWindow()
{
    delete ui;
}

// SET LOGGED IN USER - Define o usuário logado e atualiza a interface
void MainWindow::setLoggedInUser(const QString& username)
{
    loggedInUsername = username;
    // Atualiza a label de boas-vindas com o nome do usuário
    ui->atendentelabel->setText("Bem-vindo, " + username + "!");
}

// SLOT: Botão Central de Dúvidas (Fórum)
void MainWindow::on_centralDuvidasButton_clicked()
{
    ForumDialog dialog(this);
    dialog.exec();
}

// SLOT: Botão Dúvidas (Sistema de perguntas/respostas por disciplina)
void MainWindow::on_duvidasButton_clicked()
{
    DuvidasDialog dialog(this, loggedInUsername);
    dialog.exec();
}

// ============================================================================
// SLOT: Botão Grupos de Estudo (ATUALIZADO)
// Abre o sistema completo de grupos de estudo com chat
// ============================================================================
void MainWindow::on_grupoButton_clicked()
{
    GrupoEstudoDialog dialog(this, loggedInUsername);
    dialog.exec();
}

// SLOT: Botão Agenda Acadêmica
void MainWindow::on_agendaButton_clicked()
{
    // Cria e exibe a janela de Agenda Acadêmica
    AgendaAcademicaDialog dialog(this, loggedInUsername);
    dialog.exec();  // Exibe como modal (bloqueia a janela principal)
}

// SLOT: Botão Sair do Sistema
void MainWindow::on_sairButton_clicked()
{
    QMessageBox::information(this, "Sair", "Saindo do sistema, até logo....");
    QApplication::quit();
}
