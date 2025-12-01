#include "forumpostsdialog.h"
#include "ui_forumpostsdialog.h"
#include "mainwindow.h"
#include "perfildialog.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QTextEdit>
#include <QScrollBar>
#include <QEvent>
#include <QMouseEvent>

// ============================================================================
// CONSTRUTOR
// ============================================================================
ForumPostsDialog::ForumPostsDialog(QWidget *parent, int idDuvida, const QString& username)
    : QDialog(parent)
    , ui(new Ui::ForumPostsDialog)
    , duvidaId(idDuvida)
    , loggedInUsername(username)
{
    ui->setupUi(this);
    setWindowTitle("Detalhes da Dúvida");
    resize(1000, 750);

    qDebug() << "=== ForumPostsDialog ABERTO ===" << idDuvida;

    setupDatabase();

    // Configura o layout para as respostas
    if (!ui->containerPerguntas->layout()) {
        respostasLayout = new QVBoxLayout(ui->containerPerguntas);
        respostasLayout->setSpacing(12);
        respostasLayout->setContentsMargins(15, 15, 15, 15);
        respostasLayout->setAlignment(Qt::AlignTop);
        ui->containerPerguntas->setLayout(respostasLayout);
    } else {
        respostasLayout = qobject_cast<QVBoxLayout*>(ui->containerPerguntas->layout());
    }

    // Instala event filter para navegação
    ui->homeButton->installEventFilter(this);
    ui->perfilButton->installEventFilter(this);

    // Conecta botões
    connect(ui->voltarButton, &QPushButton::clicked, this, &ForumPostsDialog::on_voltarButton_clicked);

    // Carrega dados
    carregarDuvida();
    carregarRespostas();
}

// ============================================================================
// DESTRUTOR
// ============================================================================
ForumPostsDialog::~ForumPostsDialog()
{
    delete ui;
}

// ============================================================================
// EVENT FILTER - Para navegação Home e Perfil
// ============================================================================
bool ForumPostsDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        // Botão Home
        if (obj == ui->homeButton) {
            MainWindow *mainWin = new MainWindow();
            mainWin->show();
            this->close();
            return true;
        }

        // Botão Perfil
        if (obj == ui->perfilButton) {
            PerfilDialog *perfilDialog = new PerfilDialog(this, loggedInUsername);
            perfilDialog->exec();
            delete perfilDialog;
            return true;
        }
    }

    return QDialog::eventFilter(obj, event);
}

// ============================================================================
// SETUP DATABASE
// ============================================================================
void ForumPostsDialog::setupDatabase()
{
    dbConnection = QSqlDatabase::database("qt_sql_default_connection");

    if (!dbConnection.isOpen()) {
        qDebug() << "[ForumPostsDialog] ERRO: Banco de dados não está aberto.";
    }
}

// ============================================================================
// GET ID USUARIO
// ============================================================================
int ForumPostsDialog::getIdUsuario(const QString& username)
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
// VERIFICAR SE USUÁRIO JÁ CURTIU A DÚVIDA
// ============================================================================
bool ForumPostsDialog::usuarioJaCurtiuDuvida()
{
    int idUsuario = getIdUsuario(loggedInUsername);

    QSqlQuery query(dbConnection);
    query.prepare("SELECT COUNT(*) FROM Curtidas_Duvidas WHERE id_duvida = ? AND id_usuario = ?");
    query.addBindValue(duvidaId);
    query.addBindValue(idUsuario);

    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }
    return false;
}

// ============================================================================
// VERIFICAR SE USUÁRIO JÁ CURTIU UMA RESPOSTA
// ============================================================================
bool ForumPostsDialog::usuarioJaCurtiuResposta(int idResposta)
{
    int idUsuario = getIdUsuario(loggedInUsername);

    QSqlQuery query(dbConnection);
    query.prepare(
        "SELECT COUNT(*) FROM Curtidas_Respostas "
        "WHERE id_resposta = ? AND id_usuario = ?"
        );
    query.addBindValue(idResposta);
    query.addBindValue(idUsuario);

    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }
    return false;
}

// ============================================================================
// CARREGAR DÚVIDA PRINCIPAL
// ============================================================================
void ForumPostsDialog::carregarDuvida()
{
    QSqlQuery query(dbConnection);
    query.prepare(
        "SELECT d.titulo, d.descricao, d.data_criacao, d.num_curtidas, "
        "u.usuario, u.nome, u.Sobrenome, m.nome as materia "
        "FROM Duvidas_Forum d "
        "JOIN Usuario u ON d.id_usuario = u.id_usuario "
        "JOIN Materias_Forum m ON d.id_materia = m.id_materia "
        "WHERE d.id_duvida = ?"
        );
    query.addBindValue(duvidaId);

    if (!query.exec() || !query.next()) {
        QMessageBox::critical(this, "Erro", "Dúvida não encontrada!");
        close();
        return;
    }

    QString titulo = query.value(0).toString();
    QString descricao = query.value(1).toString();
    QString dataCriacao = query.value(2).toString();
    int numCurtidas = query.value(3).toInt();
    QString nomeAutor = query.value(5).toString() + " " + query.value(6).toString();
    QString materia = query.value(7).toString();

    // Atualiza o título da janela
    ui->labelTitulo->setText(titulo);

    // Cria card da dúvida principal
    QFrame *duvidaFrame = new QFrame();
    duvidaFrame->setFrameShape(QFrame::StyledPanel);
    duvidaFrame->setStyleSheet(
        "QFrame {"
        "   background-color: rgba(66, 55, 56, 0.95);"
        "   border-left: 5px solid #F4B315;"
        "   border-radius: 12px;"
        "   padding: 20px;"
        "   margin-bottom: 20px;"
        "}"
        );

    QVBoxLayout *duvidaLayout = new QVBoxLayout(duvidaFrame);
    duvidaLayout->setSpacing(12);

    // Título
    QLabel *tituloLabel = new QLabel(titulo);
    tituloLabel->setStyleSheet(
        "font-size: 20px; "
        "font-weight: bold; "
        "color: #FFFFFF;"
        );
    tituloLabel->setWordWrap(true);
    duvidaLayout->addWidget(tituloLabel);

    // Info (autor, data, matéria)
    QHBoxLayout *infoLayout = new QHBoxLayout();

    QLabel *autorLabel = new QLabel("Autor: " + nomeAutor);
    autorLabel->setStyleSheet("font-size: 12px; color: #D3AF35;");

    QLabel *dataLabel = new QLabel("Data: " + dataCriacao.left(10));
    dataLabel->setStyleSheet("font-size: 12px; color: #D3AF35;");

    QLabel *materiaLabel = new QLabel("Matéria: " + materia);
    materiaLabel->setStyleSheet("font-size: 12px; color: #D3AF35;");

    infoLayout->addWidget(autorLabel);
    infoLayout->addWidget(dataLabel);
    infoLayout->addWidget(materiaLabel);
    infoLayout->addStretch();

    duvidaLayout->addLayout(infoLayout);

    // Descrição
    QLabel *descLabel = new QLabel(descricao);
    descLabel->setWordWrap(true);
    descLabel->setStyleSheet(
        "font-size: 14px; "
        "color: #F4B315; "
        "margin-top: 10px; "
        "padding: 15px; "
        "background-color: rgba(26, 22, 26, 0.8); "
        "border-radius: 8px;"
        );
    duvidaLayout->addWidget(descLabel);

    // Botão de curtir
    QPushButton *curtirBtn = new QPushButton();
    bool jaCurtiu = usuarioJaCurtiuDuvida();

    if (jaCurtiu) {
        curtirBtn->setText(QString("♥ %1 curtida(s) - Você curtiu").arg(numCurtidas));
        curtirBtn->setEnabled(false);
        curtirBtn->setStyleSheet(
            "QPushButton {"
            "   background-color: rgba(231, 76, 60, 0.3);"
            "   color: #E74C3C;"
            "   border: 2px solid #E74C3C;"
            "   border-radius: 8px;"
            "   padding: 10px 20px;"
            "   font-weight: bold;"
            "   font-size: 13px;"
            "}"
            );
    } else {
        curtirBtn->setText(QString("♡ %1 curtida(s) - Curtir").arg(numCurtidas));
        curtirBtn->setStyleSheet(
            "QPushButton {"
            "   background-color: transparent;"
            "   color: #F4B315;"
            "   border: 2px solid #F4B315;"
            "   border-radius: 8px;"
            "   padding: 10px 20px;"
            "   font-weight: bold;"
            "   font-size: 13px;"
            "}"
            "QPushButton:hover {"
            "   background-color: rgba(244, 179, 21, 0.2);"
            "}"
            );
    }

    curtirBtn->setMinimumHeight(40);
    connect(curtirBtn, &QPushButton::clicked, this, &ForumPostsDialog::onCurtirDuvida);
    duvidaLayout->addWidget(curtirBtn);

    // Adiciona o frame ao layout principal
    respostasLayout->addWidget(duvidaFrame);

    // Botão "Responder"
    QPushButton *responderBtn = new QPushButton("+ Adicionar Resposta");
    responderBtn->setMinimumHeight(50);
    responderBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #27AE60;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 10px;"
        "   padding: 15px;"
        "   font-size: 16px;"
        "   font-weight: bold;"
        "   margin: 10px 0;"
        "}"
        "QPushButton:hover {"
        "   background-color: #229954;"
        "}"
        );
    connect(responderBtn, &QPushButton::clicked, this, &ForumPostsDialog::on_responderButton_clicked);
    respostasLayout->addWidget(responderBtn);

    // Linha separadora
    QFrame *line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("background-color: #F4B315; margin: 15px 0;");
    line->setLineWidth(2);
    respostasLayout->addWidget(line);

    // Título da seção de respostas
    QLabel *respostasTitle = new QLabel("Respostas");
    respostasTitle->setStyleSheet(
        "font-size: 18px; "
        "font-weight: bold; "
        "color: #FFFFFF; "
        "margin-top: 10px;"
        );
    respostasLayout->addWidget(respostasTitle);
}

// ============================================================================
// CARREGAR RESPOSTAS
// ============================================================================
void ForumPostsDialog::carregarRespostas()
{
    QSqlQuery query(dbConnection);
    query.prepare(
        "SELECT r.id_resposta, r.resposta, r.data_resposta, r.num_curtidas, "
        "r.melhor_resposta, u.usuario, u.nome, u.Sobrenome "
        "FROM Respostas_Forum r "
        "JOIN Usuario u ON r.id_usuario = u.id_usuario "
        "WHERE r.id_duvida = ? "
        "ORDER BY r.melhor_resposta DESC, r.num_curtidas DESC, r.data_resposta ASC"
        );
    query.addBindValue(duvidaId);

    if (!query.exec()) {
        qDebug() << "Erro ao carregar respostas:" << query.lastError().text();
        return;
    }

    int respostaCount = 0;

    while (query.next()) {
        int idResposta = query.value(0).toInt();
        QString resposta = query.value(1).toString();
        QString dataResposta = query.value(2).toString();
        int numCurtidas = query.value(3).toInt();
        bool melhorResposta = query.value(4).toInt() == 1;
        QString nomeAutor = query.value(6).toString() + " " + query.value(7).toString();

        // Frame da resposta
        QFrame *respostaFrame = new QFrame();
        respostaFrame->setFrameShape(QFrame::StyledPanel);

        QString corBorda = melhorResposta ? "#27AE60" : "#8E6915";
        QString corFundo = melhorResposta ? "rgba(39, 174, 96, 0.15)" : "rgba(42, 36, 38, 0.95)";

        respostaFrame->setStyleSheet(
            QString("QFrame {"
                    "   background-color: %1;"
                    "   border-left: 4px solid %2;"
                    "   border-radius: 10px;"
                    "   padding: 15px;"
                    "   margin: 8px 0;"
                    "}").arg(corFundo).arg(corBorda)
            );

        QVBoxLayout *respostaLayout = new QVBoxLayout(respostaFrame);
        respostaLayout->setSpacing(10);

        // Cabeçalho
        QHBoxLayout *headerLayout = new QHBoxLayout();

        QLabel *autorLabel = new QLabel("Por: " + nomeAutor);
        autorLabel->setStyleSheet("font-size: 13px; font-weight: bold; color: #FFFFFF;");

        QLabel *dataLabel = new QLabel(dataResposta.left(10));
        dataLabel->setStyleSheet("font-size: 11px; color: #8E6915;");

        headerLayout->addWidget(autorLabel);
        headerLayout->addWidget(dataLabel);

        if (melhorResposta) {
            QLabel *melhorLabel = new QLabel("✓ MELHOR RESPOSTA");
            melhorLabel->setStyleSheet(
                "font-size: 11px; "
                "color: #27AE60; "
                "font-weight: bold; "
                "background-color: rgba(39, 174, 96, 0.2); "
                "padding: 4px 8px; "
                "border-radius: 4px;"
                );
            headerLayout->addWidget(melhorLabel);
        }

        headerLayout->addStretch();
        respostaLayout->addLayout(headerLayout);

        // Texto da resposta
        QLabel *textoLabel = new QLabel(resposta);
        textoLabel->setWordWrap(true);
        textoLabel->setStyleSheet(
            "font-size: 13px; "
            "color: #F4B315; "
            "margin-top: 8px; "
            "padding: 12px; "
            "background-color: rgba(26, 22, 26, 0.6); "
            "border-radius: 6px;"
            );
        respostaLayout->addWidget(textoLabel);

        // Botão de curtir resposta
        QPushButton *curtirBtn = new QPushButton();
        bool jaCurtiu = usuarioJaCurtiuResposta(idResposta);

        if (jaCurtiu) {
            curtirBtn->setText(QString("♥ %1").arg(numCurtidas));
            curtirBtn->setEnabled(false);
            curtirBtn->setStyleSheet(
                "QPushButton {"
                "   background-color: transparent;"
                "   color: #E74C3C;"
                "   border: 1px solid #E74C3C;"
                "   border-radius: 6px;"
                "   padding: 6px 15px;"
                "   font-size: 12px;"
                "   font-weight: bold;"
                "}"
                );
        } else {
            curtirBtn->setText(QString("♡ %1").arg(numCurtidas));
            curtirBtn->setStyleSheet(
                "QPushButton {"
                "   background-color: transparent;"
                "   color: #D3AF35;"
                "   border: 1px solid #D3AF35;"
                "   border-radius: 6px;"
                "   padding: 6px 15px;"
                "   font-size: 12px;"
                "   font-weight: bold;"
                "}"
                "QPushButton:hover {"
                "   background-color: rgba(211, 175, 53, 0.2);"
                "}"
                );
        }

        curtirBtn->setMinimumHeight(30);
        connect(curtirBtn, &QPushButton::clicked, [this, idResposta]() {
            onCurtirResposta(idResposta);
        });

        respostaLayout->addWidget(curtirBtn, 0, Qt::AlignLeft);

        respostasLayout->addWidget(respostaFrame);
        respostaCount++;
    }

    if (respostaCount == 0) {
        QLabel *emptyLabel = new QLabel("Nenhuma resposta ainda.\nSeja o primeiro a responder!");
        emptyLabel->setAlignment(Qt::AlignCenter);
        emptyLabel->setStyleSheet(
            "color: #8E6915; "
            "font-size: 14px; "
            "padding: 30px; "
            "background-color: rgba(42, 36, 38, 0.7); "
            "border-radius: 10px; "
            "margin: 20px;"
            );
        respostasLayout->addWidget(emptyLabel);
    }

    respostasLayout->addStretch();
}

// ============================================================================
// ADICIONAR RESPOSTA
// ============================================================================
void ForumPostsDialog::on_responderButton_clicked()
{
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Adicionar Resposta");
    dialog->resize(650, 450);
    dialog->setStyleSheet(
        "QDialog { background-color: #1A161A; }"
        "QLabel { color: #F4B315; font-weight: bold; }"
        "QTextEdit {"
        "   background-color: #423738; color: #F4B315;"
        "   border: 2px solid #8E6915; border-radius: 8px;"
        "   padding: 10px; font-size: 14px;"
        "}"
        "QTextEdit:focus { border-color: #F4B315; }"
        );

    QVBoxLayout *layout = new QVBoxLayout(dialog);
    layout->setSpacing(15);

    QLabel *label = new QLabel("Escreva sua resposta:");
    label->setStyleSheet("font-size: 16px;");

    QTextEdit *respostaEdit = new QTextEdit();
    respostaEdit->setPlaceholderText("Digite sua resposta aqui...");
    respostaEdit->setMinimumHeight(220);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *enviarBtn = new QPushButton("Enviar Resposta");
    QPushButton *cancelarBtn = new QPushButton("Cancelar");

    enviarBtn->setMinimumHeight(45);
    cancelarBtn->setMinimumHeight(45);

    enviarBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #27AE60; color: white;"
        "   padding: 10px 20px; border-radius: 8px;"
        "   font-weight: bold; font-size: 14px; border: none;"
        "}"
        "QPushButton:hover { background-color: #229954; }"
        );

    cancelarBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #423738; color: #F4B315;"
        "   border: 2px solid #F4B315; padding: 10px 20px;"
        "   border-radius: 8px; font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: #524447; }"
        );

    btnLayout->addWidget(cancelarBtn);
    btnLayout->addWidget(enviarBtn);

    layout->addWidget(label);
    layout->addWidget(respostaEdit);
    layout->addStretch();
    layout->addLayout(btnLayout);

    connect(cancelarBtn, &QPushButton::clicked, dialog, &QDialog::reject);

    connect(enviarBtn, &QPushButton::clicked, [=]() {
        QString respostaTexto = respostaEdit->toPlainText().trimmed();

        if (respostaTexto.isEmpty()) {
            QMessageBox::warning(dialog, "Campo vazio",
                                 "Por favor, escreva sua resposta!");
            return;
        }

        int idUsuario = getIdUsuario(loggedInUsername);

        QSqlQuery insertQuery(dbConnection);
        insertQuery.prepare(
            "INSERT INTO Respostas_Forum (id_duvida, id_usuario, resposta) "
            "VALUES (?, ?, ?)"
            );
        insertQuery.addBindValue(duvidaId);
        insertQuery.addBindValue(idUsuario);
        insertQuery.addBindValue(respostaTexto);

        if (insertQuery.exec()) {
            // Atualiza status da dúvida para "respondida"
            QSqlQuery updateStatus(dbConnection);
            updateStatus.prepare("UPDATE Duvidas_Forum SET status = 'respondida' WHERE id_duvida = ?");
            updateStatus.addBindValue(duvidaId);
            updateStatus.exec();

            QMessageBox::information(dialog, "Sucesso",
                                     "Resposta enviada com sucesso!");
            dialog->accept();

            // Recarrega as respostas
            QLayoutItem *item;
            while ((item = respostasLayout->takeAt(0)) != nullptr) {
                delete item->widget();
                delete item;
            }

            carregarDuvida();
            carregarRespostas();
        } else {
            QMessageBox::critical(dialog, "Erro",
                                  "Erro ao enviar resposta: " + insertQuery.lastError().text());
        }
    });

    dialog->exec();
    delete dialog;
}

// ============================================================================
// CURTIR DÚVIDA
// ============================================================================
void ForumPostsDialog::onCurtirDuvida()
{
    int idUsuario = getIdUsuario(loggedInUsername);

    // Adiciona curtida
    QSqlQuery insertLike(dbConnection);
    insertLike.prepare("INSERT INTO Curtidas_Duvidas (id_duvida, id_usuario) VALUES (?, ?)");
    insertLike.addBindValue(duvidaId);
    insertLike.addBindValue(idUsuario);

    if (insertLike.exec()) {
        // Atualiza contador
        QSqlQuery updateLikes(dbConnection);
        updateLikes.prepare("UPDATE Duvidas_Forum SET num_curtidas = num_curtidas + 1 WHERE id_duvida = ?");
        updateLikes.addBindValue(duvidaId);
        updateLikes.exec();

        // Recarrega a página
        QLayoutItem *item;
        while ((item = respostasLayout->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }

        carregarDuvida();
        carregarRespostas();
    } else {
        qDebug() << "Erro ao curtir:" << insertLike.lastError().text();
    }
}

// ============================================================================
// CURTIR RESPOSTA
// ============================================================================
void ForumPostsDialog::onCurtirResposta(int idResposta)
{
    int idUsuario = getIdUsuario(loggedInUsername);

    QSqlQuery insertLike(dbConnection);
    insertLike.prepare("INSERT INTO Curtidas_Respostas (id_resposta, id_usuario) VALUES (?, ?)");
    insertLike.addBindValue(idResposta);
    insertLike.addBindValue(idUsuario);

    if (insertLike.exec()) {
        // Atualiza contador
        QSqlQuery updateLikes(dbConnection);
        updateLikes.prepare("UPDATE Respostas_Forum SET num_curtidas = num_curtidas + 1 WHERE id_resposta = ?");
        updateLikes.addBindValue(idResposta);
        updateLikes.exec();

        // Recarrega
        QLayoutItem *item;
        while ((item = respostasLayout->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }

        carregarDuvida();
        carregarRespostas();
    }
}

// ============================================================================
// VOLTAR
// ============================================================================
void ForumPostsDialog::on_voltarButton_clicked()
{
    this->close();
}
