#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>

// ✅ Forward declarations - declara as classes sem incluir os headers
class PerfilDialog;
class AvaliacaoMateriasDialog;

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

//------------------------------------------------------------------
// Responsabilidade: Janela principal do sistema EducaUTFPR
// Gerencia a navegação entre os diferentes módulos do sistema:
//   - Central de Dúvidas (Fórum)
//   - Dúvidas (Sistema de perguntas e respostas por disciplina)
//   - Grupos de Estudo (Listar Reservas)
//   - Agenda Acadêmica (Gerenciar tarefas acadêmicas)
//   - Ranking de Dificuldade (Visualizar grau de dificuldade das matérias)
//
// Também exibe informações do usuário logado
//-----------------------------------------------------------------
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    // -------- SLOT PÚBLICO - Define o usuário logado no sistema ------------
    // Atualiza a label de boas-vindas com o nome do usuário
    // -----------------------------------------------------------------------
    void setLoggedInUser(const QString& username);

protected:
    // Sobrescreve o filtro de eventos para capturar o clique no label de perfil
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    // Abre a janela de Agenda Acadêmica
    void on_agendaButton_clicked();

    // Abre a Central de Dúvidas (Fórum)
    void on_centralDuvidasButton_clicked();

    // Abre a janela de Grupos de Estudo
    void on_grupoButton_clicked();

    // Abre a janela de Dúvidas (Sistema de perguntas/respostas)
    void on_duvidasButton_clicked();

    // Fecha o sistema
    void on_sairButton_clicked();

    // Abre a janela de Ranking de Dificuldade das Matérias
    void on_avaliacaoButton_clicked();

    // Abre a janela de Perfil do usuário
    void on_perfil_clicked();

private:
    Ui::MainWindow *ui;                  // Interface visual
    QString loggedInUsername;            // Nome do usuário logado
};

#endif
