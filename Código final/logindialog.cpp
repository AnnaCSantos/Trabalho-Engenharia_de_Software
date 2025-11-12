#include "logindialog.h"
#include "ui_logindialog.h"
#include "criarconta.h"
#include <QMessageBox>
#include <QLineEdit>
#include <QPushButton>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>
#include <QCoreApplication>
#include <QFileInfo>
#include <QDir>
#include <QCryptographicHash>  // Biblioteca responsável pelo Hash da Senha

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    setWindowTitle("Rede Paradise Hotéis - Login");
    this->setStyleSheet("QDialog {background-color: #F8F8F8;}");

    // Verifica se já existe uma conexão antes de criar
    if(QSqlDatabase::contains("qt_sql_default_connection")) {
        DBConnection = QSqlDatabase::database("qt_sql_default_connection");
    } else {
        DBConnection = QSqlDatabase::addDatabase("QSQLITE");
        QString dbPath = QString(PROJECT_SOURCE_DIR) + "/banco_dados.db";

        DBConnection.setDatabaseName(dbPath);
        qDebug() << "Caminho do banco:" << dbPath;

        // Verifica se o arquivo existe
        QFileInfo fileInfo(dbPath);
        if(fileInfo.exists()) {
            qDebug() << "Arquivo banco_dados.db ENCONTRADO!";
            qDebug() << "Tamanho:" << fileInfo.size() << "bytes";
        } else {
            qDebug() << "O Arquivo banco_dados.db NÃO FOI ENCONTRADO!";
            qDebug() << "Caminho esperado:" << dbPath;
        }
    }

    if(DBConnection.open()){
        qDebug() << "Database is Connected :)";

        // Verifica se a tabela USUARIOS existe
        QSqlQuery checkTable(DBConnection);
        if(checkTable.exec("SELECT name FROM sqlite_master WHERE type='table' AND name='USUARIOS'")) {
            //Se a tabela não existir, cria-se uma nova
            if(!checkTable.next()) {
                qDebug() << "Tabela 'USUARIOS' não existe. Criando...";

                QSqlQuery createTable(DBConnection);
                QString createTableSQL =
                    "CREATE TABLE USUARIOS ("
                    "id_usuario INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "usuario TEXT NOT NULL UNIQUE, "
                    "senha TEXT NOT NULL, "
                    "email TEXT, "
                    "nome TEXT, "
                    "Sobrenome TEXT)";

                if(createTable.exec(createTableSQL)) {
                    qDebug() << "Tabela USUARIOS criada com sucesso!";

                    // ============================================
                    // CRIA USUÁRIO DE TESTE COM SENHA CRIPTOGRAFADA
                    // ============================================
                    QString senhaAdmin = "admin123";
                    QByteArray senhaBytes = senhaAdmin.toUtf8();
                    QByteArray senhaHash = QCryptographicHash::hash(senhaBytes, QCryptographicHash::Sha256);
                    QString senhaHashHex = senhaHash.toHex();

                    qDebug() << "🔒 Criando usuário admin com senha criptografada";

                    QSqlQuery insertUser(DBConnection);
                    insertUser.prepare("INSERT INTO USUARIOS (usuario, senha, email, nome, Sobrenome) "
                                       "VALUES (?, ?, ?, ?, ?)");
                    insertUser.addBindValue("admin");
                    insertUser.addBindValue(senhaHashHex);  // <<<< SENHA CRIPTOGRAFADA
                    insertUser.addBindValue("admin@paradisehoteis.com");
                    insertUser.addBindValue("Administrador");
                    insertUser.addBindValue("Sistema");

                    if(insertUser.exec()) {
                        qDebug() << "Usuário de teste criado: admin / admin123";
                        QMessageBox::information(this, "Banco Criado",
                                                 "Banco de dados criado com sucesso!\n\n"
                                                 "Usuário de teste:\n"
                                                 "Login: admin\n"
                                                 "Senha: admin123");
                    }
                    // ============================================
                } else {
                    qDebug() << "ERRO ao criar tabela:" << createTable.lastError().text();
                }
            }

            if(checkTable.exec("SELECT name FROM sqlite_master WHERE type='table' AND name='USUARIOS'")) {
                if(checkTable.next()) {
                    qDebug() << "Tabela 'USUARIOS' encontrada!";

                    // Mostra quantos usuários existem
                    QSqlQuery countUsers(DBConnection);
                    if(countUsers.exec("SELECT COUNT(*) FROM USUARIOS")) {
                        if(countUsers.next()) {
                            qDebug() << "Total de usuários cadastrados:" << countUsers.value(0).toInt();
                        }
                    }

                } else {
                    qDebug() << "ERRO: Tabela USUARIOS ainda não existe após tentativa de criação!";
                }
            }
        }
    } else {
        qDebug() << "Database is not Connected :(";
        qDebug() << "Erro:" << DBConnection.lastError().text();
        QMessageBox::critical(this, "Erro de Banco",
                              "Não foi possível abrir o banco de dados:\n" + DBConnection.lastError().text());
    }

    // Para a pessoa conseguir ver a senha
    QAction *toggleVisibilityAction = ui->line_senha->addAction(
        QIcon(":/icons/of.png"),
        QLineEdit::TrailingPosition
        );

    connect(toggleVisibilityAction, &QAction::triggered, this, [this, toggleVisibilityAction]()
            {
                if (ui->line_senha->echoMode() == QLineEdit::Normal)
                {
                    ui->line_senha->setEchoMode(QLineEdit::Password);
                    toggleVisibilityAction->setIcon(QIcon(":/icons/of.png"));
                    toggleVisibilityAction->setToolTip("Mostrar senha");
                }
                else
                {
                    ui->line_senha->setEchoMode(QLineEdit::Normal);
                    toggleVisibilityAction->setIcon(QIcon(":/icons/oa.png"));
                    toggleVisibilityAction->setToolTip("Esconder senha");
                }
            });

    ui->line_senha->setEchoMode(QLineEdit::Password);

    // Quando apertar Enter no campo de login, vai para o campo de senha
    connect(ui->line_login, &QLineEdit::returnPressed, ui->line_senha,
            static_cast<void (QLineEdit::*)()>(&QLineEdit::setFocus));

    // Quando apertar Enter no campo de senha, faz o login
    connect(ui->line_senha, &QLineEdit::returnPressed, this, &LoginDialog::on_entrarButton_clicked);
}

LoginDialog::~LoginDialog()
{
    //Fecha a janela quando necessário
    if(DBConnection.isOpen()) {
        DBConnection.close();
    }
    delete ui;
}

void LoginDialog::on_criarconta_clicked(){

    CriarConta telaDeCadastro(this);
    telaDeCadastro.exec(); // O código para aqui até a telaDeCadastro fechar

    // Limpa os campos de login para o novo usuário digitar
    ui->line_login->clear();
    ui->line_senha->clear();
    ui->line_login->setFocus();
}

void LoginDialog::on_entrarButton_clicked()
{
    // Verifica se o banco está aberto
    if (!DBConnection.isOpen()) {
        qDebug() << "ERRO CRÍTICO: O banco de dados não está aberto!";
        QMessageBox::critical(this, "Erro", "Não foi possível conectar ao banco de dados!");
        return;
    }

    QString username = ui->line_login->text().trimmed();
    QString password = ui->line_senha->text().trimmed();

    // Validação básica
    if(username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Campos vazios", "Por favor, preencha todos os campos!");
        return;
    }

    qDebug() << "=== TENTATIVA DE LOGIN ===";
    qDebug() << "Usuário digitado:" << username;

    // ============================================
    // CRIPTOGRAFA A SENHA DIGITADA COM SHA-256
    // ============================================
    QByteArray senhaBytes = password.toUtf8();
    QByteArray senhaHash = QCryptographicHash::hash(senhaBytes, QCryptographicHash::Sha256);
    QString senhaHashHex = senhaHash.toHex();
    // ============================================

    QSqlQuery QueryGetUser(DBConnection);

    // Query ajustada para a tabela USUARIOS - COMPARA COM HASH
    QString queryString = "SELECT id_usuario, usuario, email, nome, Sobrenome FROM USUARIOS WHERE usuario = ? AND senha = ?";

    if(!QueryGetUser.prepare(queryString)) {
        qDebug() << "ERRO ao preparar query!";
        qDebug() << "Erro SQL:" << QueryGetUser.lastError().text();
        QMessageBox::critical(this, "Erro", "Erro ao preparar consulta SQL!");
        return;
    }

    // Bind dos valores - AGORA COM HASH
    QueryGetUser.addBindValue(username);
    QueryGetUser.addBindValue(senhaHashHex);  // COMPARA COM HASH, NÃO TEXTO PURO!

    // Executa
    if(!QueryGetUser.exec())
    {
        qDebug() << "ERRO: A query SQL falhou!";
        qDebug() << "Erro completo:" << QueryGetUser.lastError().text();
        QMessageBox::critical(this, "Erro no Banco",
                              "Erro ao consultar banco de dados:\n" + QueryGetUser.lastError().text());
        return;
    }

    qDebug() << "Query executada com sucesso!";

    // Verifica se encontrou usuário
    if(QueryGetUser.next())
    {
        // Pega os dados do usuário
        int idUsuario = QueryGetUser.value("id_usuario").toInt();
        QString usuarioEncontrado = QueryGetUser.value("usuario").toString();
        QString email = QueryGetUser.value("email").toString();
        QString nome = QueryGetUser.value("nome").toString();
        QString sobrenome = QueryGetUser.value("Sobrenome").toString();
        QString nomeCompleto = nome + " " + sobrenome;

        qDebug() << "=== USUÁRIO ENCONTRADO ===";
        qDebug() << "Usuário:" << usuarioEncontrado;
        qDebug() << "Nome completo:" << nomeCompleto;

        // Verifica se existe mais de um usuário
        if(QueryGetUser.next()) {
            qWarning() << "AVISO: Múltiplos usuários encontrados com as mesmas credenciais!";
            QMessageBox::warning(this, "Erro", "Múltiplos usuários encontrados! Verifique o banco de dados.");
            return;
        }

        // Login bem-sucedido!
        QMessageBox::information(this, "Login Sucesso",
                                 "Bem-vindo(a), " + nomeCompleto + "!");
        emit loggedIn(username);
        accept();
    }
    else
    {
        qDebug() << "Nenhum usuário encontrado!";
        QMessageBox::warning(this, "Login Inválido",
                             "Usuário ou senha incorretos.\nTente novamente.");
        ui->line_senha->clear();
        ui->line_login->setFocus();
    }
}
