#include "duvidasdialog.h"
#include "ui_duvidasdialog.h"
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
#include <QComboBox>
#include <QTextEdit>
#include <QLineEdit>
#include <QFileDialog>
#include <QPixmap>
#include <QEvent>
#include <QMouseEvent>
#include "perfildialog.h"

DuvidasDialog::DuvidasDialog(QWidget *parent, const QString& username)
    : QDialog(parent)
    , ui(new Ui::DuvidasDialog)
    , loggedInUsername(username)
{
    ui->setupUi(this);
    setWindowTitle("📚 Dúvidas EducaUTFPR");
    resize(1200, 800);

    setupDatabase();
    criarTabelaDuvidas();
    criarTabelaRespostas();
    criarTabelaNotificacoes();

    // Popula o filtro com todas as disciplinas da UTFPR - Engenharia de Computação
    ui->filtroComboBox->addItem("📋 Todas as Disciplinas");

    // 1º Período
    ui->filtroComboBox->addItem("📐 Cálculo Diferencial e Integral 1");
    ui->filtroComboBox->addItem("✏️ Desenho Técnico");
    ui->filtroComboBox->addItem("💡 Introdução à Engenharia de Computação");
    ui->filtroComboBox->addItem("💻 Fundamentos de Programação 1");
    ui->filtroComboBox->addItem("📏 Geometria Analítica");
    ui->filtroComboBox->addItem("🧠 Introdução à Lógica para Computação");
    ui->filtroComboBox->addItem("📝 Comunicação Linguística");
    ui->filtroComboBox->addItem("⚡ Materiais e Equipamentos Elétricos");

    // 2º Período
    ui->filtroComboBox->addItem("📊 Álgebra Linear");
    ui->filtroComboBox->addItem("📐 Cálculo Diferencial e Integral 2");
    ui->filtroComboBox->addItem("🔌 Circuitos Digitais");
    ui->filtroComboBox->addItem("💻 Fundamentos de Programação 2");
    ui->filtroComboBox->addItem("⚛️ Física Teórica 1");
    ui->filtroComboBox->addItem("🌍 Inglês Instrumental");
    ui->filtroComboBox->addItem("📚 Metodologia de Pesquisa");
    ui->filtroComboBox->addItem("🎯 Atividades Complementares");

    // 3º Período
    ui->filtroComboBox->addItem("🖥️ Arquitetura e Organização de Computadores");
    ui->filtroComboBox->addItem("📐 Cálculo Diferencial e Integral 3");
    ui->filtroComboBox->addItem("🌱 Ciências do Ambiente");
    ui->filtroComboBox->addItem("📦 Estrutura de Dados 1");
    ui->filtroComboBox->addItem("🔢 Equações Diferenciais Ordinárias");
    ui->filtroComboBox->addItem("🧪 Física Experimental 1");
    ui->filtroComboBox->addItem("⚛️ Física Teórica 2");
    ui->filtroComboBox->addItem("🔣 Matemática Discreta");

    // 4º Período
    ui->filtroComboBox->addItem("🗄️ Banco de Dados");
    ui->filtroComboBox->addItem("📦 Estrutura de Dados 2");
    ui->filtroComboBox->addItem("🧪 Física Experimental 2");
    ui->filtroComboBox->addItem("⚛️ Física Teórica 3");
    ui->filtroComboBox->addItem("🎲 Programação Orientada a Objetos");
    ui->filtroComboBox->addItem("🧪 Química Experimental");
    ui->filtroComboBox->addItem("⚗️ Química Geral");
    ui->filtroComboBox->addItem("📈 Análise de Sistemas Lineares");

    // 5º Período
    ui->filtroComboBox->addItem("🔢 Cálculo Numérico");
    ui->filtroComboBox->addItem("📡 Comunicação de Dados");
    ui->filtroComboBox->addItem("⚡ Análise de Circuitos Elétricos 1");
    ui->filtroComboBox->addItem("📊 Probabilidade e Estatística");
    ui->filtroComboBox->addItem("💾 Sistemas Operacionais");
    ui->filtroComboBox->addItem("🔧 Sistemas Digitais");
    ui->filtroComboBox->addItem("🧮 Teoria da Computação");
    ui->filtroComboBox->addItem("🛠️ Oficina de Integração 1");

    // 6º Período
    ui->filtroComboBox->addItem("📝 Compiladores");
    ui->filtroComboBox->addItem("🔌 Eletrônica A");
    ui->filtroComboBox->addItem("🏗️ Engenharia de Software");
    ui->filtroComboBox->addItem("🎛️ Fundamentos de Controle");
    ui->filtroComboBox->addItem("📊 Processamento Digital de Sinais");
    ui->filtroComboBox->addItem("🌐 Redes de Computadores");

    // 7º Período
    ui->filtroComboBox->addItem("🎮 Controle Digital");
    ui->filtroComboBox->addItem("💼 Estágio Curricular Obrigatório");
    ui->filtroComboBox->addItem("🔌 Eletrônica B");
    ui->filtroComboBox->addItem("💡 Empreendedorismo");
    ui->filtroComboBox->addItem("🔧 Lógica Reconfigurável");
    ui->filtroComboBox->addItem("🤖 Sistemas Inteligentes 1");
    ui->filtroComboBox->addItem("⚙️ Sistemas Microcontrolados");

    // 8º Período
    ui->filtroComboBox->addItem("🌐 Desenvolvimento de Aplicações Web");
    ui->filtroComboBox->addItem("💰 Economia");
    ui->filtroComboBox->addItem("📡 Instrumentação Eletrônica");
    ui->filtroComboBox->addItem("🛠️ Oficina de Integração 2");
    ui->filtroComboBox->addItem("☁️ Sistemas Distribuídos");
    ui->filtroComboBox->addItem("🔧 Sistemas Embarcados");

    // 9º Período
    ui->filtroComboBox->addItem("🔐 Segurança e Auditoria de Sistemas");
    ui->filtroComboBox->addItem("📄 Trabalho de Conclusão de Curso 1");

    // 10º Período
    ui->filtroComboBox->addItem("📄 Trabalho de Conclusão de Curso 2");

    // Optativas
    ui->filtroComboBox->addItem("🏃 Aptidão Física");
    ui->filtroComboBox->addItem("👋 Libras 1");
    ui->filtroComboBox->addItem("🌍 Meio Ambiente e Sociedade");
    ui->filtroComboBox->addItem("💚 Qualidade de Vida");
    ui->filtroComboBox->addItem("🤝 Relações Humanas e Liderança");

    carregarDuvidas();
    setupNavigationBar();

    connect(ui->filtroComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DuvidasDialog::on_filtroComboBox_currentIndexChanged);
}

DuvidasDialog::~DuvidasDialog()
{
    delete ui;
}

void DuvidasDialog::setupDatabase()
{
    dbConnection = QSqlDatabase::database("qt_sql_default_connection");
    if (!dbConnection.isOpen()) {
        qDebug() << "[DuvidasDialog] ERRO: Banco de dados não está aberto.";
    }
}

void DuvidasDialog::criarTabelaDuvidas()
{
    QSqlQuery query(dbConnection);
    QString createTableSQL =
        "CREATE TABLE IF NOT EXISTS Duvidas ("
        "id_duvida INTEGER PRIMARY KEY AUTOINCREMENT, "
        "id_usuario INTEGER NOT NULL, "
        "disciplina TEXT NOT NULL, "
        "titulo TEXT NOT NULL, "
        "descricao TEXT NOT NULL, "
        "imagem_path TEXT, "
        "status TEXT DEFAULT 'Aberta', "
        "data_criacao DATETIME DEFAULT CURRENT_TIMESTAMP, "
        "FOREIGN KEY (id_usuario) REFERENCES Usuario(id_usuario))";

    if (!query.exec(createTableSQL)) {
        qDebug() << "Erro ao criar tabela Duvidas:" << query.lastError().text();
    } else {
        qDebug() << "Tabela Duvidas verificada/criada com sucesso!";
    }
}

void DuvidasDialog::criarTabelaRespostas()
{
    QSqlQuery query(dbConnection);
    QString createTableSQL =
        "CREATE TABLE IF NOT EXISTS Respostas_Duvidas ("
        "id_resposta INTEGER PRIMARY KEY AUTOINCREMENT, "
        "id_duvida INTEGER NOT NULL, "
        "id_usuario INTEGER NOT NULL, "
        "resposta TEXT NOT NULL, "
        "data_resposta DATETIME DEFAULT CURRENT_TIMESTAMP, "
        "FOREIGN KEY (id_duvida) REFERENCES Duvidas(id_duvida), "
        "FOREIGN KEY (id_usuario) REFERENCES Usuario(id_usuario))";

    if (!query.exec(createTableSQL)) {
        qDebug() << "Erro ao criar tabela Respostas_Duvidas:" << query.lastError().text();
    }
}

void DuvidasDialog::criarTabelaNotificacoes()
{
    QSqlQuery query(dbConnection);
    QString createTableSQL =
        "CREATE TABLE IF NOT EXISTS Notificacoes ("
        "id_notificacao INTEGER PRIMARY KEY AUTOINCREMENT, "
        "id_usuario INTEGER NOT NULL, "
        "id_duvida INTEGER NOT NULL, "
        "mensagem TEXT NOT NULL, "
        "lida INTEGER DEFAULT 0, "
        "data_notificacao DATETIME DEFAULT CURRENT_TIMESTAMP, "
        "FOREIGN KEY (id_usuario) REFERENCES Usuario(id_usuario), "
        "FOREIGN KEY (id_duvida) REFERENCES Duvidas(id_duvida))";

    if (!query.exec(createTableSQL)) {
        qDebug() << "Erro ao criar tabela Notificacoes:" << query.lastError().text();
    }
}

int DuvidasDialog::getIdUsuario(const QString& username)
{
    QSqlQuery query(dbConnection);
    query.prepare("SELECT id_usuario FROM Usuario WHERE usuario = ?");
    query.addBindValue(username);

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    return -1;
}

int DuvidasDialog::contarNotificacoesNaoLidas()
{
    int idUsuario = getIdUsuario(loggedInUsername);
    QSqlQuery query(dbConnection);
    query.prepare("SELECT COUNT(*) FROM Notificacoes WHERE id_usuario = ? AND lida = 0");
    query.addBindValue(idUsuario);

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

void DuvidasDialog::carregarDuvidas(const QString& filtro)
{
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
        qDebug() << "Usuário não encontrado!";
        return;
    }

    QString queryString =
        "SELECT d.id_duvida, d.disciplina, d.titulo, d.descricao, d.imagem_path, "
        "d.status, d.data_criacao, u.nome, u.Sobrenome, "
        "(SELECT COUNT(*) FROM Respostas_Duvidas WHERE id_duvida = d.id_duvida) as num_respostas "
        "FROM Duvidas d "
        "JOIN Usuario u ON d.id_usuario = u.id_usuario ";

    QString filtroAtual = ui->filtroComboBox->currentText();

    if (!filtroAtual.contains("Todas")) {
        // Remove o emoji do início (tudo antes do primeiro espaço)
        QString disciplinaLimpa = filtroAtual;
        int primeiroEspaco = disciplinaLimpa.indexOf(' ');
        if (primeiroEspaco != -1) {
            disciplinaLimpa = disciplinaLimpa.mid(primeiroEspaco + 1).trimmed();
        }

        qDebug() << "Filtrando por disciplina:" << disciplinaLimpa;
        queryString += "WHERE d.disciplina = '" + disciplinaLimpa + "' ";
    }

    queryString += "ORDER BY d.data_criacao DESC";

    QSqlQuery query(dbConnection);
    if (!query.exec(queryString)) {
        qDebug() << "Erro ao carregar dúvidas:" << query.lastError().text();
        return;
    }

    int count = 0;
    while (query.next()) {
        int id = query.value("id_duvida").toInt();
        QString disciplina = query.value("disciplina").toString();
        QString titulo = query.value("titulo").toString();
        QString descricao = query.value("descricao").toString();
        QString imagemPath = query.value("imagem_path").toString();
        QString status = query.value("status").toString();
        QString dataCriacao = query.value("data_criacao").toString();
        QString nomeAutor = query.value("nome").toString() + " " + query.value("Sobrenome").toString();
        int numRespostas = query.value("num_respostas").toInt();

        QFrame *duvidaCard = criarCardDuvida(id, disciplina, titulo, descricao,
                                             imagemPath, status, nomeAutor,
                                             dataCriacao, numRespostas);
        layout->addWidget(duvidaCard);
        count++;
    }

    if (count == 0) {
        QLabel *emptyLabel = new QLabel("🔍 Nenhuma dúvida encontrada.\nClique em '➕ Nova Dúvida' para adicionar!");
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

    // Atualiza contador de notificações
    int notificacoes = contarNotificacoesNaoLidas();
    if (notificacoes > 0) {
        ui->tituloLabel->setText(QString("📚 Dúvidas EducaUTFPR 🔔 (%1)").arg(notificacoes));
    }
}

QFrame* DuvidasDialog::criarCardDuvida(int id, const QString& disciplina, const QString& titulo,
                                       const QString& descricao, const QString& imagemPath,
                                       const QString& status, const QString& nomeAutor,
                                       const QString& dataCriacao, int numRespostas)
{
    QFrame *card = new QFrame();
    card->setObjectName(QString::number(id));
    card->setFrameShape(QFrame::StyledPanel);
    card->setMinimumHeight(150);

    QString corBorda = "#F4B315";
    QString corFundo = "#423738";
    QString corTexto = "#F4B315";

    if (status == "Respondida") {
        corBorda = "#8E6915";
        corFundo = "#2A2426";
        corTexto = "#8E6915";
    }

    card->setStyleSheet(
        QString("QFrame {"
                "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
                "                               stop:0 rgba(255, 255, 255, 0.03),"
                "                               stop:1 rgba(255, 255, 255, 0.01));"
                "   border: 1px solid %2;"
                "   border-left: 4px solid %2;"
                "   border-radius: 16px;"
                "   padding: 20px;"
                "   margin: 8px 0px;"
                "}"
                "QFrame:hover {"
                "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
                "                               stop:0 rgba(255, 255, 255, 0.06),"
                "                               stop:1 rgba(255, 255, 255, 0.03));"
                "   border-left: 4px solid %2;"
                "   transform: translateY(-2px);"
                "}").arg(corFundo).arg(corBorda)
        );

    QHBoxLayout *mainLayout = new QHBoxLayout(card);
    mainLayout->setSpacing(15);

    // Coluna 1: Ícone da disciplina
    QVBoxLayout *iconCol = new QVBoxLayout();
    iconCol->setAlignment(Qt::AlignTop);

    QLabel *iconeLabel = new QLabel("📚");
    iconeLabel->setStyleSheet(QString("font-size: 36px; color: %1;").arg(corTexto));
    iconeLabel->setAlignment(Qt::AlignCenter);

    QLabel *discLabel = new QLabel(disciplina);
    discLabel->setStyleSheet(QString("font-size: 10px; font-weight: bold; color: %1;").arg(corTexto));
    discLabel->setAlignment(Qt::AlignCenter);
    discLabel->setWordWrap(true);
    discLabel->setMaximumWidth(80);

    iconCol->addWidget(iconeLabel);
    iconCol->addWidget(discLabel);

    // Coluna 2: Informações
    QVBoxLayout *infoCol = new QVBoxLayout();

    QLabel *tituloLabel = new QLabel(titulo);
    tituloLabel->setStyleSheet(
        QString("font-size: 18px; font-weight: bold; color: %1; margin-bottom: 5px;").arg(corTexto)
        );
    tituloLabel->setWordWrap(true);

    QLabel *autorLabel = new QLabel("👤 " + nomeAutor);
    autorLabel->setStyleSheet("color: #D3AF35; font-size: 12px;");

    QLabel *descLabel = new QLabel(descricao);
    descLabel->setWordWrap(true);
    descLabel->setStyleSheet("color: #8E6915; font-size: 12px; margin-top: 5px;");
    if (descricao.length() > 100) {
        descLabel->setText(descricao.left(97) + "...");
    }

    // Miniatura da imagem se existir
    if (!imagemPath.isEmpty()) {
        QLabel *imgLabel = new QLabel();
        QPixmap pixmap(imagemPath);
        if (!pixmap.isNull()) {
            imgLabel->setPixmap(pixmap.scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            imgLabel->setStyleSheet("margin-top: 5px; border: 2px solid #F4B315; border-radius: 5px;");
        }
        infoCol->addWidget(imgLabel);
    }

    infoCol->addWidget(tituloLabel);
    infoCol->addWidget(autorLabel);
    infoCol->addWidget(descLabel);
    infoCol->addStretch();

    // Coluna 3: Status e Data
    QVBoxLayout *statusCol = new QVBoxLayout();
    statusCol->setAlignment(Qt::AlignTop | Qt::AlignRight);

    QLabel *statusLabel = new QLabel(status == "Aberta" ? "🟡 Aberta" : "✅ Respondida");
    statusLabel->setStyleSheet(
        QString("background-color: %1; color: %2; padding: 5px 10px; "
                "border-radius: 5px; font-size: 11px; font-weight: bold;")
            .arg(status == "Aberta" ? "#4A4020" : "#2A2426")
            .arg(status == "Aberta" ? "#D3AF35" : "#8E6915")
        );

    QLabel *dataLabel = new QLabel("📅 " + dataCriacao.left(10));
    dataLabel->setStyleSheet("color: #8E6915; font-size: 11px; margin-top: 5px;");

    QLabel *respostasLabel = new QLabel(QString("💬 %1 resposta(s)").arg(numRespostas));
    respostasLabel->setStyleSheet("color: #D3AF35; font-size: 11px; margin-top: 5px;");

    statusCol->addWidget(statusLabel);
    statusCol->addWidget(dataLabel);
    statusCol->addWidget(respostasLabel);
    statusCol->addStretch();

    // Coluna 4: Botões
    QVBoxLayout *acoesCol = new QVBoxLayout();
    acoesCol->setAlignment(Qt::AlignTop | Qt::AlignRight);

    QPushButton *verBtn = new QPushButton("👁️ Ver");
    verBtn->setMinimumSize(80, 35);
    verBtn->setStyleSheet(
        "QPushButton { background-color: #F4B315; color: #1A161A; border: none; "
        "border-radius: 6px; padding: 8px 12px; font-weight: bold; font-size: 12px; }"
        "QPushButton:hover { background-color: #D3AF35; }"
        );

    connect(verBtn, &QPushButton::clicked, [this, id]() {
        abrirDetalheDuvida(id);
    });

    acoesCol->addWidget(verBtn);
    acoesCol->addStretch();

    mainLayout->addLayout(iconCol);
    mainLayout->addLayout(infoCol, 1);
    mainLayout->addLayout(statusCol);
    mainLayout->addLayout(acoesCol);

    return card;
}

void DuvidasDialog::on_adicionarDuvidaButton_clicked()
{
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("➕ Nova Dúvida");
    dialog->resize(700, 600);

    dialog->setStyleSheet(
        "QDialog { background-color: #1A161A; }"
        "QLabel { color: #F4B315; font-size: 13px; font-weight: bold; }"
        "QLineEdit, QTextEdit, QComboBox {"
        "   background-color: #423738; color: #F4B315; border: 2px solid #8E6915;"
        "   border-radius: 6px; padding: 8px; font-size: 13px; }"
        "QLineEdit:focus, QTextEdit:focus, QComboBox:focus {"
        "   border-color: #F4B315; }"
        );

    QVBoxLayout *layout = new QVBoxLayout(dialog);
    layout->setSpacing(12);

    QLabel *tituloDialog = new QLabel("📚 Adicionar Nova Dúvida");
    tituloDialog->setStyleSheet("font-size: 20px; color: #F4B315; font-weight: bold; margin-bottom: 10px;");

    QLabel *discLabel = new QLabel("📖 Disciplina:");
    QComboBox *discCombo = new QComboBox();
    discCombo->setMinimumHeight(40);

    // Adiciona todas as disciplinas (sem emoji aqui)
    // 1º Período
    discCombo->addItem("Cálculo Diferencial e Integral 1");
    discCombo->addItem("Desenho Técnico");
    discCombo->addItem("Introdução à Engenharia de Computação");
    discCombo->addItem("Fundamentos de Programação 1");
    discCombo->addItem("Geometria Analítica");
    discCombo->addItem("Introdução à Lógica para Computação");
    discCombo->addItem("Comunicação Linguística");
    discCombo->addItem("Materiais e Equipamentos Elétricos");

    // 2º Período
    discCombo->addItem("Álgebra Linear");
    discCombo->addItem("Cálculo Diferencial e Integral 2");
    discCombo->addItem("Circuitos Digitais");
    discCombo->addItem("Fundamentos de Programação 2");
    discCombo->addItem("Física Teórica 1");
    discCombo->addItem("Inglês Instrumental");
    discCombo->addItem("Metodologia de Pesquisa");
    discCombo->addItem("Atividades Complementares");

    // 3º Período
    discCombo->addItem("Arquitetura e Organização de Computadores");
    discCombo->addItem("Cálculo Diferencial e Integral 3");
    discCombo->addItem("Ciências do Ambiente");
    discCombo->addItem("Estrutura de Dados 1");
    discCombo->addItem("Equações Diferenciais Ordinárias");
    discCombo->addItem("Física Experimental 1");
    discCombo->addItem("Física Teórica 2");
    discCombo->addItem("Matemática Discreta");

    // 4º Período
    discCombo->addItem("Banco de Dados");
    discCombo->addItem("Estrutura de Dados 2");
    discCombo->addItem("Física Experimental 2");
    discCombo->addItem("Física Teórica 3");
    discCombo->addItem("Programação Orientada a Objetos");
    discCombo->addItem("Química Experimental");
    discCombo->addItem("Química Geral");
    discCombo->addItem("Análise de Sistemas Lineares");

    // 5º Período
    discCombo->addItem("Cálculo Numérico");
    discCombo->addItem("Comunicação de Dados");
    discCombo->addItem("Análise de Circuitos Elétricos 1");
    discCombo->addItem("Probabilidade e Estatística");
    discCombo->addItem("Sistemas Operacionais");
    discCombo->addItem("Sistemas Digitais");
    discCombo->addItem("Teoria da Computação");
    discCombo->addItem("Oficina de Integração 1");

    // 6º Período
    discCombo->addItem("Compiladores");
    discCombo->addItem("Eletrônica A");
    discCombo->addItem("Engenharia de Software");
    discCombo->addItem("Fundamentos de Controle");
    discCombo->addItem("Processamento Digital de Sinais");
    discCombo->addItem("Redes de Computadores");

    // 7º Período
    discCombo->addItem("Controle Digital");
    discCombo->addItem("Estágio Curricular Obrigatório");
    discCombo->addItem("Eletrônica B");
    discCombo->addItem("Empreendedorismo");
    discCombo->addItem("Lógica Reconfigurável");
    discCombo->addItem("Sistemas Inteligentes 1");
    discCombo->addItem("Sistemas Microcontrolados");

    // 8º Período
    discCombo->addItem("Desenvolvimento de Aplicações Web");
    discCombo->addItem("Economia");
    discCombo->addItem("Instrumentação Eletrônica");
    discCombo->addItem("Oficina de Integração 2");
    discCombo->addItem("Sistemas Distribuídos");
    discCombo->addItem("Sistemas Embarcados");

    // 9º Período
    discCombo->addItem("Segurança e Auditoria de Sistemas");
    discCombo->addItem("Trabalho de Conclusão de Curso 1");

    // 10º Período
    discCombo->addItem("Trabalho de Conclusão de Curso 2");

    // Optativas
    discCombo->addItem("Aptidão Física");
    discCombo->addItem("Libras 1");
    discCombo->addItem("Meio Ambiente e Sociedade");
    discCombo->addItem("Qualidade de Vida");
    discCombo->addItem("Relações Humanas e Liderança");

    QLabel *tituloLabel = new QLabel("📝 Título:");
    QLineEdit *tituloEdit = new QLineEdit();
    tituloEdit->setPlaceholderText("Ex: Dúvida sobre derivadas");
    tituloEdit->setMinimumHeight(40);

    QLabel *descLabel = new QLabel("📄 Descrição:");
    QTextEdit *descEdit = new QTextEdit();
    descEdit->setPlaceholderText("Descreva sua dúvida em detalhes...");
    descEdit->setMaximumHeight(200);

    QLabel *imagemLabel = new QLabel("🖼️ Imagem (opcional):");
    QPushButton *selecionarImagemBtn = new QPushButton("Selecionar Imagem");
    QLabel *imagemPathLabel = new QLabel("Nenhuma imagem selecionada");
    imagemPathLabel->setStyleSheet("color: #8E6915; font-size: 11px;");

    QString imagemPath;

    connect(selecionarImagemBtn, &QPushButton::clicked, [&imagemPath, imagemPathLabel]() {
        QString path = QFileDialog::getOpenFileName(nullptr, "Selecionar Imagem", "",
                                                    "Imagens (*.png *.jpg *.jpeg *.bmp)");
        if (!path.isEmpty()) {
            imagemPath = path;
            imagemPathLabel->setText("✅ " + QFileInfo(path).fileName());
        }
    });

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
    layout->addWidget(discLabel);
    layout->addWidget(discCombo);
    layout->addWidget(tituloLabel);
    layout->addWidget(tituloEdit);
    layout->addWidget(descLabel);
    layout->addWidget(descEdit);
    layout->addWidget(imagemLabel);
    layout->addWidget(selecionarImagemBtn);
    layout->addWidget(imagemPathLabel);
    layout->addStretch();
    layout->addLayout(btnLayout);

    connect(cancelarBtn, &QPushButton::clicked, dialog, &QDialog::reject);

    connect(salvarBtn, &QPushButton::clicked, [=, &imagemPath]() {
        QString titulo = tituloEdit->text().trimmed();
        QString disciplina = discCombo->currentText();
        QString descricao = descEdit->toPlainText().trimmed();

        if (titulo.isEmpty() || descricao.isEmpty()) {
            QMessageBox::warning(dialog, "⚠️ Campos Obrigatórios",
                                 "Título e Descrição são obrigatórios!");
            return;
        }

        int idUsuario = getIdUsuario(loggedInUsername);

        qDebug() << "=== SALVANDO DÚVIDA ===";
        qDebug() << "ID Usuário:" << idUsuario;
        qDebug() << "Disciplina:" << disciplina;
        qDebug() << "Título:" << titulo;
        qDebug() << "Descrição:" << descricao;
        qDebug() << "Imagem:" << imagemPath;

        QSqlQuery insertQuery(dbConnection);
        insertQuery.prepare(
            "INSERT INTO Duvidas (id_usuario, disciplina, titulo, descricao, imagem_path) "
            "VALUES (?, ?, ?, ?, ?)"
            );
        insertQuery.addBindValue(idUsuario);
        insertQuery.addBindValue(disciplina);
        insertQuery.addBindValue(titulo);
        insertQuery.addBindValue(descricao);
        insertQuery.addBindValue(imagemPath.isEmpty() ? QVariant() : imagemPath);

        if (insertQuery.exec()) {
            int idDuvida = insertQuery.lastInsertId().toInt();
            qDebug() << "✅ Dúvida salva com sucesso! ID:" << idDuvida;

            QMessageBox::information(dialog, "✅ Sucesso", "Dúvida adicionada com sucesso!");
            dialog->accept();
            carregarDuvidas(ui->filtroComboBox->currentText());
        } else {
            qDebug() << "❌ Erro ao salvar:" << insertQuery.lastError().text();
            QMessageBox::critical(dialog, "❌ Erro",
                                  "Erro ao adicionar dúvida: " + insertQuery.lastError().text());
        }
    });

    dialog->exec();
}

void DuvidasDialog::on_filtroComboBox_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    carregarDuvidas(ui->filtroComboBox->currentText());
}

void DuvidasDialog::abrirDetalheDuvida(int idDuvida)
{
    // Cria dialog para mostrar detalhes e respostas
    QDialog *detalhes = new QDialog(this);
    detalhes->setWindowTitle("📖 Detalhes da Dúvida");
    detalhes->resize(800, 600);
    detalhes->setStyleSheet(
        "QDialog { background-color: #1A161A; }"
        "QLabel { color: #F4B315; }"
        "QTextEdit { background-color: #423738; color: #F4B315; border: 2px solid #8E6915; "
        "border-radius: 6px; padding: 8px; }"
        );

    QVBoxLayout *layout = new QVBoxLayout(detalhes);

    // Busca os dados da dúvida
    QSqlQuery query(dbConnection);
    query.prepare(
        "SELECT d.titulo, d.descricao, d.disciplina, d.imagem_path, d.status, "
        "d.data_criacao, u.nome, u.Sobrenome "
        "FROM Duvidas d "
        "JOIN Usuario u ON d.id_usuario = u.id_usuario "
        "WHERE d.id_duvida = ?"
        );
    query.addBindValue(idDuvida);

    if (query.exec() && query.next()) {
        QString titulo = query.value("titulo").toString();
        QString descricao = query.value("descricao").toString();
        QString disciplina = query.value("disciplina").toString();
        QString imagemPath = query.value("imagem_path").toString();
        QString status = query.value("status").toString();
        QString dataCriacao = query.value("data_criacao").toString();
        QString nomeAutor = query.value("nome").toString() + " " + query.value("Sobrenome").toString();

        // Título
        QLabel *tituloLabel = new QLabel("📚 " + titulo);
        tituloLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #F4B315;");
        tituloLabel->setWordWrap(true);

        // Info
        QLabel *infoLabel = new QLabel(
            QString("👤 %1 | 📖 %2 | 📅 %3 | %4")
                .arg(nomeAutor)
                .arg(disciplina)
                .arg(dataCriacao.left(10))
                .arg(status == "Aberta" ? "🟡 Aberta" : "✅ Respondida")
            );
        infoLabel->setStyleSheet("color: #D3AF35; font-size: 12px;");

        // Descrição
        QLabel *descLabel = new QLabel(descricao);
        descLabel->setWordWrap(true);
        descLabel->setStyleSheet("font-size: 14px; color: #F4B315; margin: 10px 0;");

        // Imagem se existir
        if (!imagemPath.isEmpty()) {
            QLabel *imgLabel = new QLabel();
            QPixmap pixmap(imagemPath);
            if (!pixmap.isNull()) {
                imgLabel->setPixmap(pixmap.scaled(400, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                imgLabel->setAlignment(Qt::AlignCenter);
                imgLabel->setStyleSheet("margin: 10px; border: 2px solid #F4B315; border-radius: 5px;");
                layout->addWidget(imgLabel);
            }
        }

        layout->addWidget(tituloLabel);
        layout->addWidget(infoLabel);
        layout->addWidget(descLabel);

        // Linha separadora
        QFrame *line = new QFrame();
        line->setFrameShape(QFrame::HLine);
        line->setStyleSheet("background-color: #F4B315;");
        layout->addWidget(line);

        // Respostas
        QLabel *respostasTitle = new QLabel("💬 Respostas:");
        respostasTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #F4B315; margin-top: 10px;");
        layout->addWidget(respostasTitle);

        QScrollArea *scrollRespostas = new QScrollArea();
        scrollRespostas->setWidgetResizable(true);
        QWidget *containerRespostas = new QWidget();
        QVBoxLayout *respostasLayout = new QVBoxLayout(containerRespostas);

        // Busca respostas
        QSqlQuery respostasQuery(dbConnection);
        respostasQuery.prepare(
            "SELECT r.resposta, r.data_resposta, u.nome, u.Sobrenome "
            "FROM Respostas_Duvidas r "
            "JOIN Usuario u ON r.id_usuario = u.id_usuario "
            "WHERE r.id_duvida = ? "
            "ORDER BY r.data_resposta ASC"
            );
        respostasQuery.addBindValue(idDuvida);

        int numRespostas = 0;
        if (respostasQuery.exec()) {
            while (respostasQuery.next()) {
                QString resposta = respostasQuery.value("resposta").toString();
                QString dataResp = respostasQuery.value("data_resposta").toString();
                QString nomeResp = respostasQuery.value("nome").toString() + " " +
                                   respostasQuery.value("Sobrenome").toString();

                QFrame *respostaFrame = new QFrame();
                respostaFrame->setStyleSheet(
                    "background-color: #423738; border-radius: 8px; padding: 10px; margin: 5px;"
                    );
                QVBoxLayout *respostaLayout = new QVBoxLayout(respostaFrame);

                QLabel *autorResp = new QLabel("👤 " + nomeResp + " - " + dataResp.left(10));
                autorResp->setStyleSheet("color: #D3AF35; font-size: 11px;");

                QLabel *textoResp = new QLabel(resposta);
                textoResp->setWordWrap(true);
                textoResp->setStyleSheet("color: #F4B315; font-size: 13px;");

                respostaLayout->addWidget(autorResp);
                respostaLayout->addWidget(textoResp);
                respostasLayout->addWidget(respostaFrame);
                numRespostas++;
            }
        }

        if (numRespostas == 0) {
            QLabel *semRespostas = new QLabel("Ainda não há respostas. Seja o primeiro a responder!");
            semRespostas->setAlignment(Qt::AlignCenter);
            semRespostas->setStyleSheet("color: #8E6915; margin: 20px;");
            respostasLayout->addWidget(semRespostas);
        }

        respostasLayout->addStretch();
        containerRespostas->setLayout(respostasLayout);
        scrollRespostas->setWidget(containerRespostas);
        layout->addWidget(scrollRespostas);

        // Campo para adicionar resposta
        QLabel *adicionarRespLabel = new QLabel("✍️ Adicionar sua resposta:");
        adicionarRespLabel->setStyleSheet("font-weight: bold; margin-top: 10px;");
        layout->addWidget(adicionarRespLabel);

        QTextEdit *respostaEdit = new QTextEdit();
        respostaEdit->setPlaceholderText("Digite sua resposta...");
        respostaEdit->setMaximumHeight(100);
        layout->addWidget(respostaEdit);

        QPushButton *enviarBtn = new QPushButton("📤 Enviar Resposta");
        enviarBtn->setStyleSheet(
            "background-color: #F4B315; color: #1A161A; padding: 10px; "
            "border-radius: 8px; font-weight: bold;"
            );

        connect(enviarBtn, &QPushButton::clicked, [=]() {
            QString respostaTexto = respostaEdit->toPlainText().trimmed();
            if (respostaTexto.isEmpty()) {
                QMessageBox::warning(detalhes, "⚠️ Campo vazio", "Digite uma resposta antes de enviar!");
                return;
            }

            int idUsuario = getIdUsuario(loggedInUsername);

            QSqlQuery insertResp(dbConnection);
            insertResp.prepare(
                "INSERT INTO Respostas_Duvidas (id_duvida, id_usuario, resposta) "
                "VALUES (?, ?, ?)"
                );
            insertResp.addBindValue(idDuvida);
            insertResp.addBindValue(idUsuario);
            insertResp.addBindValue(respostaTexto);

            if (insertResp.exec()) {
                // Atualiza status da dúvida
                QSqlQuery updateStatus(dbConnection);
                updateStatus.prepare("UPDATE Duvidas SET status = 'Respondida' WHERE id_duvida = ?");
                updateStatus.addBindValue(idDuvida);
                updateStatus.exec();

                // Notifica o autor
                QSqlQuery nomeQuery(dbConnection);
                nomeQuery.prepare("SELECT nome, Sobrenome FROM Usuario WHERE id_usuario = ?");
                nomeQuery.addBindValue(idUsuario);
                if (nomeQuery.exec() && nomeQuery.next()) {
                    QString nomeCompleto = nomeQuery.value(0).toString() + " " +
                                           nomeQuery.value(1).toString();
                    notificarAutor(idDuvida, nomeCompleto);
                }

                QMessageBox::information(detalhes, "✅ Sucesso", "Resposta enviada com sucesso!");
                detalhes->accept();
                carregarDuvidas(ui->filtroComboBox->currentText());
            } else {
                QMessageBox::critical(detalhes, "❌ Erro",
                                      "Erro ao enviar resposta: " + insertResp.lastError().text());
            }
        });

        layout->addWidget(enviarBtn);
    }

    detalhes->exec();
    delete detalhes;
}

void DuvidasDialog::notificarAutor(int idDuvida, const QString& nomeRespondente)
{
    // Busca o autor da dúvida
    QSqlQuery query(dbConnection);
    query.prepare("SELECT id_usuario FROM Duvidas WHERE id_duvida = ?");
    query.addBindValue(idDuvida);

    if (query.exec() && query.next()) {
        int idAutor = query.value(0).toInt();

        // Cria notificação
        QSqlQuery notifQuery(dbConnection);
        notifQuery.prepare(
            "INSERT INTO Notificacoes (id_usuario, id_duvida, mensagem) "
            "VALUES (?, ?, ?)"
            );
        notifQuery.addBindValue(idAutor);
        notifQuery.addBindValue(idDuvida);
        notifQuery.addBindValue(nomeRespondente + " respondeu sua dúvida!");

        if (notifQuery.exec()) {
            qDebug() << "Notificação criada com sucesso!";
        }
    }
}

void DuvidasDialog::setupNavigationBar()
{
    ui->homeButton->installEventFilter(this);
    ui->perfilButton->installEventFilter(this);
}

bool DuvidasDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->homeButton && event->type() == QEvent::MouseButtonPress) {
        this->close();
        return true;
    }

    if (obj == ui->perfilButton && event->type() == QEvent::MouseButtonPress) {
        // Abre a janela de perfil
        PerfilDialog *perfil = new PerfilDialog(this, loggedInUsername);
        perfil->exec();
        delete perfil;
        return true;
    }

    return QDialog::eventFilter(obj, event);
}
