#include "forumdialog.h"
#include "ui_forumdialog.h"
#include "forumpostsdialog.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QPushButton>
#include <QLayout>

ForumDialog::ForumDialog(QWidget *parent, const QString& username) :
    QDialog(parent),
    ui(new Ui::ForumDialog), // Cria a interface gráfica
    loggedInUsername(username) // Armazena o nome de usuário
{
    ui->setupUi(this);
    if (ui->containerBotoes)
    {
        // Cria o objeto de layout na memória
        forumsLayout = new QVBoxLayout();

        // Aplica o layout ao widget 'containerBotoes'
        ui->containerBotoes->setLayout(forumsLayout);
    } else {
        qDebug() << "ERRO FATAL: O widget 'containerBotoes' não foi encontrado no forumdialog.ui!";
        qDebug() << "Verifique o 'objectName' do QWidget dentro do QScrollArea.";
    }
    setupDatabase();
    carregarForuns();
}


ForumDialog::~ForumDialog()
{
    delete ui; // Libera a memória da interface gráfica
}


void ForumDialog::setupDatabase()
{
    dbConnection = QSqlDatabase::database();

    if (!dbConnection.isOpen()) {
        qDebug() << "Erro (ForumDialog): O banco de dados não está aberto.";
    } else {
        qDebug() << "Conexão com o banco estabelecida para o ForumDialog.";
    }
}

void ForumDialog::carregarForuns()
{
    // Verifica se o layout foi criado com sucesso no construtor
    if (!forumsLayout) {
        qDebug() << "ERRO: O layout dos fóruns (forumsLayout) não foi inicializado.";
        return;
    }
    QSqlQuery query(dbConnection);
    query.prepare("SELECT * FROM Forum ORDER BY titulo ASC");

    // Executa a consulta e verifica se deu erro
    if (!query.exec()) {
        qDebug() << "Erro ao carregar fóruns do banco:" << query.lastError().text();
        return;
    }

    //  Roda uma vez para cada fórum encontrado no banco
    while (query.next()) {
        // Pega os dados da linha atual
        int id = query.value("id_forum").toInt(); // Pega o ID (coluna 'id')
        QString nome = query.value("titulo").toString(); // Pega o nome (coluna 'nome_forum')

        // Cria o botão
        QPushButton *btn = new QPushButton(nome); // Texto do botão será o nome do fórum

        // Estiliza o botão
        btn->setStyleSheet(
            "QPushButton { background-color: #F4B315; color: white; border-radius: 5px; padding: 10px; font-wight:bold; }"
            "QPushButton:hover { background-color: #E59312; }"
            );
        btn->setCursor(Qt::PointingHandCursor); // Muda o cursor

        // onecta o botão ao slot 'abrirForum'
        // Isso resolve o erro 'undefined reference to abrirForum'
        connect(btn, &QPushButton::clicked, [this, id, nome]() {
            abrirForum(id, nome);
        });

        // 7. Adiciona o botão ao layout (ele aparecerá na tela)
        forumsLayout->addWidget(btn);
    }
}

void ForumDialog::abrirForum(int idForum, const QString& tituloForum)
{
    qDebug() << "Abrindo fórum:" << tituloForum << "(ID:" << idForum << ")";

    this->hide(); // Esconde a janela atual (ForumDialog)

    // Cria e abre a nova janela de Posts (ForumPostsDialog)
    ForumPostsDialog paginaPosts(this, idForum);
    paginaPosts.exec(); // 'exec()' pausa o código aqui até a janela fechar

    // Quando a janela de posts fechar, mostra a janela de fóruns novamente
    this->show();
}
