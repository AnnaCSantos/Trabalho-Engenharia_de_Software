#include "perfilDialog.h"
#include "ui_perfilDialog.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QFileDialog>
#include <QPixmap>
#include <QDebug>
#include <QInputDialog>

PerfilDialog::PerfilDialog(QWidget *parent, const QString& username)
    : QDialog(parent)
    , ui(new Ui::PerfilDialog)
    , loggedInUsername(username)
    , modoEdicao(false)  // Inicia em modo visualização
{
    ui->setupUi(this);
    setWindowTitle("👤 Meu Perfil - EducaUTFPR");
    resize(900, 700);

    setupDatabase();
    carregarDadosUsuario();
    carregarEstatisticas();

    // Inicia com campos desabilitados (modo visualização)
    setModoEdicao(false);
}

PerfilDialog::~PerfilDialog()
{
    delete ui;
}

// SETUP DATABASE - Conecta ao banco de dados
void PerfilDialog::setupDatabase()
{
    dbConnection = QSqlDatabase::database("qt_sql_default_connection");

    if (!dbConnection.isOpen()) {
        qDebug() << "[PerfilDialog] ERRO: Banco de dados não está aberto.";
    }
}

// GET ID USUARIO - Busca o ID do usuário no banco
int PerfilDialog::getIdUsuario(const QString& username)
{
    QSqlQuery query(dbConnection);
    query.prepare("SELECT id_usuario FROM Usuario WHERE usuario = ?");
    query.addBindValue(username);

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    return -1;
}

// CARREGAR DADOS DO USUÁRIO - Busca e exibe as informações
void PerfilDialog::carregarDadosUsuario()
{
    int idUsuario = getIdUsuario(loggedInUsername);
    if (idUsuario == -1) {
        QMessageBox::warning(this, "Erro", "Usuário não encontrado!");
        return;
    }

    // Busca os dados do usuário no banco
    QSqlQuery query(dbConnection);
    query.prepare(
        "SELECT nome_completo, email, curso, semestre, foto_perfil "
        "FROM Usuario WHERE id_usuario = ?"
        );
    query.addBindValue(idUsuario);

    if (!query.exec()) {
        qDebug() << "Erro ao carregar perfil:" << query.lastError().text();
        return;
    }

    if (query.next()) {
        // Preenche os campos com os dados do banco
        QString nomeCompleto = query.value("nome_completo").toString();
        QString email = query.value("email").toString();
        QString curso = query.value("curso").toString();
        int semestre = query.value("semestre").toInt();
        QString fotoPerfil = query.value("foto_perfil").toString();

        // Exibe o username (login)
        ui->usernameLabel->setText(loggedInUsername);

        // Preenche os campos editáveis
        ui->nomeEdit->setText(nomeCompleto.isEmpty() ? "Não informado" : nomeCompleto);
        ui->emailEdit->setText(email.isEmpty() ? "Não informado" : email);
        ui->cursoEdit->setText(curso.isEmpty() ? "Não informado" : curso);
        ui->semestreSpinBox->setValue(semestre == 0 ? 1 : semestre);

        // Carrega a foto de perfil (se existir)
        if (!fotoPerfil.isEmpty() && QFile::exists(fotoPerfil)) {
            QPixmap pixmap(fotoPerfil);
            ui->fotoLabel->setPixmap(
                pixmap.scaled(150, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation)
                );
        } else {
            // Foto padrão (emoji ou ícone)
            ui->fotoLabel->setText("👤");
            ui->fotoLabel->setStyleSheet(
                "font-size: 80px; "
                "background-color: #423738; "
                "border-radius: 75px; "
                "padding: 20px;"
                );
        }
    }
}

// CARREGAR ESTATÍSTICAS - Calcula e exibe métricas acadêmicas
void PerfilDialog::carregarEstatisticas()
{
    int idUsuario = getIdUsuario(loggedInUsername);
    if (idUsuario == -1) return;

    QSqlQuery query(dbConnection);

    // Total de tarefas
    query.prepare("SELECT COUNT(*) FROM Tarefas_Academicas WHERE id_usuario = ?");
    query.addBindValue(idUsuario);
    int totalTarefas = 0;
    if (query.exec() && query.next()) {
        totalTarefas = query.value(0).toInt();
    }

    // Tarefas concluídas
    query.prepare(
        "SELECT COUNT(*) FROM Tarefas_Academicas "
        "WHERE id_usuario = ? AND concluida = 1"
        );
    query.addBindValue(idUsuario);
    int tarefasConcluidas = 0;
    if (query.exec() && query.next()) {
        tarefasConcluidas = query.value(0).toInt();
    }

    // Tarefas pendentes
    int tarefasPendentes = totalTarefas - tarefasConcluidas;

    // Tarefas atrasadas
    query.prepare(
        "SELECT COUNT(*) FROM Tarefas_Academicas "
        "WHERE id_usuario = ? AND concluida = 0 AND date(data_entrega) < date('now')"
        );
    query.addBindValue(idUsuario);
    int tarefasAtrasadas = 0;
    if (query.exec() && query.next()) {
        tarefasAtrasadas = query.value(0).toInt();
    }

    // Calcula a taxa de conclusão
    double taxaConclusao = totalTarefas > 0
                               ? (tarefasConcluidas * 100.0 / totalTarefas)
                               : 0.0;

    // Atualiza os labels de estatísticas
    ui->totalTarefasLabel->setText(QString::number(totalTarefas));
    ui->concluidasLabel->setText(QString::number(tarefasConcluidas));
    ui->pendentesLabel->setText(QString::number(tarefasPendentes));
    ui->atrasadasLabel->setText(QString::number(tarefasAtrasadas));
    ui->taxaLabel->setText(QString::number(taxaConclusao, 'f', 1) + "%");

    // Define a cor da taxa baseada no valor
    QString corTaxa = "#8E6915";  // Padrão
    if (taxaConclusao >= 80) {
        corTaxa = "#4CAF50";  // Verde
    } else if (taxaConclusao >= 50) {
        corTaxa = "#D3AF35";  // Amarelo
    } else {
        corTaxa = "#FF6B6B";  // Vermelho
    }

    ui->taxaLabel->setStyleSheet(QString("color: %1; font-weight: bold;").arg(corTaxa));
}

// SET MODO EDIÇÃO - Ativa/desativa campos para edição
void PerfilDialog::setModoEdicao(bool edicao)
{
    modoEdicao = edicao;

    // Ativa ou desativa os campos
    ui->nomeEdit->setReadOnly(!edicao);
    ui->emailEdit->setReadOnly(!edicao);
    ui->cursoEdit->setReadOnly(!edicao);
    ui->semestreSpinBox->setReadOnly(!edicao);
    ui->escolherFotoButton->setEnabled(edicao);

    // Mostra/esconde botões
    ui->editarButton->setVisible(!edicao);
    ui->salvarButton->setVisible(edicao);
    ui->cancelarButton->setVisible(edicao);

    // Muda o estilo visual dos campos
    QString estiloLeitura =
        "background-color: #2A2426; "
        "border: 1px solid #423738; "
        "color: #F4B315; "
        "padding: 8px; "
        "border-radius: 5px;";

    QString estiloEdicao =
        "background-color: #423738; "
        "border: 2px solid #F4B315; "
        "color: #F4B315; "
        "padding: 8px; "
        "border-radius: 5px;";

    QString estilo = edicao ? estiloEdicao : estiloLeitura;
    ui->nomeEdit->setStyleSheet(estilo);
    ui->emailEdit->setStyleSheet(estilo);
    ui->cursoEdit->setStyleSheet(estilo);
}

// VALIDAR CAMPOS - Verifica se os dados são válidos
bool PerfilDialog::validarCampos()
{
    // Email deve conter @
    if (!ui->emailEdit->text().isEmpty() && !ui->emailEdit->text().contains("@")) {
        QMessageBox::warning(this, "⚠️ Email Inválido",
                             "Por favor, insira um email válido!");
        return false;
    }

    // Semestre deve ser entre 1 e 12
    if (ui->semestreSpinBox->value() < 1 || ui->semestreSpinBox->value() > 12) {
        QMessageBox::warning(this, "⚠️ Semestre Inválido",
                             "O semestre deve estar entre 1 e 12!");
        return false;
    }

    return true;
}

// BOTÃO EDITAR - Ativa o modo de edição
void PerfilDialog::on_editarButton_clicked()
{
    setModoEdicao(true);
    QMessageBox::information(this, "✏️ Modo Edição",
                             "Agora você pode editar seus dados!");
}

// BOTÃO SALVAR - Salva as alterações no banco
void PerfilDialog::on_salvarButton_clicked()
{
    // Valida os campos antes de salvar
    if (!validarCampos()) {
        return;
    }

    int idUsuario = getIdUsuario(loggedInUsername);
    if (idUsuario == -1) return;

    // Atualiza os dados no banco
    QSqlQuery updateQuery(dbConnection);
    updateQuery.prepare(
        "UPDATE Usuario SET "
        "nome_completo = ?, "
        "email = ?, "
        "curso = ?, "
        "semestre = ? "
        "WHERE id_usuario = ?"
        );

    updateQuery.addBindValue(ui->nomeEdit->text());
    updateQuery.addBindValue(ui->emailEdit->text());
    updateQuery.addBindValue(ui->cursoEdit->text());
    updateQuery.addBindValue(ui->semestreSpinBox->value());
    updateQuery.addBindValue(idUsuario);

    if (updateQuery.exec()) {
        QMessageBox::information(this, "✅ Sucesso",
                                 "Dados atualizados com sucesso!");
        setModoEdicao(false);
        carregarDadosUsuario();  // Recarrega os dados
    } else {
        QMessageBox::critical(this, "❌ Erro",
                              "Erro ao atualizar dados: " + updateQuery.lastError().text());
    }
}

// BOTÃO CANCELAR - Cancela a edição e restaura dados originais
void PerfilDialog::on_cancelarButton_clicked()
{
    setModoEdicao(false);
    carregarDadosUsuario();  // Recarrega os dados originais
    QMessageBox::information(this, "🚫 Cancelado",
                             "Edição cancelada. Dados não foram alterados.");
}

// BOTÃO ALTERAR SENHA - Permite mudar a senha do usuário
void PerfilDialog::on_alterarSenhaButton_clicked()
{
    // Pede a senha atual
    bool ok;
    QString senhaAtual = QInputDialog::getText(
        this,
        "🔐 Alterar Senha",
        "Digite sua senha atual:",
        QLineEdit::Password,
        "",
        &ok
        );

    if (!ok || senhaAtual.isEmpty()) {
        return;  // Usuário cancelou
    }

    // Verifica se a senha atual está correta
    int idUsuario = getIdUsuario(loggedInUsername);
    QSqlQuery query(dbConnection);
    query.prepare("SELECT senha FROM Usuario WHERE id_usuario = ?");
    query.addBindValue(idUsuario);

    if (!query.exec() || !query.next()) {
        QMessageBox::critical(this, "Erro", "Erro ao verificar senha!");
        return;
    }

    QString senhaCorreta = query.value(0).toString();
    if (senhaAtual != senhaCorreta) {
        QMessageBox::warning(this, "⚠️ Senha Incorreta",
                             "A senha atual está incorreta!");
        return;
    }

    // Pede a nova senha
    QString novaSenha = QInputDialog::getText(
        this,
        "🔐 Nova Senha",
        "Digite a nova senha:",
        QLineEdit::Password,
        "",
        &ok
        );

    if (!ok || novaSenha.isEmpty()) {
        return;
    }

    if (novaSenha.length() < 4) {
        QMessageBox::warning(this, "⚠️ Senha Fraca",
                             "A senha deve ter pelo menos 4 caracteres!");
        return;
    }

    // Confirma a nova senha
    QString confirmacao = QInputDialog::getText(
        this,
        "🔐 Confirmar Senha",
        "Digite a nova senha novamente:",
        QLineEdit::Password,
        "",
        &ok
        );

    if (!ok || confirmacao != novaSenha) {
        QMessageBox::warning(this, "⚠️ Senhas Diferentes",
                             "As senhas não coincidem!");
        return;
    }

    // Atualiza a senha no banco
    QSqlQuery updateQuery(dbConnection);
    updateQuery.prepare("UPDATE Usuario SET senha = ? WHERE id_usuario = ?");
    updateQuery.addBindValue(novaSenha);
    updateQuery.addBindValue(idUsuario);

    if (updateQuery.exec()) {
        QMessageBox::information(this, "✅ Sucesso",
                                 "Senha alterada com sucesso!");
    } else {
        QMessageBox::critical(this, "❌ Erro",
                              "Erro ao alterar senha: " + updateQuery.lastError().text());
    }
}

// ============================================================================
// BOTÃO ESCOLHER FOTO - Permite selecionar uma foto de perfil
// ============================================================================
void PerfilDialog::on_escolherFotoButton_clicked()
{
    // Abre um diálogo para escolher uma imagem
    QString caminhoFoto = QFileDialog::getOpenFileName(
        this,
        "Escolher Foto de Perfil",
        QDir::homePath(),
        "Imagens (*.png *.jpg *.jpeg *.bmp)"
        );

    if (caminhoFoto.isEmpty()) {
        return;  // Usuário cancelou
    }

    // Carrega e exibe a foto
    QPixmap pixmap(caminhoFoto);
    if (pixmap.isNull()) {
        QMessageBox::warning(this, "⚠️ Erro", "Não foi possível carregar a imagem!");
        return;
    }

    ui->fotoLabel->setPixmap(
        pixmap.scaled(150, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation)
        );

    // Salva o caminho da foto no banco
    int idUsuario = getIdUsuario(loggedInUsername);
    QSqlQuery updateQuery(dbConnection);
    updateQuery.prepare("UPDATE Usuario SET foto_perfil = ? WHERE id_usuario = ?");
    updateQuery.addBindValue(caminhoFoto);
    updateQuery.addBindValue(idUsuario);

    if (updateQuery.exec()) {
        QMessageBox::information(this, "✅ Sucesso", "Foto atualizada!");
    } else {
        QMessageBox::critical(this, "❌ Erro",
                              "Erro ao salvar foto: " + updateQuery.lastError().text());
    }
}
