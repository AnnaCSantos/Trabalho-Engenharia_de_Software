#include "perfildialog.h"
#include "ui_perfildialog.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QFileDialog>
#include <QBuffer>

PerfilDialog::PerfilDialog(QWidget *parent, const QString& username)
    : QDialog(parent)
    , ui(new Ui::PerfilDialog)
    , loggedInUsername(username)
    , modoEdicao(false)
{
    ui->setupUi(this);
    setWindowTitle("👤 Meu Perfil - EducaUTFPR");

    setupDatabase();

    // Remove a altura fixa e permite redimensionamento
    this->setMinimumSize(800, 600);  // Tamanho mínimo razoável
    this->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX); // Remove limite máximo
    this->resize(1000, 700); // Tamanho inicial confortável

    // Permite redimensionar a janela
    this->setSizeGripEnabled(true);
    
    // Configura a UI inicial
    ui->usernameLabel->setText("@" + loggedInUsername);
    setModoEdicao(false); // Começa bloqueado

    // Carrega os dados
    carregarDadosUsuario();
    carregarEstatisticas();
}

PerfilDialog::~PerfilDialog()
{
    delete ui;
}

void PerfilDialog::setupDatabase()
{
    dbConnection = QSqlDatabase::database("qt_sql_default_connection");
    if (!dbConnection.isOpen()) {
        qDebug() << "[PerfilDialog] ERRO: Banco de dados não está aberto.";
    }
}

int PerfilDialog::getIdUsuario(const QString& username)
{
    QSqlQuery query(dbConnection);
    // CORREÇÃO: Tabela USUARIOS
    query.prepare("SELECT id_usuario FROM USUARIOS WHERE usuario = ?");
    query.addBindValue(username);

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    return -1;
}

void PerfilDialog::carregarDadosUsuario()
{
    QSqlQuery query(dbConnection);
    // CORREÇÃO: Busca na tabela USUARIOS
    query.prepare("SELECT nome, Sobrenome, email, curso, semestre FROM USUARIOS WHERE usuario = ?");
    query.addBindValue(loggedInUsername);

    if (query.exec()) {
        if (query.next()) {
            QString nome = query.value("nome").toString();
            QString sobrenome = query.value("Sobrenome").toString();
            QString email = query.value("email").toString();
            QString curso = query.value("curso").toString();
            int semestre = query.value("semestre").toInt();

            // Preenche os campos
            ui->nomeEdit->setText(nome + " " + sobrenome);
            ui->emailEdit->setText(email);
            ui->cursoEdit->setText(curso);
        } else {
            QMessageBox::warning(this, "Erro", "Usuário não encontrado no banco de dados.");
            this->close(); // Fecha se não achar o usuário
        }
    } else {
        qDebug() << "Erro ao carregar perfil:" << query.lastError().text();
    }
}

void PerfilDialog::carregarEstatisticas()
{
    int idUsuario = getIdUsuario(loggedInUsername);
    if (idUsuario == -1) return;

    QSqlQuery query(dbConnection);

    // 1. Total de Tarefas
    query.prepare("SELECT COUNT(*) FROM Tarefas_Academicas WHERE id_usuario = ?");
    query.addBindValue(idUsuario);
    if (query.exec() && query.next()) {
        ui->totalTarefasLabel->setText(query.value(0).toString());
    }

    // 2. Concluídas
    query.prepare("SELECT COUNT(*) FROM Tarefas_Academicas WHERE id_usuario = ? AND concluida = 1");
    query.addBindValue(idUsuario);
    int concluidas = 0;
    if (query.exec() && query.next()) {
        concluidas = query.value(0).toInt();
        ui->concluidasLabel->setText(QString::number(concluidas));
    }

    // 3. Pendentes
    query.prepare("SELECT COUNT(*) FROM Tarefas_Academicas WHERE id_usuario = ? AND concluida = 0");
    query.addBindValue(idUsuario);
    if (query.exec() && query.next()) {
        ui->pendentesLabel->setText(query.value(0).toString());
    }

    // 4. Atrasadas
    query.prepare("SELECT COUNT(*) FROM Tarefas_Academicas WHERE id_usuario = ? AND concluida = 0 AND data_entrega < date('now')");
    query.addBindValue(idUsuario);
    if (query.exec() && query.next()) {
        ui->atrasadasLabel->setText(query.value(0).toString());
    }

    // 5. Taxa de Conclusão
    int total = ui->totalTarefasLabel->text().toInt();
    if (total > 0) {
        int taxa = (concluidas * 100) / total;
        ui->taxaLabel->setText(QString::number(taxa) + "%");
    } else {
        ui->taxaLabel->setText("0%");
    }
}

void PerfilDialog::setModoEdicao(bool edicao)
{
    modoEdicao = edicao;

    ui->nomeEdit->setReadOnly(!edicao);
    ui->emailEdit->setReadOnly(!edicao);
    ui->cursoEdit->setReadOnly(!edicao);

    ui->salvarButton->setVisible(edicao);
    ui->cancelarButton->setVisible(edicao);
    ui->escolherFotoButton->setVisible(true);
    ui->editarButton->setVisible(!edicao);

    if (edicao) {
        ui->nomeEdit->setStyleSheet("border: 2px solid #F4B315;");
    } else {
        ui->nomeEdit->setStyleSheet("");
    }
}
void PerfilDialog::on_editarButton_clicked()
{
    setModoEdicao(true);
}

void PerfilDialog::on_cancelarButton_clicked()
{
    carregarDadosUsuario(); // Recarrega os dados originais do banco
    setModoEdicao(false);
}

void PerfilDialog::on_salvarButton_clicked()
{
    // Separa Nome e Sobrenome (lógica simples)
    QString nomeCompleto = ui->nomeEdit->text().trimmed();
    QString nome, sobrenome;

    int espacoIndex = nomeCompleto.indexOf(' ');
    if (espacoIndex != -1) {
        nome = nomeCompleto.left(espacoIndex);
        sobrenome = nomeCompleto.mid(espacoIndex + 1);
    } else {
        nome = nomeCompleto;
        sobrenome = "";
    }

    QSqlQuery query(dbConnection);
    // CORREÇÃO: Update na tabela USUARIOS
    query.prepare("UPDATE USUARIOS SET nome = ?, Sobrenome = ?, email = ?, curso = ?, semestre = ? WHERE usuario = ?");

    query.addBindValue(nome);
    query.addBindValue(sobrenome);
    query.addBindValue(ui->emailEdit->text().trimmed());
    query.addBindValue(ui->cursoEdit->text().trimmed());
    query.addBindValue(loggedInUsername);

    if (query.exec()) {
        QMessageBox::information(this, "Sucesso", "Perfil atualizado com sucesso!");
        setModoEdicao(false);
    } else {
        QMessageBox::critical(this, "Erro", "Falha ao atualizar perfil: " + query.lastError().text());
    }
}

void PerfilDialog::on_alterarSenhaButton_clicked()
{
    bool ok;
    QString novaSenha = "";
    // Aqui você pode implementar um QInputDialog ou outro Dialog para pedir a senha
    // Por enquanto, deixei apenas o esqueleto
    QMessageBox::information(this, "Em breve", "Funcionalidade de alterar senha será implementada.");
}

void PerfilDialog::on_escolherFotoButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Escolher Foto", "", "Imagens (*.png *.jpg *.jpeg)");
    if (!fileName.isEmpty()) {
        // Aqui você pode implementar a lógica para salvar a foto no banco ou numa pasta
        // Exemplo visual apenas:
        // ui->fotoLabel->setPixmap(QPixmap(fileName).scaled(150, 150, Qt::KeepAspectRatioByExpanding));
        QMessageBox::information(this, "Foto", "Foto selecionada: " + fileName);
    }
}

