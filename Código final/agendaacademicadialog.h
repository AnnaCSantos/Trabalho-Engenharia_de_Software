#ifndef AGENDAACADEMICADIALOG_H
#define AGENDAACADEMICADIALOG_H

#include <QDialog>
#include <QSqlDatabase>
#include <QVBoxLayout>
#include <QDate>
#include <QFrame>

namespace Ui {
class AgendaAcademicaDialog;
}

// ---------------------- CLASSE AgendaAcademicaDialog -------------------
//
// Responsabilidade: Gerenciar a interface de Agenda Acadêmica do sistema
// Permite que os usuários:
//   - Visualizem suas tarefas acadêmicas (Provas, Trabalhos, Projetos)
//   - Adicionem novas tarefas com título, descrição, data e disciplina
//   - Filtrem tarefas por tipo, status ou período
//   - Marquem tarefas como concluídas
//   - Removam tarefas do sistema
//
// Herda de: QDialog (janela de diálogo do Qt)
//------------------------------------------------------------------------
class AgendaAcademicaDialog : public QDialog
{
    Q_OBJECT  // Macro necessária para usar signals e slots do Qt

public:
    explicit AgendaAcademicaDialog(QWidget *parent = nullptr, const QString& username = "");

    ~AgendaAcademicaDialog();

protected:
    //----- PROTECTED - Funções que podem ser sobrescritas ------

    // Sobrescreve o filtro de eventos para detectar cliques nos ícones
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    // ------ SLOTS - Funções conectadas a eventos da interface (auto-conectadas) --------
    // Chamado quando o botão "Nova Tarefa" é clicado
    // Abre um diálogo para inserir uma nova tarefa acadêmica
    void on_adicionarTarefaButton_clicked();

    // Chamado quando o botão "Remover" de uma tarefa é clicado
    // Remove a tarefa selecionada do banco de dados
    void on_removerTarefaButton_clicked();

    // Chamado quando o filtro (ComboBox) é alterado
    // Recarrega a lista de tarefas com o filtro selecionado
    void on_filtroComboBox_currentIndexChanged(int index);

    // Chamado quando o botão "Concluir" de uma tarefa é clicado
    // Marca a tarefa como concluída no banco de dados
    void on_marcarConcluidaButton_clicked();

private:
    Ui::AgendaAcademicaDialog *ui;  // Ponteiro para a interface visual (.ui)
    QString loggedInUsername;        // Nome do usuário atualmente logado
    QSqlDatabase dbConnection;       // Conexão com o banco de dados SQLite
    QVBoxLayout *tarefasLayout;      // Layout vertical que contém os cards das tarefas

    // Configura os eventos de clique nos botões da barra de navegação
    void setupNavigationBar();

    // Configura a conexão com o banco de dados
    // Usa a conexão padrão já aberta pelo sistema
    void setupDatabase();

    // Carrega e exibe as tarefas do usuário logado
    // Parâmetro: filtro - tipo de filtro a aplicar (Todos, Provas, Pendentes, etc.)
    void carregarTarefas(const QString& filtro = "Todos");

    // Busca o ID do usuário no banco de dados pelo username
    // Retorna: ID do usuário ou -1 se não encontrado
    int getIdUsuario(const QString& username);

    // Cria a tabela "Tarefas_Academicas" no banco se ela não existir
    // Define a estrutura: id, tipo, título, descrição, data, etc.
    void criarTabelaTarefas();

    // Cria um card visual (QFrame) para exibir uma tarefa
    // Parâmetros: todos os dados da tarefa
    // Retorna: ponteiro para o QFrame criado (card completo)
    QFrame* criarCardTarefa(int id, const QString& tipo, const QString& titulo,
                            const QString& descricao, const QDate& dataEntrega,
                            const QString& disciplina, bool concluida);
};

#endif
