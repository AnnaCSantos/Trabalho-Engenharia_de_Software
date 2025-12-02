#ifndef FORUMDIALOG_H
#define FORUMDIALOG_H

#include <QDialog>
#include <QSqlDatabase>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFrame>
#include <QLabel>

namespace Ui {
class ForumDialog;
}

class ForumDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ForumDialog(QWidget *parent = nullptr, const QString& username = "");
    ~ForumDialog();

private slots:
    void onCategoriaClicked(const QString& categoria);
    void onMateriaClicked(int idDisciplina, const QString& nomeDisciplina);
    void onDuvidaClicked(int idDuvida);
    void on_searchLine_textChanged(const QString& texto);
    void voltarParaCategorias();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    Ui::ForumDialog *ui;
    QString loggedInUsername;
    QSqlDatabase dbConnection;
    QVBoxLayout *layoutPrincipal;
    QString categoriaAtual;
    int disciplinaAtual;

    void setupDatabase();
    void criarTabelasDeCurtidas();
    int getIdUsuario(const QString& username);
    void atualizarEstatisticas();

    void carregarCategorias();
    void carregarDisciplinasDaCategoria(const QString& categoria);
    void carregarDuvidasDaDisciplina(int idDisciplina);

    // Funções visuais
    QPushButton* criarBotaoCategoria(const QString& categoria);
    QPushButton* criarBotaoDisciplina(int id, const QString& nome);
    QFrame* criarCardDuvida(int idDuvida, const QString& titulo, const QString& preview,
                            const QString& autor, const QString& data,
                            int numRespostas, int numCurtidas, bool jaRespondida);

    QString identificarCategoria(const QString& nomeDisciplina);
    QPair<QString, QString> getEstiloCategoria(const QString& categoria);
};

#endif // FORUMDIALOG_H
