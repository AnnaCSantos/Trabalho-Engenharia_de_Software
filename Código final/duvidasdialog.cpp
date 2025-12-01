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
#include <QSqlRecord>
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
    qDebug() << ">>> PASTA DO BANCO: " << QCoreApplication::applicationDirPath();
    setWindowTitle("📚 Dúvidas EducaUTFPR");
    resize(1200, 800);

    setupDatabase();

    //Garante com que os dados do bc estejam funcionando
    garantirTabelaDisciplinas();
    criarTabelaDuvidas();
    criarTabelaRespostas();
    criarTabelaNotificacoes();

    // 2. Popula o filtro principal usando dados do banco
    popularComboDisciplinas(ui->filtroComboBox, true);

    carregarDuvidas(); // Carrega duvidas que já estão armazenadas no banco de dados
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

//Preenche a tabela de DIsciplinas
void DuvidasDialog::garantirTabelaDisciplinas()
{
    QSqlQuery query(dbConnection);

    // Cria a tabela
    QString createTable =
        "CREATE TABLE IF NOT EXISTS Disciplinas ("
        "id_disciplina INTEGER PRIMARY KEY AUTOINCREMENT, "
        "nome TEXT UNIQUE NOT NULL"
        ")";

    if (!query.exec(createTable)) {
        qDebug() << "Erro tabela Disciplinas:" << query.lastError().text();
        return;
    }

    // Verifica se já tem dados
    query.exec("SELECT COUNT(*) FROM Disciplinas");
    if (query.next() && query.value(0).toInt() > 0) {
        return; // Já tem dados, não faz nada
    }

    qDebug() << "Inserindo disciplinas iniciais no banco...";

    // Função onde adiciona as disciplinas no banco de dados caso a tabela não seja encontrada
    // OBservação: As devidas matérias JÁ ESTÃO INSERIDAS NO BANCO, essa função está sendo criada por segurança
    QStringList materias;
    materias
        // 1º Período
        << "📐 Cálculo Diferencial e Integral 1" << "✏️ Desenho Técnico"
        << "💡 Introdução à Engenharia de Computação" << "💻 Fundamentos de Programação 1"
        << "📏 Geometria Analítica" << "🧠 Introdução à Lógica para Computação"
        << "📝 Comunicação Linguística" << "⚡ Materiais e Equipamentos Elétricos"
        // 2º Período
        << "📊 Álgebra Linear" << "📐 Cálculo Diferencial e Integral 2"
        << "🔌 Circuitos Digitais" << "💻 Fundamentos de Programação 2"
        << "⚛️ Física Teórica 1" << "🌍 Inglês Instrumental"
        << "📚 Metodologia de Pesquisa" << "🎯 Atividades Complementares"
        // 3º Período
        << "🖥️ Arquitetura e Organização de Computadores" << "📐 Cálculo Diferencial e Integral 3"
        << "🌱 Ciências do Ambiente" << "📦 Estrutura de Dados 1"
        << "🔢 Equações Diferenciais Ordinárias" << "🧪 Física Experimental 1"
        << "⚛️ Física Teórica 2" << "🔣 Matemática Discreta"
        // 4º Período
        << "🗄️ Banco de Dados" << "📦 Estrutura de Dados 2"
        << "🧪 Física Experimental 2" << "⚛️ Física Teórica 3"
        << "🎲 Programação Orientada a Objetos" << "🧪 Química Experimental"
        << "⚗️ Química Geral" << "📈 Análise de Sistemas Lineares"
        // 5º Período
        << "🔢 Cálculo Numérico" << "📡 Comunicação de Dados"
        << "⚡ Análise de Circuitos Elétricos 1" << "📊 Probabilidade e Estatística"
        << "💾 Sistemas Operacionais" << "🔧 Sistemas Digitais"
        << "🧮 Teoria da Computação" << "🛠️ Oficina de Integração 1"
        // 6º Período
        << "📝 Compiladores" << "🔌 Eletrônica A" << "🏗️ Engenharia de Software"
        << "🎛️ Fundamentos de Controle" << "📊 Processamento Digital de Sinais"
        << "🌐 Redes de Computadores"
        // 7º Período
        << "🎮 Controle Digital" << "💼 Estágio Curricular Obrigatório"
        << "🔌 Eletrônica B" << "💡 Empreendedorismo"
        << "🔧 Lógica Reconfigurável" << "🤖 Sistemas Inteligentes 1"
        << "⚙️ Sistemas Microcontrolados"
        // 8º Período
        << "🌐 Desenvolvimento de Aplicações Web" << "💰 Economia"
        << "📡 Instrumentação Eletrônica" << "🛠️ Oficina de Integração 2"
        << "☁️ Sistemas Distribuídos" << "🔧 Sistemas Embarcados"
        // 9º Período
        << "🔐 Segurança e Auditoria de Sistemas" << "📄 Trabalho de Conclusão de Curso 1"
        // 10º Período
        << "📄 Trabalho de Conclusão de Curso 2"
        // Optativas
        << "🏃 Aptidão Física" << "👋 Libras 1"
        << "🌍 Meio Ambiente e Sociedade" << "💚 Qualidade de Vida"
        << "🤝 Relações Humanas e Liderança";

    dbConnection.transaction();
    QSqlRecord registro = dbConnection.record("Duvidas");
    qDebug() << "=== COLUNAS QUE O QT ESTÁ VENDO ===";
    for(int i=0; i < registro.count(); ++i) {
        qDebug() << "Coluna" << i << ":" << registro.fieldName(i);
    }
    qDebug() << "==================================";
    QSqlQuery insertQuery(dbConnection);
    insertQuery.prepare("INSERT INTO Disciplinas (nome) VALUES (?)");

    for (const QString &materia : materias) {
        insertQuery.addBindValue(materia);
        insertQuery.exec();
    }
    dbConnection.commit();
}

// --- NOVA FUNÇÃO: Popula ComboBox ---
void DuvidasDialog::popularComboDisciplinas(QComboBox *combo, bool incluirOpcaoTodas)
{
    combo->clear();

    if (incluirOpcaoTodas) {
        combo->addItem("📋 Todas as Disciplinas", 0); // ID 0 = Todas
    }

    QSqlQuery query(dbConnection);
    query.exec("SELECT id_disciplina, nome FROM Disciplinas ORDER BY nome ASC");

    while (query.next()) {
        int id = query.value(0).toInt();
        QString nome = query.value(1).toString();

        // Texto visível = Nome, Dado oculto = ID
        combo->addItem(nome, id);
    }
}

void DuvidasDialog::criarTabelaDuvidas()
{
    QSqlQuery query(dbConnection);
    // AGORA USA id_disciplina (INTEGER) em vez de texto
    QString createTableSQL =
        "CREATE TABLE IF NOT EXISTS Duvidas ("
        "id_duvida INTEGER PRIMARY KEY AUTOINCREMENT, "
        "id_usuario INTEGER NOT NULL, "
        "id_disciplina INTEGER NOT NULL, "
        "titulo TEXT NOT NULL, "
        "descricao TEXT NOT NULL, "
        "imagem_path TEXT, "
        "status TEXT DEFAULT 'Aberta', "
        "data_criacao DATETIME DEFAULT CURRENT_TIMESTAMP, "
        "FOREIGN KEY (id_usuario) REFERENCES USUARIOS(id_usuario), "
        "FOREIGN KEY (id_disciplina) REFERENCES Disciplinas(id_disciplina))";

    if (!query.exec(createTableSQL)) {
        qDebug() << "Erro ao criar tabela Duvidas:" << query.lastError().text();
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
        "FOREIGN KEY (id_usuario) REFERENCES USUARIOS(id_usuario))";

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
        "FOREIGN KEY (id_usuario) REFERENCES USUARIOS(id_usuario), "
        "FOREIGN KEY (id_duvida) REFERENCES Duvidas(id_duvida))";

    if (!query.exec(createTableSQL)) {
        qDebug() << "Erro ao criar tabela Notificacoes:" << query.lastError().text();
    }
}

int DuvidasDialog::getIdUsuario(const QString& username)
{
    QSqlQuery query(dbConnection);
    query.prepare("SELECT id_usuario FROM USUARIOS WHERE usuario = ?");
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
    Q_UNUSED(filtro); // Não usamos mais o texto, usamos o ID do combo

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

    // Faz o JOIN para pegar o nome da disciplina baseado no ID
    QString queryString =
        "SELECT d.id_duvida, d.titulo, d.descricao, d.imagem_path, "
        "d.status, d.data_criacao, u.nome, u.Sobrenome, "
        "disc.nome as nome_disciplina, "
        "(SELECT COUNT(*) FROM Respostas_Duvidas WHERE id_duvida = d.id_duvida) as num_respostas "
        "FROM Duvidas d "
        "JOIN USUARIOS u ON d.id_usuario = u.id_usuario "
        "JOIN Disciplinas disc ON d.id_disciplina = disc.id_disciplina ";

    // Filtra pelo ID atual do ComboBox
    int idFiltro = ui->filtroComboBox->currentData().toInt();

    if (idFiltro > 0) { // 0 é "Todas"
        queryString += "WHERE d.id_disciplina = " + QString::number(idFiltro) + " ";
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
        // Pega o nome vindo do JOIN
        QString disciplina = query.value("nome_disciplina").toString();
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
            "color: #8E6915; font-size: 16px; margin: 50px; padding: 30px; "
            "background-color: #423738; border-radius: 10px;"
            );
        layout->addWidget(emptyLabel);
    }

    layout->addStretch();

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

    // Coluna 1: Ícone
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

    // Coluna 2: Info
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

    // Coluna 3: Status
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

    // Carrega do banco (SEM a opção "Todas")
    popularComboDisciplinas(discCombo, false);

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

    // Variável 'imagemPath' precisa ser capturada pelo lambda depois
    // Usamos um ponteiro inteligente ou uma string estática na classe,
    // mas aqui vamos usar uma variável local e capturá-la por referência no lambda
    // ATENÇÃO: QDialog::exec() é bloqueante, então a referência é segura aqui.
    QString imagemPath;
    // OBS: Como imagemPath é local, precisamos tomar cuidado.
    // O jeito mais seguro no Qt moderno é capturar o ponteiro do label para atualizar o texto.

    // Hack para a string persistir: Vamos usar property do botão ou label
    selecionarImagemBtn->setProperty("path", "");

    connect(selecionarImagemBtn, &QPushButton::clicked, [=, &imagemPath]() {
        // Nota: &imagemPath funciona aqui porque o dialog.exec() bloqueia o escopo
        QString path = QFileDialog::getOpenFileName(nullptr, "Selecionar Imagem", "",
                                                    "Imagens (*.png *.jpg *.jpeg *.bmp)");
        if (!path.isEmpty()) {
            imagemPath = path; // Atualiza a variável local
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

    // CORREÇÃO DOS ERROS DE ESCOPO:
    // Capturamos as variáveis locais necessárias (tituloEdit, descEdit, discCombo) por valor (=)
    // imagemPath capturamos por referência (&) pois ela é modificada pelo outro botão
    connect(salvarBtn, &QPushButton::clicked, [=, &imagemPath]() {
        QString titulo = tituloEdit->text().trimmed();
        QString descricao = descEdit->toPlainText().trimmed();

        // Pega o ID da disciplina (UserData)
        int idDisciplina = discCombo->currentData().toInt();

        if (titulo.isEmpty() || descricao.isEmpty()) {
            QMessageBox::warning(dialog, "⚠️ Campos Obrigatórios",
                                 "Título e Descrição são obrigatórios!");
            return;
        }

        int idUsuario = getIdUsuario(loggedInUsername);

        QSqlQuery insertQuery(dbConnection);
        insertQuery.prepare(
            "INSERT INTO Duvidas (id_usuario, id_disciplina, titulo, descricao, imagem_path) "
            "VALUES (?, ?, ?, ?, ?)"
            );
        insertQuery.addBindValue(idUsuario);
        insertQuery.addBindValue(idDisciplina); // Salva o ID
        insertQuery.addBindValue(titulo);
        insertQuery.addBindValue(descricao);
        insertQuery.addBindValue(imagemPath.isEmpty() ? QVariant() : imagemPath);

        if (insertQuery.exec()) {
            QMessageBox::information(dialog, "✅ Sucesso", "Dúvida adicionada com sucesso!");
            dialog->accept();
            carregarDuvidas(); // Recarrega a lista principal
        } else {
            QMessageBox::critical(dialog, "❌ Erro",
                                  "Erro ao adicionar dúvida: " + insertQuery.lastError().text());
        }
    });

    dialog->exec();
}

void DuvidasDialog::on_filtroComboBox_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    carregarDuvidas();
}

void DuvidasDialog::abrirDetalheDuvida(int idDuvida)
{
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

    QSqlQuery query(dbConnection);
    // JOIN aqui também para pegar o nome da disciplina no detalhe
    query.prepare(
        "SELECT d.titulo, d.descricao, disc.nome, d.imagem_path, d.status, "
        "d.data_criacao, u.nome, u.Sobrenome "
        "FROM Duvidas d "
        "JOIN USUARIOS u ON d.id_usuario = u.id_usuario "
        "JOIN Disciplinas disc ON d.id_disciplina = disc.id_disciplina "
        "WHERE d.id_duvida = ?"
        );
    query.addBindValue(idDuvida);

    if (query.exec() && query.next()) {
        QString titulo = query.value("titulo").toString();
        QString descricao = query.value("descricao").toString();
        QString disciplina = query.value("nome").toString(); // Nome da disciplina
        QString imagemPath = query.value("imagem_path").toString();
        QString status = query.value("status").toString();
        QString dataCriacao = query.value("data_criacao").toString();
        QString nomeAutor = query.value("nome").toString() + " " + query.value("Sobrenome").toString();

        QLabel *tituloLabel = new QLabel("📚 " + titulo);
        tituloLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #F4B315;");
        tituloLabel->setWordWrap(true);

        QLabel *infoLabel = new QLabel(
            QString("👤 %1 | 📖 %2 | 📅 %3 | %4")
                .arg(nomeAutor)
                .arg(disciplina)
                .arg(dataCriacao.left(10))
                .arg(status == "Aberta" ? "🟡 Aberta" : "✅ Respondida")
            );
        infoLabel->setStyleSheet("color: #D3AF35; font-size: 12px;");

        QLabel *descLabel = new QLabel(descricao);
        descLabel->setWordWrap(true);
        descLabel->setStyleSheet("font-size: 14px; color: #F4B315; margin: 10px 0;");

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

        QFrame *line = new QFrame();
        line->setFrameShape(QFrame::HLine);
        line->setStyleSheet("background-color: #F4B315;");
        layout->addWidget(line);

        QLabel *respostasTitle = new QLabel("💬 Respostas:");
        respostasTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #F4B315; margin-top: 10px;");
        layout->addWidget(respostasTitle);

        QScrollArea *scrollRespostas = new QScrollArea();
        scrollRespostas->setWidgetResizable(true);
        QWidget *containerRespostas = new QWidget();
        QVBoxLayout *respostasLayout = new QVBoxLayout(containerRespostas);

        QSqlQuery respostasQuery(dbConnection);
        respostasQuery.prepare(
            "SELECT r.resposta, r.data_resposta, u.nome, u.Sobrenome "
            "FROM Respostas_Duvidas r "
            "JOIN USUARIOS u ON r.id_usuario = u.id_usuario "
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
                QSqlQuery updateStatus(dbConnection);
                updateStatus.prepare("UPDATE Duvidas SET status = 'Respondida' WHERE id_duvida = ?");
                updateStatus.addBindValue(idDuvida);
                updateStatus.exec();

                QSqlQuery nomeQuery(dbConnection);
                nomeQuery.prepare("SELECT nome, Sobrenome FROM USUARIOS WHERE id_usuario = ?");
                nomeQuery.addBindValue(idUsuario);
                if (nomeQuery.exec() && nomeQuery.next()) {
                    QString nomeCompleto = nomeQuery.value(0).toString() + " " +
                                           nomeQuery.value(1).toString();
                    notificarAutor(idDuvida, nomeCompleto);
                }

                QMessageBox::information(detalhes, "✅ Sucesso", "Resposta enviada com sucesso!");
                detalhes->accept();
                carregarDuvidas();
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
    QSqlQuery query(dbConnection);
    query.prepare("SELECT id_usuario FROM Duvidas WHERE id_duvida = ?");
    query.addBindValue(idDuvida);

    if (query.exec() && query.next()) {
        int idAutor = query.value(0).toInt();

        QSqlQuery notifQuery(dbConnection);
        notifQuery.prepare(
            "INSERT INTO Notificacoes (id_usuario, id_duvida, mensagem) "
            "VALUES (?, ?, ?)"
            );
        notifQuery.addBindValue(idAutor);
        notifQuery.addBindValue(idDuvida);
        notifQuery.addBindValue(nomeRespondente + " respondeu sua dúvida!");
        notifQuery.exec();
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
        PerfilDialog *perfil = new PerfilDialog(this, loggedInUsername);
        perfil->exec();
        delete perfil;
        return true;
    }

    return QDialog::eventFilter(obj, event);
}
