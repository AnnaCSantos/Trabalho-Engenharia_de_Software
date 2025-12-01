#include "chatmateria.h"
#include "ui_chatmateria.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QFrame>
#include <QLabel>
#include <QVBoxLayout>
#include <QScrollBar>
#include <QDateTime>

// ============================================================================
// CONSTRUTOR
// ============================================================================
ChatMateria::ChatMateria(QWidget *parent, const QString& username,
                         int idSalaParam, const QString& nomeSalaParam)
    : QDialog(parent)
    , ui(new Ui::ChatMateria)
    , loggedInUsername(username)
    , idSala(idSalaParam)
    , nomeSala(nomeSalaParam)
    , ultimaMensagemId(0)
{
    ui->setupUi(this);
    setWindowTitle(QString("💬 Chat - %1").arg(nomeSala));
    resize(800, 600);

    setupDatabase();
    carregarHistorico();
    carregarParticipantes();

    // Configura o timer para atualizar o chat a cada 2 segundos
    timerAtualizacao = new QTimer(this);
    connect(timerAtualizacao, &QTimer::timeout, this, &ChatMateria::atualizarChat);
    timerAtualizacao->start(2000);  // Atualiza a cada 2 segundos

    // Foco no campo de mensagem
    ui->mensagemEdit->setFocus();

    // Permite enviar com Enter
    connect(ui->mensagemEdit, &QLineEdit::returnPressed,
            this, &ChatMateria::on_enviarButton_clicked);
}

// ============================================================================
// DESTRUTOR
// ============================================================================
ChatMateria::~ChatMateria()
{
    timerAtualizacao->stop();
    delete ui;
}

// ============================================================================
// SETUP DATABASE
// ============================================================================
void ChatMateria::setupDatabase()
{
    dbConnection = QSqlDatabase::database("qt_sql_default_connection");

    if (!dbConnection.isOpen()) {
        qDebug() << "[ChatMateria] ERRO: Banco de dados não está aberto.";
    }
}

// ============================================================================
// GET ID USUARIO
// ============================================================================
int ChatMateria::getIdUsuario(const QString& username)
{
    QSqlQuery query(dbConnection);
    query.prepare("SELECT id_usuario FROM Usuario WHERE usuario = ?");
    query.addBindValue(username);

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    return -1;
}

// ============================================================================
// CARREGAR HISTÓRICO - Carrega todas as mensagens antigas
// ============================================================================
void ChatMateria::carregarHistorico()
{
    // Limpa o container de mensagens
    QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(ui->chatContainer->layout());
    if (!layout) {
        layout = new QVBoxLayout(ui->chatContainer);
        layout->setSpacing(10);
        layout->setContentsMargins(15, 15, 15, 15);
        layout->setAlignment(Qt::AlignTop);
        ui->chatContainer->setLayout(layout);
    }

    QLayoutItem *item;
    while ((item = layout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    // Busca todas as mensagens da sala
    QSqlQuery query(dbConnection);
    query.prepare(
        "SELECT m.id_mensagem, m.mensagem, m.data_envio, u.usuario "
        "FROM Mensagens_Chat m "
        "JOIN Usuario u ON m.id_usuario = u.id_usuario "
        "WHERE m.id_sala = ? "
        "ORDER BY m.data_envio ASC"
        );
    query.addBindValue(idSala);

    if (!query.exec()) {
        qDebug() << "Erro ao carregar mensagens:" << query.lastError().text();
        return;
    }

    // Adiciona cada mensagem ao chat
    while (query.next()) {
        int idMensagem = query.value("id_mensagem").toInt();
        QString mensagem = query.value("mensagem").toString();
        QString dataHora = query.value("data_envio").toString();
        QString usuario = query.value("usuario").toString();

        // Formata a hora (HH:mm)
        QDateTime dt = QDateTime::fromString(dataHora, "yyyy-MM-dd HH:mm:ss");
        QString horario = dt.toString("HH:mm");

        bool ehMinha = (usuario == loggedInUsername);

        adicionarMensagemAoChat(usuario, mensagem, ehMinha, horario);

        // Atualiza o ID da última mensagem
        if (idMensagem > ultimaMensagemId) {
            ultimaMensagemId = idMensagem;
        }
    }

    scrollParaFinal();
}

// ============================================================================
// ATUALIZAR CHAT - Carrega apenas mensagens novas
// ============================================================================
void ChatMateria::atualizarChat()
{
    QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(ui->chatContainer->layout());
    if (!layout) return;

    // Busca apenas mensagens novas (id > ultimaMensagemId)
    QSqlQuery query(dbConnection);
    query.prepare(
        "SELECT m.id_mensagem, m.mensagem, m.data_envio, u.usuario "
        "FROM Mensagens_Chat m "
        "JOIN Usuario u ON m.id_usuario = u.id_usuario "
        "WHERE m.id_sala = ? AND m.id_mensagem > ? "
        "ORDER BY m.data_envio ASC"
        );
    query.addBindValue(idSala);
    query.addBindValue(ultimaMensagemId);

    if (!query.exec()) {
        qDebug() << "Erro ao atualizar chat:" << query.lastError().text();
        return;
    }

    bool temNovasMensagens = false;

    while (query.next()) {
        int idMensagem = query.value("id_mensagem").toInt();
        QString mensagem = query.value("mensagem").toString();
        QString dataHora = query.value("data_envio").toString();
        QString usuario = query.value("usuario").toString();

        QDateTime dt = QDateTime::fromString(dataHora, "yyyy-MM-dd HH:mm:ss");
        QString horario = dt.toString("HH:mm");

        bool ehMinha = (usuario == loggedInUsername);

        adicionarMensagemAoChat(usuario, mensagem, ehMinha, horario);

        ultimaMensagemId = idMensagem;
        temNovasMensagens = true;
    }

    // Se teve novas mensagens, faz scroll para o final
    if (temNovasMensagens) {
        scrollParaFinal();
    }
}

// ============================================================================
// ADICIONAR MENSAGEM AO CHAT - Cria o elemento visual da mensagem
// ============================================================================
void ChatMateria::adicionarMensagemAoChat(const QString& usuario, const QString& mensagem,
                                          bool ehMinha, const QString& horario)
{
    QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(ui->chatContainer->layout());
    if (!layout) return;

    // Frame da mensagem
    QFrame *msgFrame = new QFrame();
    msgFrame->setMaximumWidth(500);

    // Define o alinhamento (direita se for minha, esquerda se for dos outros)
    QHBoxLayout *alignLayout = new QHBoxLayout();
    if (ehMinha) {
        alignLayout->addStretch();
    }

    // Layout interno da mensagem
    QVBoxLayout *msgLayout = new QVBoxLayout(msgFrame);
    msgLayout->setSpacing(5);
    msgLayout->setContentsMargins(12, 10, 12, 10);

    // Nome do usuário (só mostra se não for minha)
    if (!ehMinha) {
        QLabel *nomeLabel = new QLabel(usuario);
        nomeLabel->setStyleSheet(
            "font-size: 11px; "
            "font-weight: bold; "
            "color: #D3AF35;"
            );
        msgLayout->addWidget(nomeLabel);
    }

    // Texto da mensagem
    QLabel *textoLabel = new QLabel(mensagem);
    textoLabel->setWordWrap(true);
    textoLabel->setStyleSheet(
        "font-size: 14px; "
        "color: white;"
        );
    msgLayout->addWidget(textoLabel);

    // Horário
    QLabel *horaLabel = new QLabel(horario);
    horaLabel->setAlignment(ehMinha ? Qt::AlignRight : Qt::AlignLeft);
    horaLabel->setStyleSheet(
        "font-size: 10px; "
        "color: #8E6915;"
        );
    msgLayout->addWidget(horaLabel);

    // Estilo do frame baseado em quem enviou
    if (ehMinha) {
        msgFrame->setStyleSheet(
            "QFrame {"
            "   background-color: #F4B315;"
            "   border-radius: 10px;"
            "   padding: 5px;"
            "}"
            );
        textoLabel->setStyleSheet("font-size: 14px; color: #1A161A;");
        horaLabel->setStyleSheet("font-size: 10px; color: #423738;");
    } else {
        msgFrame->setStyleSheet(
            "QFrame {"
            "   background-color: #423738;"
            "   border-radius: 10px;"
            "   padding: 5px;"
            "}"
            );
    }

    alignLayout->addWidget(msgFrame);

    if (!ehMinha) {
        alignLayout->addStretch();
    }

    // Widget container para o alinhamento
    QWidget *containerWidget = new QWidget();
    containerWidget->setLayout(alignLayout);

    layout->addWidget(containerWidget);
}

// ============================================================================
// SCROLL PARA FINAL - Move a barra de rolagem para o fim
// ============================================================================
void ChatMateria::scrollParaFinal()
{
    QScrollBar *scrollBar = ui->scrollArea->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

// ============================================================================
// ENVIAR MENSAGEM
// ============================================================================
void ChatMateria::enviarMensagem(const QString& mensagem)
{
    if (mensagem.trimmed().isEmpty()) {
        return;
    }

    int idUsuario = getIdUsuario(loggedInUsername);
    if (idUsuario == -1) {
        QMessageBox::warning(this, "Erro", "Usuário não encontrado!");
        return;
    }

    // Insere a mensagem no banco
    QSqlQuery query(dbConnection);
    query.prepare(
        "INSERT INTO Mensagens_Chat (id_sala, id_usuario, mensagem) "
        "VALUES (?, ?, ?)"
        );
    query.addBindValue(idSala);
    query.addBindValue(idUsuario);
    query.addBindValue(mensagem);

    if (query.exec()) {
        // Atualiza o chat imediatamente para mostrar a mensagem
        atualizarChat();

        // Limpa o campo de texto
        ui->mensagemEdit->clear();
    } else {
        QMessageBox::critical(this, "Erro",
                              "Erro ao enviar mensagem: " + query.lastError().text());
    }
}

// ============================================================================
// CARREGAR PARTICIPANTES - Lista os membros da sala
// ============================================================================
void ChatMateria::carregarParticipantes()
{
    QSqlQuery query(dbConnection);
    query.prepare(
        "SELECT u.usuario "
        "FROM Participantes_Sala p "
        "JOIN Usuario u ON p.id_usuario = u.id_usuario "
        "WHERE p.id_sala = ? "
        "ORDER BY u.usuario"
        );
    query.addBindValue(idSala);

    if (!query.exec()) {
        qDebug() << "Erro ao carregar participantes:" << query.lastError().text();
        return;
    }

    QStringList participantes;
    while (query.next()) {
        participantes << query.value(0).toString();
    }

    // Atualiza o label de participantes
    QString textoParticipantes = QString("👥 %1 participantes: %2")
                                     .arg(participantes.count())
                                     .arg(participantes.join(", "));

    ui->participantesLabel->setText(textoParticipantes);
}

// ============================================================================
// BOTÃO ENVIAR
// ============================================================================
void ChatMateria::on_enviarButton_clicked()
{
    QString mensagem = ui->mensagemEdit->text();
    enviarMensagem(mensagem);
}

// ============================================================================
// BOTÃO SAIR
// ============================================================================
void ChatMateria::on_sairButton_clicked()
{
    this->close();
}
