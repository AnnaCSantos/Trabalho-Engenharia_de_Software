#ifndef CHATMATERIA_H
#define CHATMATERIA_H

#include <QDialog>
#include <QSqlDatabase>
#include <QString>
#include <QTimer>

namespace Ui {
class ChatMateria;
}

// ============================================================================
// CHAT MATERIA - Chat em tempo real para grupos de estudo
// ============================================================================
// Funcionalidades:
//   - Enviar e receber mensagens em tempo real
//   - Ver histórico de mensagens
//   - Identificar quem enviou cada mensagem
//   - Auto-scroll para mensagens novas
//   - Lista de participantes online
// ============================================================================

class ChatMateria : public QDialog
{
    Q_OBJECT

public:
    explicit ChatMateria(QWidget *parent = nullptr,
                         const QString& username = "",
                         int idSala = 0,
                         const QString& nomeSala = "");
    ~ChatMateria();

private slots:
    void on_enviarButton_clicked();
    void on_sairButton_clicked();
    void atualizarChat();  // Chamado pelo timer para carregar novas mensagens

private:
    Ui::ChatMateria *ui;
    QString loggedInUsername;
    int idSala;
    QString nomeSala;
    QSqlDatabase dbConnection;
    QTimer *timerAtualizacao;
    int ultimaMensagemId;  // ID da última mensagem carregada

    // -------- FUNÇÕES AUXILIARES --------

    void setupDatabase();
    int getIdUsuario(const QString& username);
    void carregarHistorico();
    void carregarParticipantes();
    void enviarMensagem(const QString& mensagem);
    void adicionarMensagemAoChat(const QString& usuario, const QString& mensagem,
                                 bool ehMinha, const QString& horario);
    void scrollParaFinal();
};

#endif // CHATMATERIA_H
