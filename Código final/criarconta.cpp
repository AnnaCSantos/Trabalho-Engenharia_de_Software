#include "criarconta.h"
#include "ui_criarconta.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QRegularExpression>
#include <QCoreApplication>
#include <QFileInfo>
#include <QDir>
#include <QCryptographicHash>  //Biblioteca responsável pela Criptrografica

CriarConta::CriarConta(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CriarConta)
{
    ui->setupUi(this);
    setWindowTitle("Rede Paradise Hotéis - Criar Conta");

    // Conecta ao banco de dados
    if(QSqlDatabase::contains("qt_sql_default_connection")) {
        DBConnection = QSqlDatabase::database("qt_sql_default_connection");
    } else {
        DBConnection = QSqlDatabase::addDatabase("QSQLITE");

        QString dbPath = QString(PROJECT_SOURCE_DIR) + "/banco_dados.db";

        DBConnection.setDatabaseName(dbPath);
        qDebug() << "[CriarConta] Caminho do banco:" << dbPath;
    }

    // Tenta abrir a conexão se não estiver aberta
    if(!DBConnection.open()) {
        qDebug() << "[CriarConta] ERRO ao abrir banco de dados!";
        qDebug() << "Erro:" << DBConnection.lastError().text();
        QMessageBox::critical(this, "Erro de Banco",
                              "Não foi possível conectar ao banco de dados:\n" + DBConnection.lastError().text());
    } else {
        qDebug() << "[CriarConta] Banco de dados conectado com sucesso!";
    }

    // Limpa os placeholders padrão
    ui->sobrenomeLineEdit->clear();
    ui->lineEdit_5->clear();
    ui->senhaLineEdit->clear();
    ui->emailLineEdit->clear();
    ui->lineEdit_3->clear();

    // Define placeholders mais claros
    ui->sobrenomeLineEdit->setPlaceholderText("Nome");
    ui->lineEdit_5->setPlaceholderText("Sobrenome");
    ui->senhaLineEdit->setPlaceholderText("Usuário");
    ui->emailLineEdit->setPlaceholderText("E-mail");
    ui->lineEdit_3->setPlaceholderText("Senha");

    // Define o campo senha como password (oculta os caracteres)
    ui->lineEdit_3->setEchoMode(QLineEdit::Password);

    // ============================================
    // CONFIGURA ENTER PARA NAVEGAR ENTRE CAMPOS
    // ============================================
    connect(ui->sobrenomeLineEdit, &QLineEdit::returnPressed, ui->lineEdit_5,
            static_cast<void (QLineEdit::*)()>(&QLineEdit::setFocus));
    connect(ui->lineEdit_5, &QLineEdit::returnPressed, ui->senhaLineEdit,
            static_cast<void (QLineEdit::*)()>(&QLineEdit::setFocus));
    connect(ui->senhaLineEdit, &QLineEdit::returnPressed, ui->emailLineEdit,
            static_cast<void (QLineEdit::*)()>(&QLineEdit::setFocus));
    connect(ui->emailLineEdit, &QLineEdit::returnPressed, ui->lineEdit_3,
            static_cast<void (QLineEdit::*)()>(&QLineEdit::setFocus));
    // Quando apertar Enter no campo de senha, cria a conta
    connect(ui->lineEdit_3, &QLineEdit::returnPressed, this, &CriarConta::on_criarcontaButton_clicked);
    // ============================================
}

CriarConta::~CriarConta()
{
    delete ui;
}


void CriarConta::on_criarcontaButton_clicked()
{
    qDebug() << "=== TENTATIVA DE CRIAR CONTA ===";

    // Verifica se o banco está aberto
    if(!DBConnection.isOpen()) {
        QMessageBox::critical(this, "Erro", "Banco de dados não está conectado!");
        return;
    }

    // Valida os campos
    if(!validarCampos()) {
        return;  // Se validação falhar, para aqui
    }

    // Pega os valores dos campos
    QString nome = ui->sobrenomeLineEdit->text().trimmed();
    QString sobrenome = ui->lineEdit_5->text().trimmed();
    QString usuario = ui->senhaLineEdit->text().trimmed();
    QString email = ui->emailLineEdit->text().trimmed();
    QString senha = ui->lineEdit_3->text().trimmed();

    qDebug() << "Nome:" << nome;
    qDebug() << "Sobrenome:" << sobrenome;
    qDebug() << "Usuário:" << usuario;
    qDebug() << "Email:" << email;

    // Verifica se o usuário já existe
    if(usuarioJaExiste(usuario)) {
        QMessageBox::warning(this, "Usuário já existe",
                             "Este nome de usuário já está cadastrado!\nEscolha outro nome de usuário.");
        ui->senhaLineEdit->setFocus();
        return;
    }

    // ============================================
    // CRIPTOGRAFA A SENHA COM SHA-256
    // ============================================
    QByteArray senhaBytes = senha.toUtf8();
    QByteArray senhaHash = QCryptographicHash::hash(senhaBytes, QCryptographicHash::Sha256);
    QString senhaHashHex = senhaHash.toHex();  // Converte para formato hexadecimal
    // ============================================

    // Insere no banco de dados
    QSqlQuery insertQuery(DBConnection);
    QString queryString = "INSERT INTO USUARIOS (usuario, senha, email, nome, Sobrenome) VALUES (?, ?, ?, ?, ?)";

    if(!insertQuery.prepare(queryString)) {
        qDebug() << "ERRO ao preparar INSERT:" << insertQuery.lastError().text();
        QMessageBox::critical(this, "Erro", "Erro ao preparar inserção no banco!");
        return;
    }

    // Bind dos valores - AGORA COM SENHA CRIPTOGRAFADA
    insertQuery.addBindValue(usuario);
    insertQuery.addBindValue(senhaHashHex);  // <<<< HASH SHA-256 em vez de texto puro!
    insertQuery.addBindValue(email);
    insertQuery.addBindValue(nome);
    insertQuery.addBindValue(sobrenome);

    // Executa a inserção
    if(insertQuery.exec()) {
        qDebug() << "✓ Usuário cadastrado com sucesso!";

        QMessageBox::information(this, "Conta Criada!",
                                 "Conta criada com sucesso!\n\n"
                                 "Você já pode fazer login com seu usuário e senha.");

        // Fecha a janela e volta para o login
        accept();
    } else {
        qDebug() << "ERRO ao inserir usuário:" << insertQuery.lastError().text();
        QMessageBox::critical(this, "Erro ao Criar Conta",
                              "Não foi possível criar a conta:\n" + insertQuery.lastError().text());
    }
}

// Slot para o botão "Login" (voltar)
void CriarConta::on_pushButton_2_clicked()
{
    qDebug() << "Voltando para a tela de login...";
    close();  // Fecha a janela e volta para o login
}

// Valida se os campos estão preenchidos
bool CriarConta::validarCampos()
{
    QString nome = ui->sobrenomeLineEdit->text().trimmed();
    QString sobrenome = ui->lineEdit_5->text().trimmed();
    QString usuario = ui->senhaLineEdit->text().trimmed();
    QString email = ui->emailLineEdit->text().trimmed();
    QString senha = ui->lineEdit_3->text().trimmed();

    // Verifica campos vazios
    if(nome.isEmpty()) {
        QMessageBox::warning(this, "Campo vazio", "Por favor, preencha o Nome!");
        ui->sobrenomeLineEdit->setFocus();
        return false;
    }

    if(sobrenome.isEmpty()) {
        QMessageBox::warning(this, "Campo vazio", "Por favor, preencha o Sobrenome!");
        ui->lineEdit_5->setFocus();
        return false;
    }

    if(usuario.isEmpty()) {
        QMessageBox::warning(this, "Campo vazio", "Por favor, preencha o Usuário!");
        ui->senhaLineEdit->setFocus();
        return false;
    }

    if(email.isEmpty()) {
        QMessageBox::warning(this, "Campo vazio", "Por favor, preencha o E-mail!");
        ui->emailLineEdit->setFocus();
        return false;
    }

    if(senha.isEmpty()) {
        QMessageBox::warning(this, "Campo vazio", "Por favor, preencha a Senha!");
        ui->lineEdit_3->setFocus();
        return false;
    }

    // Validação de senha mínima
    if(senha.length() < 6) {
        QMessageBox::warning(this, "Senha fraca",
                             "A senha deve ter no mínimo 6 caracteres!");
        ui->lineEdit_3->setFocus();
        return false;
    }

    // Validação básica de email
    QRegularExpression emailRegex("^[\\w\\.-]+@[\\w\\.-]+\\.\\w+$");
    if(!emailRegex.match(email).hasMatch()) {
        QMessageBox::warning(this, "E-mail inválido",
                             "Por favor, insira um e-mail válido!\nExemplo: usuario@exemplo.com");
        ui->emailLineEdit->setFocus();
        return false;
    }

    return true;  // Tudo OK!
}

// Verifica se o usuário já existe no banco
bool CriarConta::usuarioJaExiste(const QString &usuario)
{
    QSqlQuery checkQuery(DBConnection);
    QString queryString = "SELECT COUNT(*) FROM USUARIOS WHERE usuario = ?";

    if(!checkQuery.prepare(queryString)) {
        qDebug() << "ERRO ao preparar verificação:" << checkQuery.lastError().text();
        return false;
    }

    checkQuery.addBindValue(usuario);

    if(checkQuery.exec() && checkQuery.next()) {
        int count = checkQuery.value(0).toInt();
        qDebug() << "Usuários com esse nome encontrados:" << count;
        return count > 0;
    }

    return false;
}
