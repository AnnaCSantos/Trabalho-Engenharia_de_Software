#ifndef DUVIDASDIALOG_H
#define DUVIDASDIALOG_H

#include <QDialog>
#include <QSqlDatabase>
#include <QVBoxLayout>
#include <QFrame>
#include <QComboBox>

namespace Ui {
class DuvidasDialog;
}

// ---------------------- CLASSE DuvidasDialog -------------------
//
// Responsabilidade: Gerenciar a interface de Dúvidas do sistema
// Permite que os usuários:
//   - Visualizem suas dúvidas por disciplina
//   - Adicionem novas dúvidas com título, descrição e imagens
//   - Filtrem dúvidas por disciplina e status
//   - Respondam dúvidas de outros usuários
//   - Recebam notificações quando suas dúvidas forem respondidas
//
// Herda de: QDialog (janela de diálogo do Qt)
//------------------------------------------------------------------------
class DuvidasDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DuvidasDialog(QWidget *parent = nullptr, const QString& username = "");
    ~DuvidasDialog();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void on_adicionarDuvidaButton_clicked();
    void on_filtroComboBox_currentIndexChanged(int index);

private:
    Ui::DuvidasDialog *ui;
    QString loggedInUsername;
    QSqlDatabase dbConnection;
    QVBoxLayout *duvidasLayout;

    void setupNavigationBar();
    void setupDatabase();
    void carregarDuvidas(const QString& filtro = "Todas");
    int getIdUsuario(const QString& username);
    void criarTabelaDuvidas();
    void criarTabelaRespostas();
    void criarTabelaNotificacoes();
    int contarNotificacoesNaoLidas();

    void garantirTabelaDisciplinas();
    void popularComboDisciplinas(QComboBox *combo, bool incluirOpcaoTodas);

    QFrame* criarCardDuvida(int id, const QString& disciplina, const QString& titulo,
                            const QString& descricao, const QString& imagemPath,
                            const QString& status, const QString& nomeAutor,
                            const QString& dataCriacao, int numRespostas);

    void abrirDetalheDuvida(int idDuvida);
    void notificarAutor(int idDuvida, const QString& nomeRespondente);
};

#endif // DUVIDASDIALOG_H
