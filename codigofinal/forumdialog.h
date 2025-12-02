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

// ----------------------- CLASSE ForumDialog - Central de Dúvidas EducaUTFPR ---------------------
// Responsabilidade: Gerenciar o sistema de dúvidas e respostas por matéria
// Funcionalidades:
//   - Navegar por categorias de matérias
//   - Buscar dúvidas existentes
//   - Visualizar e responder dúvidas
//   - Sistema de votação (curtidas)
//   - Criar novas dúvidas
// -------------------------------------------------------------------------------------------------
class ForumDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ForumDialog(QWidget *parent = nullptr, const QString& username = "");
    ~ForumDialog();

private slots:
    // Navegação por categorias
    void onCategoriaClicked(const QString& categoria);

    // Abrir matéria específica
    void onMateriaClicked(int idMateria, const QString& nomeMateria);

    // Abrir detalhes de uma dúvida
    void onDuvidaClicked(int idDuvida);


    // Buscar dúvidas
    void on_searchLine_textChanged(const QString& texto);

    // Voltar para categorias
    void voltarParaCategorias();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    Ui::ForumDialog *ui;
    QString loggedInUsername;
    QSqlDatabase dbConnection;
    QVBoxLayout *layoutPrincipal;
    QString categoriaAtual;
    int materiaAtual;

    void setupDatabase();
    void criarTabelasNecessarias();
    void popularMateriasExemplo();
    int getIdUsuario(const QString& username);
    void atualizarEstatisticas();  // <-- ADICIONE ESTA LINHA

    // Carregar dados
    void carregarCategorias();
    void carregarMateriasDaCategoria(const QString& categoria);
    void carregarDuvidasDaMateria(int idMateria);

    // Criar elementos visuais
    QPushButton* criarBotaoCategoria(const QString& categoria, const QString& icone, const QString& cor);
    QPushButton* criarBotaoMateria(int idMateria, const QString& nome, const QString& cor);
    QFrame* criarCardDuvida(int idDuvida, const QString& titulo, const QString& preview,
                            const QString& autor, const QString& data,
                            int numRespostas, int numCurtidas, bool jaRespondida);
};

#endif
