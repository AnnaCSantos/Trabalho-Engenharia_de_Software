#ifndef FORUMPOSTSDIALOG_H
#define FORUMPOSTSDIALOG_H

#include <QDialog>
#include <QSqlDatabase>

namespace Ui {
class ForumPostsDialog;
}

class ForumPostsDialog : public QDialog
{
    Q_OBJECT

public:
    // O construtor recebe o ID do Fórum que foi clicado
    explicit ForumPostsDialog(QWidget *parent = nullptr, int forumId = 0);
    ~ForumPostsDialog();

private slots:
    // Slot para quando um post específico for clicado
    void on_postClicked(int postId, const QString &titulo);

    //void on_voltarButton_clicked();

private:
    Ui::ForumPostsDialog *ui; // Ponteiro para os itens do .ui
    QSqlDatabase dbConnection;
    int m_forumId; // Variável para guardar o ID do fórum

    void setupDatabase();
    void carregarPosts(); // função que cria os itens dinâmicos
    int contarLikes(int postId); // Função auxiliar para contar os likes
};

#endif // FORUMPOSTSDIALOG_H
