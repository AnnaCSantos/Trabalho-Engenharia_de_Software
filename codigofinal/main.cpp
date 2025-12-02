#include <QApplication>
#include <QSqlDatabase>
#include <QMessageBox>
#include <QSqlError>
#include <QDebug>
#include <QStyleFactory>
// Inclua os cabeçalhos das suas janelas
#include "logindialog.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setStyle(QStyleFactory::create("Fusion"));
    // === DIAGNÓSTICO DE DRIVERS SQL ===
    qDebug() << "=== DIAGNÓSTICO DE DRIVERS SQL ===";
    qDebug() << "Drivers SQL disponíveis:" << QSqlDatabase::drivers();
    qDebug() << "===================================";

    // Cria a janela principal (mas não mostra ainda)
    MainWindow mainWindow;

    // Loop até o usuário fazer login com sucesso ou fechar a janela
    while(true) {
        // Cria o diálogo de login
        LoginDialog loginDialog;

        // Conecta os sinais à MainWindow
        QObject::connect(&loginDialog, &LoginDialog::loggedIn,
                         &mainWindow, &MainWindow::setLoggedInUser);
        QObject::connect(&loginDialog, &LoginDialog::loggedIn,
                         &mainWindow, &MainWindow::show);

        // Mostra o diálogo de login
        int result = loginDialog.exec();

        if(result == QDialog::Accepted) {
            // Login bem-sucedido! Inicia o loop principal
            qDebug() << "Login aceito! Iniciando aplicação...";
            return a.exec();
        } else {
            // Usuário fechou a janela de login (clicou no X)
            qDebug() << "Login cancelado. Encerrando aplicação...";
            return 0;
        }
    }
}
