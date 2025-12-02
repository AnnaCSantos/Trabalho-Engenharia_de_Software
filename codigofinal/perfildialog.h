#ifndef PERFILDIALOG_H
#define PERFILDIALOG_H

#include <QDialog>
#include <QSqlDatabase>
#include <QString>

namespace Ui {
class PerfilDialog;
}

//  --------------- PERFIL DIALOG - Gerencia o perfil do usuário  ---------------
// Funcionalidades:
//   - Visualizar informações do usuário
//   - Editar dados pessoais (nome, email, curso, semestre)
//   - Alterar senha
//   - Ver estatísticas acadêmicas (tarefas concluídas, pendentes, etc)
//   - Upload de foto de perfil (opcional)
// ------------------------------------------------------------------------------

class PerfilDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PerfilDialog(QWidget *parent = nullptr, const QString& username = "");
    ~PerfilDialog();

private slots:
    // Botão para entrar no modo de edição
    void on_editarButton_clicked();

    // Botão para salvar as alterações
    void on_salvarButton_clicked();

    // Botão para cancelar edição
    void on_cancelarButton_clicked();

    // Botão para alterar senha
    void on_alterarSenhaButton_clicked();

    // Botão para escolher foto de perfil
    void on_escolherFotoButton_clicked();

private:
    Ui::PerfilDialog *ui;
    QString loggedInUsername;        // Nome de usuário (login)
    QSqlDatabase dbConnection;       // Conexão com banco de dados
    bool modoEdicao;                 // Controla se está em modo edição

    // -------- FUNÇÕES AUXILIARES --------

    // Conecta ao banco de dados
    void setupDatabase();

    // Carrega todas as informações do usuário do banco
    void carregarDadosUsuario();

    // Busca o ID do usuário pelo username
    int getIdUsuario(const QString& username);

    // Carrega e exibe as estatísticas acadêmicas
    void carregarEstatisticas();

    // Ativa/desativa campos para edição
    void setModoEdicao(bool edicao);

    // Valida os campos antes de salvar
    bool validarCampos();
};

#endif
