#include "agendaacademicadialog.h"
#include "ui_agendaacademicadialog.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QDateEdit>
#include <QComboBox>
#include <QTextEdit>
#include <QLineEdit>
#include <QCheckBox>
#include <QCalendarWidget>
#include <QEvent>
#include <QMouseEvent>

#include "perfildialog.h"

AgendaAcademicaDialog::AgendaAcademicaDialog(QWidget *parent, const QString& username)
    : QDialog(parent)
    , ui(new Ui::AgendaAcademicaDialog)
    , loggedInUsername(username)
{
    ui->setupUi(this);
    setWindowTitle("📅 Agenda Acadêmica EducaUTFPR");
    resize(1200, 800);

    setupDatabase();
    criarTabelaTarefas();

    ui->filtroComboBox->addItem("Todos");
    ui->filtroComboBox->addItem("Provas");
    ui->filtroComboBox->addItem("Trabalhos");
    ui->filtroComboBox->addItem("Projetos");
    ui->filtroComboBox->addItem("Pendentes");
    ui->filtroComboBox->addItem("✅Concluídas");
    ui->filtroComboBox->addItem("Esta Semana");
    ui->filtroComboBox->addItem("Este Mês");

    carregarTarefas();

    setupNavigationBar();

    connect(ui->filtroComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AgendaAcademicaDialog::on_filtroComboBox_currentIndexChanged);
}

AgendaAcademicaDialog::~AgendaAcademicaDialog()
{
    delete ui;
}

void AgendaAcademicaDialog::setupDatabase()
{
    dbConnection = QSqlDatabase::database("qt_sql_default_connection");

    if (!dbConnection.isOpen()) {
        qDebug() << "[AgendaAcademica] ERRO: Banco de dados não está aberto.";
    }
}

void AgendaAcademicaDialog::criarTabelaTarefas()
{
    QSqlQuery query(dbConnection);

    QString createTableSQL =
        "CREATE TABLE IF NOT EXISTS Tarefas_Academicas ("
        "id_tarefa INTEGER PRIMARY KEY AUTOINCREMENT, "
        "id_usuario INTEGER NOT NULL, "
        "tipo TEXT NOT NULL, "
        "titulo TEXT NOT NULL, "
        "descricao TEXT, "
        "data_entrega DATE NOT NULL, "
        "disciplina TEXT, "
        "concluida INTEGER DEFAULT 0, "
        "data_criacao DATETIME DEFAULT CURRENT_TIMESTAMP, "
        "FOREIGN KEY (id_usuario) REFERENCES USUARIOS(id_usuario))";

    if (!query.exec(createTableSQL)) {
        qDebug() << "Erro ao criar tabela Tarefas_Academicas:" << query.lastError().text();
    } else {
        qDebug() << "Tabela Tarefas_Academicas verificada/criada com sucesso!";
    }
}

int AgendaAcademicaDialog::getIdUsuario(const QString& username)
{
    QSqlQuery query(dbConnection);
    query.prepare("SELECT id_usuario FROM USUARIOS WHERE usuario = ?");
    query.addBindValue(username);

    if (query.exec() && query.next()) {
        int id = query.value(0).toInt();
        qDebug() << "[AgendaAcademica] ID do usuário" << username << ":" << id;
        return id;
    }

    qDebug() << "[AgendaAcademica] ERRO: Usuário não encontrado!" << query.lastError().text();
    return -1;
}

void AgendaAcademicaDialog::carregarTarefas(const QString& filtro)
{
    qDebug() << "=== CARREGANDO TAREFAS ===";
    qDebug() << "Usuário logado:" << loggedInUsername;

    QWidget *containerWidget = ui->scrollArea->widget();

    QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(containerWidget->layout());
    if (!layout) {
        layout = new QVBoxLayout(containerWidget);
        layout->setSpacing(12);
        layout->setContentsMargins(15, 15, 15, 15);
        containerWidget->setLayout(layout);
    }

    QLayoutItem *item;
    while ((item = layout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    int idUsuario = getIdUsuario(loggedInUsername);
    if (idUsuario == -1) {
        qDebug() << "ERRO: Usuário não encontrado!";
        QLabel *erroLabel = new QLabel("❌ Erro: Usuário não encontrado no banco de dados!");
        erroLabel->setAlignment(Qt::AlignCenter);
        erroLabel->setStyleSheet("color: #FF6B6B; font-size: 16px; padding: 30px;");
        layout->addWidget(erroLabel);
        return;
    }

    qDebug() << "ID do usuário:" << idUsuario;

    // DEBUG: Verificar se a tabela existe e tem dados
    QSqlQuery debugQuery(dbConnection);
    debugQuery.exec("SELECT COUNT(*) FROM Tarefas_Academicas");
    if (debugQuery.next()) {
        qDebug() << "Total de tarefas no banco:" << debugQuery.value(0).toInt();
    }

    debugQuery.prepare("SELECT COUNT(*) FROM Tarefas_Academicas WHERE id_usuario = ?");
    debugQuery.addBindValue(idUsuario);
    debugQuery.exec();
    if (debugQuery.next()) {
        qDebug() << "Tarefas deste usuário:" << debugQuery.value(0).toInt();
    }

    QString queryString = "SELECT * FROM Tarefas_Academicas WHERE id_usuario = ?";

    QString filtroAtual = ui->filtroComboBox->currentText();

    if (filtroAtual.contains("Provas")) {
        queryString += " AND tipo = 'Prova'";
    } else if (filtroAtual.contains("Trabalhos")) {
        queryString += " AND tipo = 'Trabalho'";
    } else if (filtroAtual.contains("Projetos")) {
        queryString += " AND tipo = 'Projeto'";
    } else if (filtroAtual.contains("Pendentes")) {
        queryString += " AND concluida = 0";
    } else if (filtroAtual.contains("Concluídas")) {
        queryString += " AND concluida = 1";
    } else if (filtroAtual.contains("Esta Semana")) {
        queryString += QString(" AND date(data_entrega) BETWEEN date('%1') AND date('%2')")
        .arg(QDate::currentDate().toString("yyyy-MM-dd"))
            .arg(QDate::currentDate().addDays(7).toString("yyyy-MM-dd"));
    } else if (filtroAtual.contains("Este Mês")) {
        queryString += QString(" AND strftime('%Y-%m', data_entrega) = '%1'")
        .arg(QDate::currentDate().toString("yyyy-MM"));
    }

    queryString += " ORDER BY data_entrega ASC, concluida ASC";

    qDebug() << "Query executada:" << queryString;

    QSqlQuery query(dbConnection);
    query.prepare(queryString);
    query.addBindValue(idUsuario);

    if (!query.exec()) {
        qDebug() << "ERRO ao executar query:" << query.lastError().text();
        QLabel *erroLabel = new QLabel("❌ Erro ao carregar tarefas: " + query.lastError().text());
        erroLabel->setAlignment(Qt::AlignCenter);
        erroLabel->setStyleSheet("color: #FF6B6B; font-size: 14px; padding: 20px;");
        layout->addWidget(erroLabel);
        return;
    }

    int count = 0;
    while (query.next()) {
        int id = query.value("id_tarefa").toInt();
        QString tipo = query.value("tipo").toString();
        QString titulo = query.value("titulo").toString();
        QString descricao = query.value("descricao").toString();
        QDate dataEntrega = query.value("data_entrega").toDate();
        QString disciplina = query.value("disciplina").toString();
        bool concluida = query.value("concluida").toInt() == 1;

        qDebug() << "Tarefa carregada:" << id << "-" << titulo;

        QFrame *tarefaCard = criarCardTarefa(id, tipo, titulo, descricao, dataEntrega, disciplina, concluida);
        layout->addWidget(tarefaCard);
        count++;
    }

    qDebug() << "Total de tarefas exibidas:" << count;

    if (count == 0) {
        QLabel *emptyLabel = new QLabel("Nenhuma tarefa encontrada.\nClique em '➕ Nova Tarefa' para adicionar!");
        emptyLabel->setAlignment(Qt::AlignCenter);
        emptyLabel->setStyleSheet(
            "color: #8E6915; "
            "font-size: 16px; "
            "margin: 50px; "
            "padding: 30px; "
            "background-color: #423738; "
            "border-radius: 10px;"
            );
        layout->addWidget(emptyLabel);
    }

    layout->addStretch();
}

QFrame* AgendaAcademicaDialog::criarCardTarefa(int id, const QString& tipo, const QString& titulo,
                                               const QString& descricao, const QDate& dataEntrega,
                                               const QString& disciplina, bool concluida)
{
    QFrame *card = new QFrame();
    card->setObjectName(QString::number(id));
    card->setFrameShape(QFrame::StyledPanel);
    card->setMinimumHeight(120);

    QString corBorda = "#F4B315";
    QString corFundo = "#423738";
    QString corTexto = "#F4B315";

    if (concluida) {
        corBorda = "#8E6915";
        corFundo = "#2A2426";
        corTexto = "#8E6915";
    } else if (dataEntrega < QDate::currentDate()) {
        corBorda = "#E53232";
        corFundo = "#4A2020";
        corTexto = "#FF6B6B";
    } else if (dataEntrega <= QDate::currentDate().addDays(3)) {
        corBorda = "#D3AF35";
        corFundo = "#4A4020";
    }

    card->setStyleSheet(
        QString("QFrame {"
                "   background-color: %1;"
                "   border-left: 6px solid %2;"
                "   border-radius: 10px;"
                "   padding: 15px;"
                "   margin: 5px;"
                "}"
                "QFrame:hover {"
                "   background-color: #524447;"
                "   border-left: 8px solid %2;"
                "}").arg(corFundo).arg(corBorda)
        );

    QHBoxLayout *mainLayout = new QHBoxLayout(card);
    mainLayout->setSpacing(15);

    // Coluna 1: Ícone
    QVBoxLayout *iconCol = new QVBoxLayout();
    iconCol->setAlignment(Qt::AlignTop);

    QString icone;
    if (tipo == "Prova") icone = "📝";
    else if (tipo == "Trabalho") icone = "📄";
    else if (tipo == "Projeto") icone = "🎯";

    QLabel *iconeLabel = new QLabel(icone);
    iconeLabel->setStyleSheet(QString("font-size: 36px; color: %1;").arg(corTexto));
    iconeLabel->setAlignment(Qt::AlignCenter);

    QLabel *tipoLabel = new QLabel(tipo);
    tipoLabel->setStyleSheet(QString("font-size: 11px; font-weight: bold; color: %1;").arg(corTexto));
    tipoLabel->setAlignment(Qt::AlignCenter);

    iconCol->addWidget(iconeLabel);
    iconCol->addWidget(tipoLabel);

    // Coluna 2: Informações
    QVBoxLayout *infoCol = new QVBoxLayout();

    QLabel *tituloLabel = new QLabel(titulo);
    tituloLabel->setStyleSheet(
        QString("font-size: 18px; font-weight: bold; color: %1; margin-bottom: 5px;").arg(corTexto)
        );
    tituloLabel->setWordWrap(true);

    QLabel *disciplinaLabel = new QLabel("📚 " + disciplina);
    disciplinaLabel->setStyleSheet("color: #D3AF35; font-size: 13px; font-weight: bold;");

    QLabel *descLabel = new QLabel(descricao);
    descLabel->setWordWrap(true);
    descLabel->setStyleSheet("color: #8E6915; font-size: 12px; margin-top: 5px;");
    if (descricao.length() > 80) {
        descLabel->setText(descricao.left(77) + "...");
    }

    infoCol->addWidget(tituloLabel);
    infoCol->addWidget(disciplinaLabel);
    infoCol->addWidget(descLabel);
    infoCol->addStretch();

    // Coluna 3: Data
    QVBoxLayout *statusCol = new QVBoxLayout();
    statusCol->setAlignment(Qt::AlignTop | Qt::AlignRight);

    QFrame *dataFrame = new QFrame();
    dataFrame->setStyleSheet(
        QString("background-color: %1; border-radius: 8px; padding: 8px;").arg(corBorda)
        );
    QVBoxLayout *dataLayout = new QVBoxLayout(dataFrame);
    dataLayout->setSpacing(2);
    dataLayout->setContentsMargins(10, 5, 10, 5);

    QLabel *diaLabel = new QLabel(QString::number(dataEntrega.day()));
    diaLabel->setStyleSheet("color: #1A161A; font-size: 24px; font-weight: bold;");
    diaLabel->setAlignment(Qt::AlignCenter);

    QLabel *mesLabel = new QLabel(dataEntrega.toString("MMM").toUpper());
    mesLabel->setStyleSheet("color: #1A161A; font-size: 11px; font-weight: bold;");
    mesLabel->setAlignment(Qt::AlignCenter);

    dataLayout->addWidget(diaLabel);
    dataLayout->addWidget(mesLabel);

    statusCol->addWidget(dataFrame);
    statusCol->addSpacing(10);

    if (concluida) {
        QLabel *statusLabel = new QLabel("✅ CONCLUÍDA");
        statusLabel->setStyleSheet(
            "background-color: #2A2426; color: #8E6915; padding: 5px 10px; "
            "border-radius: 5px; font-size: 11px; font-weight: bold;"
            );
        statusCol->addWidget(statusLabel);
    } else if (dataEntrega < QDate::currentDate()) {
        int diasAtrasado = QDate::currentDate().daysTo(dataEntrega);
        QLabel *statusLabel = new QLabel(QString("⚠️ ATRASADA\n%1 dias").arg(abs(diasAtrasado)));
        statusLabel->setStyleSheet(
            "background-color: #4A2020; color: #FF6B6B; padding: 5px 10px; "
            "border-radius: 5px; font-size: 10px; font-weight: bold;"
            );
        statusLabel->setAlignment(Qt::AlignCenter);
        statusCol->addWidget(statusLabel);
    } else {
        int diasRestantes = QDate::currentDate().daysTo(dataEntrega);
        QString textoStatus = diasRestantes == 0 ? "HOJE!" : QString("%1 dias").arg(diasRestantes);
        QLabel *statusLabel = new QLabel("" + textoStatus);
        statusLabel->setStyleSheet(
            "background-color: #4A4020; color: #D3AF35; padding: 5px 10px; "
            "border-radius: 5px; font-size: 11px; font-weight: bold;"
            );
        statusCol->addWidget(statusLabel);
    }

    statusCol->addStretch();

    // Coluna 4: Botões
    QVBoxLayout *acoesCol = new QVBoxLayout();
    acoesCol->setAlignment(Qt::AlignTop | Qt::AlignRight);
    acoesCol->setSpacing(8);

    QPushButton *concluirBtn = new QPushButton(concluida ? "✓" : "✓ Concluir");
    concluirBtn->setEnabled(!concluida);
    concluirBtn->setMinimumSize(90, 35);
    concluirBtn->setStyleSheet(
        "QPushButton { background-color: #8E6915; color: #F4B315; border: none; "
        "border-radius: 6px; padding: 8px 12px; font-weight: bold; font-size: 12px; }"
        "QPushButton:hover { background-color: #6B4F0F; }"
        "QPushButton:disabled { background-color: #2A2426; color: #423738; }"
        );

    connect(concluirBtn, &QPushButton::clicked, [this, id]() {
        QSqlQuery updateQuery(dbConnection);
        updateQuery.prepare("UPDATE Tarefas_Academicas SET concluida = 1 WHERE id_tarefa = ?");
        updateQuery.addBindValue(id);

        if (updateQuery.exec()) {
            QMessageBox::information(this, "✅ Sucesso", "Tarefa marcada como concluída!");
            carregarTarefas(ui->filtroComboBox->currentText());
        }
    });

    QPushButton *removerBtn = new QPushButton("🗑");
    removerBtn->setMinimumSize(35, 35);
    removerBtn->setMaximumWidth(35);
    removerBtn->setStyleSheet(
        "QPushButton { background-color: #4A2020; color: #FF6B6B; border: none; "
        "border-radius: 6px; font-size: 16px; }"
        "QPushButton:hover { background-color: #5A2828; }"
        );

    connect(removerBtn, &QPushButton::clicked, [this, id]() {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, "⚠️ Confirmar Remoção",
            "Tem certeza que deseja remover esta tarefa?",
            QMessageBox::Yes | QMessageBox::No
            );

        if (reply == QMessageBox::Yes) {
            QSqlQuery deleteQuery(dbConnection);
            deleteQuery.prepare("DELETE FROM Tarefas_Academicas WHERE id_tarefa = ?");
            deleteQuery.addBindValue(id);

            if (deleteQuery.exec()) {
                QMessageBox::information(this, "✅ Sucesso", "Tarefa removida!");
                carregarTarefas(ui->filtroComboBox->currentText());
            }
        }
    });

    acoesCol->addWidget(concluirBtn);
    acoesCol->addWidget(removerBtn);
    acoesCol->addStretch();

    mainLayout->addLayout(iconCol);
    mainLayout->addLayout(infoCol, 1);
    mainLayout->addLayout(statusCol);
    mainLayout->addLayout(acoesCol);

    return card;
}

void AgendaAcademicaDialog::on_adicionarTarefaButton_clicked()
{
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Nova Tarefa Acadêmica");
    dialog->resize(650, 550);

    dialog->setStyleSheet(
        "QDialog { background-color: #1A161A; }"
        "QLabel { color: #F4B315; font-size: 13px; font-weight: bold; }"
        "QLineEdit, QTextEdit, QDateEdit, QComboBox {"
        "   background-color: #423738; color: #F4B315; border: 2px solid #8E6915;"
        "   border-radius: 6px; padding: 8px; font-size: 13px; }"
        "QLineEdit:focus, QTextEdit:focus, QDateEdit:focus, QComboBox:focus {"
        "   border-color: #F4B315; }"
        );

    QVBoxLayout *layout = new QVBoxLayout(dialog);
    layout->setSpacing(12);

    QLabel *tituloDialog = new QLabel("Adicionar Nova Tarefa");
    tituloDialog->setStyleSheet("font-size: 20px; color: #F4B315; font-weight: bold; margin-bottom: 10px;");

    QLabel *tipoLabel = new QLabel("Tipo de Tarefa:");
    QComboBox *tipoCombo = new QComboBox();
    tipoCombo->addItems({"Prova", "Trabalho", "Projeto"});
    tipoCombo->setMinimumHeight(40);

    QLabel *tituloLabel = new QLabel("Título:");
    QLineEdit *tituloEdit = new QLineEdit();
    tituloEdit->setPlaceholderText("Ex: Prova de Cálculo 2");
    tituloEdit->setMinimumHeight(40);

    QLabel *disciplinaLabel = new QLabel("Disciplina:");
    QLineEdit *disciplinaEdit = new QLineEdit();
    disciplinaEdit->setPlaceholderText("Ex: Cálculo 2");
    disciplinaEdit->setMinimumHeight(40);

    QLabel *dataLabel = new QLabel("📅 Data de Entrega:");
    QDateEdit *dataEdit = new QDateEdit();
    dataEdit->setDate(QDate::currentDate());
    dataEdit->setDisplayFormat("dd/MM/yyyy");
    dataEdit->setCalendarPopup(true);
    dataEdit->setMinimumHeight(40);

    QLabel *descLabel = new QLabel("Descrição:");
    QTextEdit *descEdit = new QTextEdit();
    descEdit->setPlaceholderText("Detalhes sobre a tarefa...");
    descEdit->setMaximumHeight(120);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *salvarBtn = new QPushButton("💾 Salvar");
    QPushButton *cancelarBtn = new QPushButton("✕ Cancelar");

    salvarBtn->setMinimumHeight(45);
    cancelarBtn->setMinimumHeight(45);

    salvarBtn->setStyleSheet(
        "QPushButton { background-color: #F4B315; color: #1A161A; border: none; "
        "border-radius: 8px; padding: 10px 20px; font-weight: bold; font-size: 14px; }"
        "QPushButton:hover { background-color: #D3AF35; }"
        );

    cancelarBtn->setStyleSheet(
        "QPushButton { background-color: #423738; color: #F4B315; border: 2px solid #F4B315; "
        "border-radius: 8px; padding: 10px 20px; font-weight: bold; font-size: 14px; }"
        "QPushButton:hover { background-color: #524447; }"
        );

    btnLayout->addWidget(cancelarBtn);
    btnLayout->addWidget(salvarBtn);

    layout->addWidget(tituloDialog);
    layout->addWidget(tipoLabel);
    layout->addWidget(tipoCombo);
    layout->addWidget(tituloLabel);
    layout->addWidget(tituloEdit);
    layout->addWidget(disciplinaLabel);
    layout->addWidget(disciplinaEdit);
    layout->addWidget(dataLabel);
    layout->addWidget(dataEdit);
    layout->addWidget(descLabel);
    layout->addWidget(descEdit);
    layout->addStretch();
    layout->addLayout(btnLayout);

    connect(cancelarBtn, &QPushButton::clicked, dialog, &QDialog::reject);

    connect(salvarBtn, &QPushButton::clicked, [=]() {
        QString titulo = tituloEdit->text().trimmed();
        QString disciplina = disciplinaEdit->text().trimmed();
        QString descricao = descEdit->toPlainText().trimmed();

        if (titulo.isEmpty() || disciplina.isEmpty()) {
            QMessageBox::warning(dialog, "⚠️ Campos Obrigatórios",
                                 "Título e Disciplina são obrigatórios!");
            return;
        }

        int idUsuario = getIdUsuario(loggedInUsername);

        if (idUsuario == -1) {
            QMessageBox::critical(dialog, "❌ Erro", "Erro: Usuário não encontrado no banco de dados!");
            return;
        }

        qDebug() << "=== SALVANDO TAREFA ===";
        qDebug() << "ID Usuário:" << idUsuario;
        qDebug() << "Tipo:" << tipoCombo->currentText();
        qDebug() << "Título:" << titulo;
        qDebug() << "Disciplina:" << disciplina;
        qDebug() << "Data:" << dataEdit->date().toString("yyyy-MM-dd");

        QSqlQuery insertQuery(dbConnection);
        insertQuery.prepare(
            "INSERT INTO Tarefas_Academicas (id_usuario, tipo, titulo, descricao, "
            "data_entrega, disciplina) VALUES (?, ?, ?, ?, ?, ?)"
            );
        insertQuery.addBindValue(idUsuario);
        insertQuery.addBindValue(tipoCombo->currentText());
        insertQuery.addBindValue(titulo);
        insertQuery.addBindValue(descricao);
        insertQuery.addBindValue(dataEdit->date().toString("yyyy-MM-dd"));
        insertQuery.addBindValue(disciplina);

        if (insertQuery.exec()) {
            qDebug() << "Tarefa inserida com ID:" << insertQuery.lastInsertId().toInt();
            QMessageBox::information(dialog, "✅ Sucesso", "Tarefa adicionada com sucesso!");
            dialog->accept();
            carregarTarefas(ui->filtroComboBox->currentText());
        } else {
            qDebug() << "ERRO ao inserir tarefa:" << insertQuery.lastError().text();
            QMessageBox::critical(dialog, "❌ Erro",
                                  "Erro ao adicionar tarefa: " + insertQuery.lastError().text());
        }
    });

    dialog->exec();
    delete dialog;
}

void AgendaAcademicaDialog::on_filtroComboBox_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    carregarTarefas(ui->filtroComboBox->currentText());
}

void AgendaAcademicaDialog::on_removerTarefaButton_clicked()
{
    // Remoção é feita diretamente nos cards
}

void AgendaAcademicaDialog::on_marcarConcluidaButton_clicked()
{
    // Conclusão é feita diretamente nos cards
}

void AgendaAcademicaDialog::setupNavigationBar()
{
    ui->homeButton->installEventFilter(this);
    ui->perfilButton->installEventFilter(this);
}

bool AgendaAcademicaDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->homeButton && event->type() == QEvent::MouseButtonPress) {
        this->close();
        return true;
    }

    if (obj == ui->perfilButton && event->type() == QEvent::MouseButtonPress) {
        PerfilDialog *perfil = new PerfilDialog(this, loggedInUsername);
        perfil->exec();
        delete perfil;
        return true;
    }

    return QDialog::eventFilter(obj, event);
}
