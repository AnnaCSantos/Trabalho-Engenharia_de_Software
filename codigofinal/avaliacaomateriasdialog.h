#ifndef AVALIACAOMATERIASDIALOG_H
#define AVALIACAOMATERIASDIALOG_H

#include <QDialog>
#include <QSqlDatabase>
#include <QVBoxLayout>

namespace Ui {
class AvaliacaoMateriasDialog;
}

class AvaliacaoMateriasDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AvaliacaoMateriasDialog(QWidget *parent = nullptr, const QString& username = "");
    ~AvaliacaoMateriasDialog();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void onCategoriaChanged(int index);
    void voltarParaHome();

private:
    Ui::AvaliacaoMateriasDialog *ui;
    QString loggedInUsername;
    QSqlDatabase dbConnection;
    QVBoxLayout *layoutPrincipal;

    void setupDatabase();
    void criarTabelasNecessarias();
    void carregarCategorias();
    int getIdUsuario(const QString& username);
    void carregarMaterias(int idCategoria);
    void criarCardMateria(int idMateria, const QString& nome,
                          int facil, int medio, int dificil,
                          double mediaNotas, int totalNotas, int totalDuvidas);
    void votarDificuldade(int idMateria, const QString& nivel);
    void avaliarNota(int idMateria, int nota);
};

#endif
