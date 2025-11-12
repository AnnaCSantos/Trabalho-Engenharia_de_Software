#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include "backend/gerenciador_reservas.h"
#include "logindialog.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

    public:
        explicit MainWindow(QWidget *parent = nullptr);
        ~MainWindow();

    public slots:
        void setLoggedInUser(const QString& username);

    private slots:
        //Slots pra cada janela diferente e o sair
        void on_forumButton_clicked();
        void on_realizarReservaButton_clicked();
        void on_grupoButton_clicked();
        void on_gerarRelatorioButton_clicked();
        void on_sairButton_clicked();

    private:
        Ui::MainWindow *ui;
        QString loggedInUsername;          // Armazena o nome do atendente para salvar no 'Bem vindo'
        Gerenciador_Reservas* gerenciador; //Para egrar os relatorios
};
#endif // MAINWINDOW_H
