#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QString>
#include <QObject>
#include <QAction>
#include <QIcon>
#include <QSqlDatabase>
#include <QtSql/QtSql>

namespace Ui
{
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

signals:
    void loggedIn(const QString& username);  // Sinal emitido quando login é bem sucedido

private slots:
    void on_entrarButton_clicked();
    void on_criarconta_clicked();

private:
    Ui::LoginDialog *ui;
    QSqlDatabase DBConnection;  // Conexão com o banco de dados
};

#endif // LOGINDIALOG_H
