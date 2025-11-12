#include "forumpostsdialog.h"
#include "ui_forumpostsdialog.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QPushButton>
#include <QLayout>
#include <QVBoxLayout>

ForumPostsDialog::ForumPostsDialog(QWidget *parent, int forumId) :
    QDialog(parent),
    ui(new Ui::ForumPostsDialog),
    m_forumId(forumId)
{
    ui->setupUi(this);

    setWindowTitle("Posts do Fórum");
    ui->labelTitulo->setText("Posts do Fórum");

    // Garante que o 'containerPerguntas' tenha um layout
    if (!ui->containerPerguntas->layout()) {
        QVBoxLayout *layout = new QVBoxLayout(ui->containerPerguntas);
        ui->containerPerguntas->setLayout(layout);
        qDebug() << "[ForumPostsDialog] Layout vertical criado via C++";
    }

    // Conecta o 'voltarButton'
    //    connect(ui->voltarButton, &QPushButton::clicked, this, &ForumPostsDialog::on_voltarButton_clicked);

    setupDatabase();
    carregarPosts();
}

ForumPostsDialog::~ForumPostsDialog()
{
    delete ui;
}

void ForumPostsDialog::setupDatabase()
{
    // Usa a conexão de banco de dados que já está aberta
    dbConnection = QSqlDatabase::database();
    if (!dbConnection.isOpen()) {
        qDebug() << "[ForumPostsDialog] ERRO: Banco de dados não está aberto.";
    }
}

void ForumPostsDialog::carregarPosts()
{
    // Pega o layout do 'containerPerguntas'
    QLayout *layout = ui->containerPerguntas->layout();
    if (!layout) {
        qDebug() << "ERRO FATAL: 'containerPerguntas' não tem layout!";
        return;
    }

    // Limpa o layout de posts antigos
    QLayoutItem *item;
    while ((item = layout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    QSqlQuery query(dbConnection);
    QString queryString = QString(
                              "SELECT "
                              "    P.id_postagem, "
                              "    P.titulo, "
                              "    P.conteudo "
                              "FROM "
                              "    Postagem P "
                              "WHERE "
                              "    P.id_forum = %1 "
                              "ORDER BY "
                              "    P.id_postagem DESC" // Ordenamos pelo ID (mais novo primeiro)
                              ).arg(m_forumId);

    if (!query.exec(queryString)) {
        qDebug() << "Erro ao carregar posts:" << query.lastError().text();
        return;
    }

    // Loop: Cria um "botão" para cada post encontrado
    while (query.next()) {
        int id = query.value("id_postagem").toInt();
        QString titulo = query.value("titulo").toString();
        QString conteudo = query.value("conteudo").toString();

        // Trunca o conteúdo se for muito longo
        if (conteudo.length() > 100) {
            conteudo = conteudo.left(100) + "...";
        }

        // Formata o texto para Título e Conteúdo (Título em negrito)
        QString itemTexto = QString("<b>%1</b>\n%2").arg(titulo).arg(conteudo);
        //QString itemTexto = QString("%l\n%2").arg(titulo).arg(conteudo);

        QPushButton *btnPost = new QPushButton(itemTexto);

        // Estiliza o botão (QSS)
        btnPost->setStyleSheet(
            "QPushButton {"
            "background-color: #F4B315;"
            "color: white;"
            "font-weight:bold;"
            "border: 1px solid #555;"
            "border-radius: 10px;"
            "padding: 15px;"
            "text-align: left;"
            "}"
            "QPushButton:hover { background-color: #E59312; }"
            );
        btnPost->setCursor(Qt::PointingHandCursor);
        btnPost->setMinimumHeight(70);

        // Conecta o clique do botão ao slot
        connect(btnPost, &QPushButton::clicked, [this, id, titulo]() {
            on_postClicked(id, titulo);
        });
        // Adiciona o botão ao layout
        layout->addWidget(btnPost);
    }
}

// Slot que é chamado quando um post é clicado
void ForumPostsDialog::on_postClicked(int postId, const QString &titulo)
{
    qDebug() << "Clicou no post:" << titulo << "(ID:" << postId << ")";

    // Aqui seria aberta as novas páginas
    // RespostasDialog *respostas = new RespostasDialog(this, postId);
    // respostas->exec();
}

// Slot para o botão 'voltarButton'
/*void ForumPostsDialog::on_voltarButton_clicked()
{
    qDebug() << "Botão Voltar clicado, fechando PostsDialog.";
    close(); // Fecha esta janela
}
*/
