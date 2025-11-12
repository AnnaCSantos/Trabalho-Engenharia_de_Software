#ifndef POSTDETAILSDIALOG_H
#define POSTDETAILSDIALOG_H

#include <QDialog>
#include <QSqlDatabase>
#include <QVBoxLayout>

namespace Ui {
class PostDetailsDialog;
}

class PostDetailsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PostDetailsDialog(QWidget *parent = nullptr, int idPostagem = 0,
                               const QString& username = "");
    ~PostDetailsDialog();

private slots:
    void darLikePostagem();
    void adicionarResposta();
    void darLikeResposta(int idResposta);

private:
    Ui::PostDetailsDialog *ui;
    int postagemId;
    QString loggedInUsername;
    QSqlDatabase dbConnection;
    QVBoxLayout *respostasLayout;

    void carregarPostagem();
    void carregarRespostas();
    int getIdUsuario(const QString& username);
    bool usuarioJaDeuLike(int idPostagem);
    bool usuarioJaDeuLikeResposta(int idResposta);
};

#endif // POSTDETAILSDIALOG_H
