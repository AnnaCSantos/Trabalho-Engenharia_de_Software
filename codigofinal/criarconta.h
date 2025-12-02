#ifndef CRIARCONTA_H
#define CRIARCONTA_H

#include <QDialog>
#include <QSqlDatabase>

namespace Ui {
class CriarConta;
}

class CriarConta : public QDialog
{
    Q_OBJECT

public:
    explicit CriarConta(QWidget *parent = nullptr);
    ~CriarConta();

private slots:
    void on_criarcontaButton_clicked();  // Slot para o botão Criar Conta
    void on_pushButton_2_clicked();      // Slot para o botão Login (voltar)

private:
    Ui::CriarConta *ui;
    QSqlDatabase DBConnection;

    bool validarCampos();  // Valida se os campos estão preenchidos
    bool usuarioJaExiste(const QString &usuario);  // Verifica se usuário já existe
};

#endif // CRIARCONTA_H
