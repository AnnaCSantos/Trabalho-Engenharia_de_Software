#ifndef GRUPOESTUDODIALOG_H
#define GRUPOESTUDODIALOG_H

#include <QDialog>
#include <QSqlDatabase>
#include <QString>
#include <QPushButton>
#include <QFrame>

namespace Ui {
class GrupoEstudoDialog;
}

// ============================================================================
// CLASSE GrupoEstudoDialog
//
// Responsabilidade: Gerenciar grupos de estudo por matéria
// Funcionalidades:
//   - Visualizar matérias por categoria
//   - Criar salas de estudo (públicas ou privadas)
//   - Entrar em salas existentes
//   - Chat em tempo real para cada sala
//   - Navegação para Home e Perfil
// ============================================================================
class GrupoEstudoDialog : public QDialog
{
    Q_OBJECT

public:
    // Construtor
    explicit GrupoEstudoDialog(QWidget *parent = nullptr, const QString& username = "");

    // Destrutor
    ~GrupoEstudoDialog();

protected:
    // ========================================================================
    // PROTECTED - Funções que podem ser sobrescritas
    // ========================================================================

    // Sobrescreve o filtro de eventos para detectar cliques nos ícones
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    // ========================================================================
    // SLOTS - Conectados aos botões da interface
    // ========================================================================

    // Navegação entre telas
    void on_materiasButton_clicked();
    void on_gruposButton_clicked();
    void on_criarButton_clicked();

    // Ações
    void on_confirmarCriarButton_clicked();
    void on_categoriaComboBox_currentIndexChanged(int index);

    // Entrar em grupo privado com código
    void onEntrarGrupoPrivado();

private:
    // ========================================================================
    // ATRIBUTOS
    // ========================================================================
    Ui::GrupoEstudoDialog *ui;
    QString loggedInUsername;
    QSqlDatabase dbConnection;
    QString telaAtual;

    // ========================================================================
    // MÉTODOS AUXILIARES
    // ========================================================================

    // Configuração inicial
    void setupDatabase();
    void criarTabelasNecessarias();
    void popularMateriasCompletas();  // ✅ TODAS AS MATÉRIAS DA UTFPR
    void setupNavigationBar();        // ✅ NOVO: Configura navegação

    // Navegação
    void mostrarTela(const QString& tela);

    // Carregar dados
    void carregarMateriasDaCategoria(const QString& categoria);
    void carregarSalasPublicas();

    // Criar elementos visuais
    QPushButton* criarBotaoMateria(int idMateria, const QString& nome,
                                   const QString& icone, const QString& cor);
    QFrame* criarCardSala(int idSala, const QString& codigoSala,
                          const QString& nomeSala, const QString& tipo,
                          int numParticipantes, int maxParticipantes);

    // Ações do usuário
    void onMateriaClicked(int idMateria);
    void onSalaClicked(int idSala);

    // Utilidades
    int getIdUsuario(const QString& username);
    QString gerarCodigoSala();
    bool validarCriacaoSala();
};

#endif // GRUPOESTUDODIALOG_H
