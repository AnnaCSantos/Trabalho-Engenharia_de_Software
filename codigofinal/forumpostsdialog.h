#ifndef FORUMPOSTSDIALOG_H
#define FORUMPOSTSDIALOG_H

#include <QDialog>
#include <QSqlDatabase>
#include <QVBoxLayout>

namespace Ui {
class ForumPostsDialog;
}

// ============================================================================
// CLASSE ForumPostsDialog - Detalhes de uma Dúvida
// ============================================================================
// Responsabilidade: Exibir uma dúvida completa com suas respostas
// Funcionalidades:
//   - Mostrar título, descrição, autor e data da dúvida
//   - Listar todas as respostas
//   - Permitir adicionar novas respostas
//   - Sistema de curtidas (dúvidas e respostas)
// ============================================================================
class ForumPostsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ForumPostsDialog(QWidget *parent = nullptr,
                              int idDuvida = 0,
                              const QString& username = "");
    ~ForumPostsDialog();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;  // <-- ADICIONE ESTA LINHA

private slots:
    void on_responderButton_clicked();
    void on_voltarButton_clicked();
    void onCurtirDuvida();
    void onCurtirResposta(int idResposta);

private:
    Ui::ForumPostsDialog *ui;
    int duvidaId;
    QString loggedInUsername;
    QSqlDatabase dbConnection;
    QVBoxLayout *respostasLayout;

    // Métodos auxiliares
    void setupDatabase();
    void carregarDuvida();
    void carregarRespostas();
    int getIdUsuario(const QString& username);
    bool usuarioJaCurtiuDuvida();
    bool usuarioJaCurtiuResposta(int idResposta);
};

#endif
