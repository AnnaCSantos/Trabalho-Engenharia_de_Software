#include <QLineEdit>
#include "postdetailsdialog.h"
#include "ui_postdetailsdialog.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QDebug>
#include <QCoreApplication>

PostDetailsDialog::PostDetailsDialog(QWidget *parent, int idPostagem, const QString& username)
    : QDialog(parent)
    , ui(new Ui::PostDetailsDialog)
    , postagemId(idPostagem)
    , loggedInUsername(username)
{
    ui->setupUi(this);
    setWindowTitle("Detalhes da Postagem");
    resize(900, 700);

    dbConnection = QSqlDatabase::database("qt_sql_default_connection");

    carregarPostagem();
}

PostDetailsDialog::~PostDetailsDialog()
{
    delete ui;
}

int PostDetailsDialog::getIdUsuario(const QString& username)
{
    QSqlQuery query(dbConnection);
    query.prepare("SELECT id_usuario FROM Usuario WHERE usuario = ?");
    query.addBindValue(username);

    if(query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    return 1;
}

bool PostDetailsDialog::usuarioJaDeuLike(int idPostagem)
{
    int idUsuario = getIdUsuario(loggedInUsername);

    QSqlQuery query(dbConnection);
    query.prepare("SELECT COUNT(*) FROM Like_Postagem WHERE id_postagem = ? AND id_usuario = ?");
    query.addBindValue(idPostagem);
    query.addBindValue(idUsuario);

    if(query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }
    return false;
}

bool PostDetailsDialog::usuarioJaDeuLikeResposta(int idResposta)
{
    int idUsuario = getIdUsuario(loggedInUsername);

    QSqlQuery query(dbConnection);
    query.prepare("SELECT COUNT(*) FROM Like_Resposta WHERE id_resposta = ? AND id_usuario = ?");
    query.addBindValue(idResposta);
    query.addBindValue(idUsuario);

    if(query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }
    return false;
}

void PostDetailsDialog::carregarPostagem()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Consulta os dados da postagem
    QSqlQuery query(dbConnection);
    query.prepare("SELECT p.titulo, p.conteudo, p.data_postagem, p.num_likes, "
                  "u.usuario, u.nome, u.Sobrenome "
                  "FROM Postagem p "
                  "JOIN Usuario u ON p.id_usuario = u.id_usuario "
                  "WHERE p.id_postagem = ?");
    query.addBindValue(postagemId);

    if(query.exec() && query.next()) {
        QString titulo = query.value("titulo").toString();
        QString conteudo = query.value("conteudo").toString();
        QString dataPost = query.value("data_postagem").toString();
        int numLikes = query.value("num_likes").toInt();
        QString nomeCompleto = query.value("nome").toString() + " " + query.value("Sobrenome").toString();

        // Frame da postagem principal
        QFrame *postFrame = new QFrame();
        postFrame->setFrameShape(QFrame::StyledPanel);
        postFrame->setStyleSheet(
            "QFrame {"
            "   background-color: #FFFFFF;"
            "   border: 2px solid #4A90E2;"
            "   border-radius: 10px;"
            "   padding: 20px;"
            "}"
            );

        QVBoxLayout *postLayout = new QVBoxLayout(postFrame);

        // Cabeçalho
        QHBoxLayout *headerLayout = new QHBoxLayout();
        QLabel *autorLabel = new QLabel("👤 " + nomeCompleto);
        autorLabel->setStyleSheet("font-weight: bold; color: #4A90E2; font-size: 14px;");

        QLabel *dataLabel = new QLabel(dataPost);
        dataLabel->setStyleSheet("color: #95A5A6; font-size: 12px;");

        headerLayout->addWidget(autorLabel);
        headerLayout->addStretch();
        headerLayout->addWidget(dataLabel);

        // Título
        QLabel *tituloLabel = new QLabel(titulo);
        tituloLabel->setStyleSheet(
            "font-size: 22px;"
            "font-weight: bold;"
            "color: #2C3E50;"
            "margin-top: 10px;"
            "margin-bottom: 10px;"
            );
        tituloLabel->setWordWrap(true);

        // Conteúdo
        QLabel *conteudoLabel = new QLabel(conteudo);
        conteudoLabel->setWordWrap(true);
        conteudoLabel->setStyleSheet(
            "font-size: 14px;"
            "color: #34495E;"
            "line-height: 1.6;"
            "margin-bottom: 15px;"
            );

        // Botão de like
        QPushButton *likeBtn = new QPushButton();
        bool jaDeuLike = usuarioJaDeuLike(postagemId);

        if(jaDeuLike) {
            likeBtn->setText("❤️ " + QString::number(numLikes) + " (Você curtiu)");
            likeBtn->setEnabled(false);
        } else {
            likeBtn->setText("🤍 " + QString::number(numLikes) + " - Curtir");
        }

        likeBtn->setStyleSheet(
            "QPushButton {"
            "   background-color: #E8F4F8;"
            "   color: #4A90E2;"
            "   border: 1px solid #4A90E2;"
            "   border-radius: 5px;"
            "   padding: 8px 20px;"
            "   font-weight: bold;"
            "}"
            "QPushButton:hover {"
            "   background-color: #D0E9F5;"
            "}"
            "QPushButton:disabled {"
            "   background-color: #FFEBEE;"
            "   color: #E74C3C;"
            "   border: 1px solid #E74C3C;"
            "}"
            );

        connect(likeBtn, &QPushButton::clicked, this, &PostDetailsDialog::darLikePostagem);

        postLayout->addLayout(headerLayout);
        postLayout->addWidget(tituloLabel);
        postLayout->addWidget(conteudoLabel);
        postLayout->addWidget(likeBtn);

        mainLayout->addWidget(postFrame);

        // Seção de respostas
        QLabel *respostasTitle = new QLabel("💬 Respostas");
        respostasTitle->setStyleSheet(
            "font-size: 18px;"
            "font-weight: bold;"
            "color: #2C3E50;"
            "margin-top: 20px;"
            "margin-bottom: 10px;"
            );
        mainLayout->addWidget(respostasTitle);

        // Scroll area para respostas
        QScrollArea *scrollArea = new QScrollArea();
        scrollArea->setWidgetResizable(true);
        scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        QWidget *containerWidget = new QWidget();
        respostasLayout = new QVBoxLayout(containerWidget);
        respostasLayout->setSpacing(10);

        carregarRespostas();

        respostasLayout->addStretch();
        containerWidget->setLayout(respostasLayout);
        scrollArea->setWidget(containerWidget);

        mainLayout->addWidget(scrollArea);

        // Botão para adicionar resposta
        QPushButton *addRespostaBtn = new QPushButton("+ Adicionar Resposta");
        addRespostaBtn->setStyleSheet(
            "QPushButton {"
            "   background-color: #27AE60;"
            "   color: white;"
            "   border: none;"
            "   border-radius: 5px;"
            "   padding: 10px 20px;"
            "   font-weight: bold;"
            "   font-size: 14px;"
            "}"
            "QPushButton:hover {"
            "   background-color: #229954;"
            "}"
            );
        connect(addRespostaBtn, &QPushButton::clicked, this, &PostDetailsDialog::adicionarResposta);

        mainLayout->addWidget(addRespostaBtn);

    } else {
        qDebug() << "Erro ao carregar postagem:" << query.lastError().text();
        QLabel *errorLabel = new QLabel("Erro ao carregar postagem.");
        mainLayout->addWidget(errorLabel);
    }

    setLayout(mainLayout);
}

void PostDetailsDialog::carregarRespostas()
{
    QSqlQuery query(dbConnection);
    query.prepare("SELECT r.id_resposta, r.conteudo, r.data_resposta, r.num_likes, "
                  "u.usuario, u.nome, u.Sobrenome "
                  "FROM Resposta r "
                  "JOIN Usuario u ON r.id_usuario = u.id_usuario "
                  "WHERE r.id_postagem = ? "
                  "ORDER BY r.data_resposta ASC");
    query.addBindValue(postagemId);

    if(query.exec()) {
        int respostaCount = 0;

        while(query.next()) {
            int idResposta = query.value("id_resposta").toInt();
            QString conteudo = query.value("conteudo").toString();
            QString dataResp = query.value("data_resposta").toString();
            int numLikes = query.value("num_likes").toInt();
            QString nomeCompleto = query.value("nome").toString() + " " + query.value("Sobrenome").toString();

            // Frame da resposta
            QFrame *respostaFrame = new QFrame();
            respostaFrame->setFrameShape(QFrame::StyledPanel);
            respostaFrame->setStyleSheet(
                "QFrame {"
                "   background-color: #F9F9F9;"
                "   border: 1px solid #E0E0E0;"
                "   border-radius: 8px;"
                "   padding: 12px;"
                "}"
                );

            QVBoxLayout *respostaLayout = new QVBoxLayout(respostaFrame);

            // Cabeçalho da resposta
            QHBoxLayout *headerLayout = new QHBoxLayout();
            QLabel *autorLabel = new QLabel("👤 " + nomeCompleto);
            autorLabel->setStyleSheet("font-weight: bold; color: #7F8C8D; font-size: 12px;");

            QLabel *dataLabel = new QLabel(dataResp);
            dataLabel->setStyleSheet("color: #95A5A6; font-size: 11px;");

            headerLayout->addWidget(autorLabel);
            headerLayout->addStretch();
            headerLayout->addWidget(dataLabel);

            // Conteúdo da resposta
            QLabel *conteudoLabel = new QLabel(conteudo);
            conteudoLabel->setWordWrap(true);
            conteudoLabel->setStyleSheet(
                "font-size: 13px;"
                "color: #2C3E50;"
                "margin-top: 5px;"
                "margin-bottom: 8px;"
                );

            // Botão de like na resposta
            QPushButton *likeBtn = new QPushButton();
            bool jaDeuLike = usuarioJaDeuLikeResposta(idResposta);

            if(jaDeuLike) {
                likeBtn->setText("❤️ " + QString::number(numLikes));
                likeBtn->setEnabled(false);
            } else {
                likeBtn->setText("🤍 " + QString::number(numLikes));
            }

            likeBtn->setStyleSheet(
                "QPushButton {"
                "   background-color: transparent;"
                "   color: #7F8C8D;"
                "   border: 1px solid #BDC3C7;"
                "   border-radius: 4px;"
                "   padding: 4px 12px;"
                "   font-size: 11px;"
                "}"
                "QPushButton:hover {"
                "   background-color: #ECF0F1;"
                "}"
                "QPushButton:disabled {"
                "   color: #E74C3C;"
                "   border: 1px solid #E74C3C;"
                "}"
                );

            connect(likeBtn, &QPushButton::clicked, [this, idResposta]() {
                darLikeResposta(idResposta);
            });

            respostaLayout->addLayout(headerLayout);
            respostaLayout->addWidget(conteudoLabel);
            respostaLayout->addWidget(likeBtn, 0, Qt::AlignLeft);

            respostasLayout->addWidget(respostaFrame);
            respostaCount++;
        }

        if(respostaCount == 0) {
            QLabel *emptyLabel = new QLabel("Nenhuma resposta ainda. Seja o primeiro a responder!");
            emptyLabel->setAlignment(Qt::AlignCenter);
            emptyLabel->setStyleSheet("color: #95A5A6; font-size: 13px; margin: 30px;");
            respostasLayout->addWidget(emptyLabel);
        }
    }
}

void PostDetailsDialog::darLikePostagem()
{
    int idUsuario = getIdUsuario(loggedInUsername);

    // Adiciona o like
    QSqlQuery insertLike(dbConnection);
    insertLike.prepare("INSERT INTO Like_Postagem (id_postagem, id_usuario) VALUES (?, ?)");
    insertLike.addBindValue(postagemId);
    insertLike.addBindValue(idUsuario);

    if(insertLike.exec()) {
        // Atualiza o contador de likes
        QSqlQuery updateLikes(dbConnection);
        updateLikes.prepare("UPDATE Postagem SET num_likes = num_likes + 1 WHERE id_postagem = ?");
        updateLikes.addBindValue(postagemId);
        updateLikes.exec();

        // Recarrega a página
        QLayoutItem *item;
        QLayout *layout = this->layout();
        while((item = layout->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
        delete layout;

        carregarPostagem();
    } else {
        qDebug() << "Erro ao dar like:" << insertLike.lastError().text();
    }
}

void PostDetailsDialog::darLikeResposta(int idResposta)
{
    int idUsuario = getIdUsuario(loggedInUsername);

    // Adiciona o like
    QSqlQuery insertLike(dbConnection);
    insertLike.prepare("INSERT INTO Like_Resposta (id_resposta, id_usuario) VALUES (?, ?)");
    insertLike.addBindValue(idResposta);
    insertLike.addBindValue(idUsuario);

    if(insertLike.exec()) {
        // Atualiza o contador
        QSqlQuery updateLikes(dbConnection);
        updateLikes.prepare("UPDATE Resposta SET num_likes = num_likes + 1 WHERE id_resposta = ?");
        updateLikes.addBindValue(idResposta);
        updateLikes.exec();

        // Recarrega
        QLayoutItem *item;
        while((item = respostasLayout->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }

        carregarRespostas();
    }
}

void PostDetailsDialog::adicionarResposta()
{
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Adicionar Resposta");
    dialog->resize(500, 300);

    QVBoxLayout *layout = new QVBoxLayout(dialog);

    QLabel *label = new QLabel("Sua resposta:");
    QTextEdit *respostaEdit = new QTextEdit();
    respostaEdit->setPlaceholderText("Digite sua resposta...");

    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *enviarBtn = new QPushButton("Enviar");
    QPushButton *cancelarBtn = new QPushButton("Cancelar");

    enviarBtn->setStyleSheet(
        "background-color: #27AE60; color: white; padding: 8px 20px; border-radius: 5px; font-weight: bold;"
        );
    cancelarBtn->setStyleSheet(
        "background-color: #E74C3C; color: white; padding: 8px 20px; border-radius: 5px; font-weight: bold;"
        );

    btnLayout->addWidget(cancelarBtn);
    btnLayout->addWidget(enviarBtn);

    layout->addWidget(label);
    layout->addWidget(respostaEdit);
    layout->addLayout(btnLayout);

    connect(cancelarBtn, &QPushButton::clicked, dialog, &QDialog::reject);
    connect(enviarBtn, &QPushButton::clicked, [=]() {
        QString conteudo = respostaEdit->toPlainText().trimmed();

        if(conteudo.isEmpty()) {
            QMessageBox::warning(dialog, "Campo vazio", "Por favor, escreva sua resposta!");
            return;
        }

        int idUsuario = getIdUsuario(loggedInUsername);

        QSqlQuery insertQuery(dbConnection);
        insertQuery.prepare("INSERT INTO Resposta (id_postagem, id_usuario, conteudo) "
                            "VALUES (?, ?, ?)");
        insertQuery.addBindValue(postagemId);
        insertQuery.addBindValue(idUsuario);
        insertQuery.addBindValue(conteudo);

        if(insertQuery.exec()) {
            QMessageBox::information(dialog, "Sucesso", "Resposta enviada com sucesso!");
            dialog->accept();

            // Recarrega as respostas
            QLayoutItem *item;
            while((item = respostasLayout->takeAt(0)) != nullptr) {
                delete item->widget();
                delete item;
            }
            carregarRespostas();
        } else {
            qDebug() << "Erro ao enviar resposta:" << insertQuery.lastError().text();
            QMessageBox::critical(dialog, "Erro", "Não foi possível enviar a resposta.");
        }
    });

    dialog->exec();
    delete dialog;
}
