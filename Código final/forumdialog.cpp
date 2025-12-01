#include "forumdialog.h"
#include "ui_forumdialog.h"
#include "perfildialog.h"
#include "mainwindow.h"
#include "forumpostsdialog.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QScrollArea>
#include <QLineEdit>
#include <QTextEdit>
#include <QDialog>
#include <QEvent>
#include <QMouseEvent>
#include <QComboBox>

// ============================================================================
// CONSTRUTOR
// ============================================================================
ForumDialog::ForumDialog(QWidget *parent, const QString& username)
    : QDialog(parent)
    , ui(new Ui::ForumDialog)
    , loggedInUsername(username)
    , materiaAtual(0)
{
    ui->setupUi(this);
    setWindowTitle("📚 Central de Dúvidas - EducaUTFPR");
    resize(1200, 800);

    setupDatabase();
    criarTabelasNecessarias();

    // Configura o layout principal do container
    layoutPrincipal = new QVBoxLayout(ui->containerBotoes);
    layoutPrincipal->setSpacing(12);
    layoutPrincipal->setContentsMargins(15, 15, 15, 15);
    ui->containerBotoes->setLayout(layoutPrincipal);

    // Configura navegação superior - NOMES CORRIGIDOS
    ui->homeButton->installEventFilter(this);
    ui->perfilButton->installEventFilter(this);

    // Conecta busca
    connect(ui->line_login, &QLineEdit::textChanged,
            this, &ForumDialog::on_searchLine_textChanged);


    // Conecta botão voltar
    connect(ui->voltarButton, &QPushButton::clicked,
            this, &ForumDialog::voltarParaCategorias);

    // Carrega categorias na tela inicial
    carregarCategorias();

    // Atualiza estatísticas
    atualizarEstatisticas();
}

// ============================================================================
// DESTRUTOR
// ============================================================================
ForumDialog::~ForumDialog()
{
    delete ui;
}

// ============================================================================
// SETUP DATABASE
// ============================================================================
void ForumDialog::setupDatabase()
{
    dbConnection = QSqlDatabase::database("qt_sql_default_connection");

    if (!dbConnection.isOpen()) {
        qDebug() << "[ForumDialog] ERRO: Banco de dados não está aberto.";
    }
}

// ============================================================================
// CRIAR TABELAS NECESSÁRIAS
// ============================================================================
void ForumDialog::criarTabelasNecessarias()
{
    QSqlQuery query(dbConnection);

    // Tabela de Categorias de Matérias
    query.exec(
        "CREATE TABLE IF NOT EXISTS Categorias_Materias ("
        "id_categoria INTEGER PRIMARY KEY AUTOINCREMENT, "
        "nome TEXT NOT NULL UNIQUE, "
        "icone TEXT, "
        "cor TEXT)"
        );

    // Tabela de Matérias
    query.exec(
        "CREATE TABLE IF NOT EXISTS Materias_Forum ("
        "id_materia INTEGER PRIMARY KEY AUTOINCREMENT, "
        "id_categoria INTEGER NOT NULL, "
        "nome TEXT NOT NULL, "
        "codigo TEXT, "
        "cor TEXT, "
        "FOREIGN KEY (id_categoria) REFERENCES Categorias_Materias(id_categoria))"
        );

    // Tabela de Dúvidas
    query.exec(
        "CREATE TABLE IF NOT EXISTS Duvidas_Forum ("
        "id_duvida INTEGER PRIMARY KEY AUTOINCREMENT, "
        "id_materia INTEGER NOT NULL, "
        "id_usuario INTEGER NOT NULL, "
        "titulo TEXT NOT NULL, "
        "descricao TEXT NOT NULL, "
        "data_criacao DATETIME DEFAULT CURRENT_TIMESTAMP, "
        "num_curtidas INTEGER DEFAULT 0, "
        "status TEXT DEFAULT 'aberta', "
        "FOREIGN KEY (id_materia) REFERENCES Materias_Forum(id_materia), "
        "FOREIGN KEY (id_usuario) REFERENCES Usuario(id_usuario))"
        );

    // Tabela de Respostas
    query.exec(
        "CREATE TABLE IF NOT EXISTS Respostas_Forum ("
        "id_resposta INTEGER PRIMARY KEY AUTOINCREMENT, "
        "id_duvida INTEGER NOT NULL, "
        "id_usuario INTEGER NOT NULL, "
        "resposta TEXT NOT NULL, "
        "data_resposta DATETIME DEFAULT CURRENT_TIMESTAMP, "
        "num_curtidas INTEGER DEFAULT 0, "
        "melhor_resposta INTEGER DEFAULT 0, "
        "FOREIGN KEY (id_duvida) REFERENCES Duvidas_Forum(id_duvida), "
        "FOREIGN KEY (id_usuario) REFERENCES Usuario(id_usuario))"
        );

    // Tabela de Curtidas em Dúvidas
    query.exec(
        "CREATE TABLE IF NOT EXISTS Curtidas_Duvidas ("
        "id_curtida INTEGER PRIMARY KEY AUTOINCREMENT, "
        "id_duvida INTEGER NOT NULL, "
        "id_usuario INTEGER NOT NULL, "
        "FOREIGN KEY (id_duvida) REFERENCES Duvidas_Forum(id_duvida), "
        "FOREIGN KEY (id_usuario) REFERENCES Usuario(id_usuario), "
        "UNIQUE(id_duvida, id_usuario))"
        );

    // Tabela de Curtidas em Respostas
    query.exec(
        "CREATE TABLE IF NOT EXISTS Curtidas_Respostas ("
        "id_curtida INTEGER PRIMARY KEY AUTOINCREMENT, "
        "id_resposta INTEGER NOT NULL, "
        "id_usuario INTEGER NOT NULL, "
        "FOREIGN KEY (id_resposta) REFERENCES Respostas_Forum(id_resposta), "
        "FOREIGN KEY (id_usuario) REFERENCES Usuario(id_usuario), "
        "UNIQUE(id_resposta, id_usuario))"
        );

    // Verifica se precisa popular dados de exemplo
    query.exec("SELECT COUNT(*) FROM Categorias_Materias");
    if (query.next() && query.value(0).toInt() == 0) {
        popularMateriasExemplo();
    }
}

// ============================================================================
// POPULAR MATÉRIAS DE EXEMPLO
// ============================================================================
void ForumDialog::popularMateriasExemplo()
{
    QSqlQuery query(dbConnection);

    // 1. Inserir Categorias (sem emojis)
    QMap<QString, QPair<QString, QString>> categorias = {
        {"Matemática", {"", "#FF6B6B"}},
        {"Física", {"", "#4ECDC4"}},
        {"Química", {"", "#95E1D3"}},
        {"Programação", {"", "#F38181"}},
        {"Optativas", {"", "#AA96DA"}}
    };

    for (auto it = categorias.begin(); it != categorias.end(); ++it) {
        query.prepare("INSERT INTO Categorias_Materias (nome, icone, cor) VALUES (?, ?, ?)");
        query.addBindValue(it.key());
        query.addBindValue(it.value().first);
        query.addBindValue(it.value().second);
        query.exec();
    }

    // 2. Buscar IDs das categorias
    QMap<QString, int> categoriasIds;
    query.exec("SELECT id_categoria, nome FROM Categorias_Materias");
    while (query.next()) {
        categoriasIds[query.value(1).toString()] = query.value(0).toInt();
    }

    // 3. Inserir Matérias por Categoria

    // MATEMÁTICA
    QStringList matematica = {
        "Cálculo Diferencial e Integral 1",
        "Cálculo Diferencial e Integral 2",
        "Cálculo Diferencial e Integral 3",
        "Álgebra Linear",
        "Geometria Analítica",
        "Matemática Discreta",
        "Equações Diferenciais Ordinárias",
        "Cálculo Numérico",
        "Probabilidade e Estatística"
    };

    for (const QString& mat : matematica) {
        query.prepare("INSERT INTO Materias_Forum (id_categoria, nome, cor) VALUES (?, ?, ?)");
        query.addBindValue(categoriasIds["Matemática"]);
        query.addBindValue(mat);
        query.addBindValue("#FF8787");
        query.exec();
    }

    // FÍSICA
    QStringList fisica = {
        "Física Teórica 1",
        "Física Teórica 2",
        "Física Teórica 3",
        "Física Experimental 1",
        "Física Experimental 2",
        "Análise de Sistemas Lineares"
    };

    for (const QString& mat : fisica) {
        query.prepare("INSERT INTO Materias_Forum (id_categoria, nome, cor) VALUES (?, ?, ?)");
        query.addBindValue(categoriasIds["Física"]);
        query.addBindValue(mat);
        query.addBindValue("#45B7D1");
        query.exec();
    }

    // QUÍMICA
    QStringList quimica = {
        "Química Geral",
        "Química Experimental"
    };

    for (const QString& mat : quimica) {
        query.prepare("INSERT INTO Materias_Forum (id_categoria, nome, cor) VALUES (?, ?, ?)");
        query.addBindValue(categoriasIds["Química"]);
        query.addBindValue(mat);
        query.addBindValue("#7FDBCA");
        query.exec();
    }

    // PROGRAMAÇÃO
    QStringList programacao = {
        "Fundamentos de Programação 1",
        "Fundamentos de Programação 2",
        "Programação Orientada a Objetos",
        "Estrutura de Dados 1",
        "Estrutura de Dados 2",
        "Banco de Dados",
        "Engenharia de Software",
        "Compiladores",
        "Sistemas Operacionais",
        "Redes de Computadores",
        "Desenvolvimento de Aplicações Web",
        "Sistemas Distribuídos",
        "Sistemas Embarcados"
    };

    for (const QString& mat : programacao) {
        query.prepare("INSERT INTO Materias_Forum (id_categoria, nome, cor) VALUES (?, ?, ?)");
        query.addBindValue(categoriasIds["Programação"]);
        query.addBindValue(mat);
        query.addBindValue("#FC9D9A");
        query.exec();
    }

    // OPTATIVAS
    QStringList optativas = {
        "Aptidão Física",
        "Libras 1",
        "Meio Ambiente e Sociedade",
        "Qualidade de Vida",
        "Relações Humanas e Liderança",
        "Comunicação Linguística",
        "Inglês Instrumental",
        "Metodologia de Pesquisa",
        "Ciências do Ambiente",
        "Economia",
        "Empreendedorismo"
    };

    for (const QString& mat : optativas) {
        query.prepare("INSERT INTO Materias_Forum (id_categoria, nome, cor) VALUES (?, ?, ?)");
        query.addBindValue(categoriasIds["Optativas"]);
        query.addBindValue(mat);
        query.addBindValue("#C7BFDD");
        query.exec();
    }

    qDebug() << "✅ Matérias de exemplo populadas com sucesso!";
}

// ============================================================================
// GET ID USUARIO
// ============================================================================
int ForumDialog::getIdUsuario(const QString& username)
{
    QSqlQuery query(dbConnection);
    query.prepare("SELECT id_usuario FROM Usuario WHERE usuario = ?");
    query.addBindValue(username);

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    return -1;
}

// ============================================================================
// ATUALIZAR ESTATÍSTICAS
// ============================================================================
void ForumDialog::atualizarEstatisticas()
{
    QSqlQuery query(dbConnection);

    // Conta total de dúvidas
    query.exec("SELECT COUNT(*) FROM Duvidas_Forum");
    int totalDuvidas = 0;
    if (query.next()) {
        totalDuvidas = query.value(0).toInt();
    }

    // Conta dúvidas respondidas
    query.exec("SELECT COUNT(*) FROM Duvidas_Forum WHERE status = 'respondida'");
    int duvidasRespondidas = 0;
    if (query.next()) {
        duvidasRespondidas = query.value(0).toInt();
    }

    ui->estatisticasLabel->setText(
        QString("📊 %1 dúvidas | ✅ %2 respondidas")
            .arg(totalDuvidas)
            .arg(duvidasRespondidas)
    );
}

// ============================================================================
// CARREGAR CATEGORIAS
// ============================================================================
void ForumDialog::carregarCategorias()
{
    // Limpa o layout
    QLayoutItem *item;
    while ((item = layoutPrincipal->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    // Esconde botão voltar na tela inicial
    ui->voltarButton->setVisible(false);

    // Título
    QLabel *tituloLabel = new QLabel("Selecione uma Categoria de Matérias");
    tituloLabel->setStyleSheet(
        "font-size: 24px; "
        "font-weight: bold; "
        "color: #FFFFFF; "  // Branco para destaque
        "padding: 20px; "
        "background-color: transparent;"
        );
    tituloLabel->setAlignment(Qt::AlignCenter);
    layoutPrincipal->addWidget(tituloLabel);

    // Descrição
    QLabel *descLabel = new QLabel(
        "Navegue pelas categorias abaixo para encontrar dúvidas ou criar uma nova"
        );
    descLabel->setStyleSheet(
        "font-size: 14px; "
        "color: #D3AF35; "
        "padding: 5px 20px 15px 20px; "
        "background-color: transparent;"
        );
    descLabel->setAlignment(Qt::AlignCenter);
    layoutPrincipal->addWidget(descLabel);

    // Linha separadora
    QFrame *line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("background-color: #F4B315; margin: 10px 20px;");
    line->setLineWidth(2);
    layoutPrincipal->addWidget(line);

    // Busca categorias no banco
    QSqlQuery query(dbConnection);
    query.exec("SELECT nome, icone, cor FROM Categorias_Materias ORDER BY nome");

    while (query.next()) {
        QString nome = query.value(0).toString();
        QString icone = query.value(1).toString();
        QString cor = query.value(2).toString();

        QPushButton *btn = criarBotaoCategoria(nome, icone, cor);
        layoutPrincipal->addWidget(btn);
    }

    layoutPrincipal->addStretch();

    // Atualiza estatísticas
    atualizarEstatisticas();
}

// ============================================================================
// CRIAR BOTÃO CATEGORIA
// ============================================================================
QPushButton* ForumDialog::criarBotaoCategoria(const QString& categoria,
                                              const QString& icone,
                                              const QString& cor)
{
    QPushButton *btn = new QPushButton();

    // Conta quantas dúvidas tem nesta categoria
    QSqlQuery countQuery(dbConnection);
    countQuery.prepare(
        "SELECT COUNT(*) FROM Duvidas_Forum d "
        "JOIN Materias_Forum m ON d.id_materia = m.id_materia "
        "JOIN Categorias_Materias c ON m.id_categoria = c.id_categoria "
        "WHERE c.nome = ?"
        );
    countQuery.addBindValue(categoria);
    int numDuvidas = 0;
    if (countQuery.exec() && countQuery.next()) {
        numDuvidas = countQuery.value(0).toInt();
    }

    // Texto sem emoji
    btn->setText(QString("%1\n%2 dúvida(s)").arg(categoria).arg(numDuvidas));
    btn->setMinimumHeight(90);
    btn->setCursor(Qt::PointingHandCursor);

    // Cria cor de hover
    QString corHover = cor;
    if (corHover.contains("FF")) {
        corHover.replace("FF", "DD");
    } else if (corHover.contains("4E")) {
        corHover.replace("4E", "3D");
    } else if (corHover.contains("95")) {
        corHover.replace("95", "75");
    } else if (corHover.contains("F3")) {
        corHover.replace("F3", "D3");
    } else if (corHover.contains("AA")) {
        corHover.replace("AA", "8A");
    }

    btn->setStyleSheet(
        QString("QPushButton {"
                "   background-color: %1;"
                "   color: white;"  // Texto branco fica melhor
                "   border: none;"
                "   border-radius: 15px;"
                "   padding: 20px;"
                "   font-size: 20px;"
                "   font-weight: bold;"
                "   text-align: left;"
                "}"
                "QPushButton:hover {"
                "   background-color: %2;"
                "   transform: scale(1.02);"
                "}").arg(cor).arg(corHover)
        );

    connect(btn, &QPushButton::clicked, [this, categoria]() {
        onCategoriaClicked(categoria);
    });

    return btn;
}

// ============================================================================
// ON CATEGORIA CLICADA
// ============================================================================
void ForumDialog::onCategoriaClicked(const QString& categoria)
{
    categoriaAtual = categoria;
    carregarMateriasDaCategoria(categoria);
}

// ============================================================================
// CARREGAR MATÉRIAS DA CATEGORIA
// ============================================================================
void ForumDialog::carregarMateriasDaCategoria(const QString& categoria)
{
    // Limpa o layout
    QLayoutItem *item;
    while ((item = layoutPrincipal->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    // Mostra botão voltar
    ui->voltarButton->setVisible(true);

    // Busca ícone da categoria
    QSqlQuery iconQuery(dbConnection);
    iconQuery.prepare("SELECT icone FROM Categorias_Materias WHERE nome = ?");
    iconQuery.addBindValue(categoria);
    QString iconeCategoria = "📚";
    if (iconQuery.exec() && iconQuery.next()) {
        iconeCategoria = iconQuery.value(0).toString();
    }

    // Título
    QLabel *tituloLabel = new QLabel(iconeCategoria + " " + categoria);
    tituloLabel->setStyleSheet(
        "font-size: 22px; "
        "font-weight: bold; "
        "color: #F4B315; "
        "padding: 15px;"
        );
    tituloLabel->setAlignment(Qt::AlignCenter);
    layoutPrincipal->addWidget(tituloLabel);

    // Subtítulo
    QLabel *subtituloLabel = new QLabel("Selecione uma matéria para ver as dúvidas");
    subtituloLabel->setStyleSheet(
        "font-size: 13px; "
        "color: #D3AF35; "
        "padding: 5px;"
        );
    subtituloLabel->setAlignment(Qt::AlignCenter);
    layoutPrincipal->addWidget(subtituloLabel);

    // Busca matérias da categoria
    QSqlQuery query(dbConnection);
    query.prepare(
        "SELECT m.id_materia, m.nome, m.cor "
        "FROM Materias_Forum m "
        "JOIN Categorias_Materias c ON m.id_categoria = c.id_categoria "
        "WHERE c.nome = ? "
        "ORDER BY m.nome"
        );
    query.addBindValue(categoria);
    query.exec();

    int count = 0;
    while (query.next()) {
        int id = query.value(0).toInt();
        QString nome = query.value(1).toString();
        QString cor = query.value(2).toString();

        QPushButton *btn = criarBotaoMateria(id, nome, cor);
        layoutPrincipal->addWidget(btn);
        count++;
    }

    if (count == 0) {
        QLabel *emptyLabel = new QLabel("Nenhuma matéria encontrada nesta categoria.");
        emptyLabel->setStyleSheet("color: #8E6915; font-size: 14px; padding: 20px;");
        emptyLabel->setAlignment(Qt::AlignCenter);
        layoutPrincipal->addWidget(emptyLabel);
    }

    layoutPrincipal->addStretch();
}

// ============================================================================
// CRIAR BOTÃO MATÉRIA
// ============================================================================
QPushButton* ForumDialog::criarBotaoMateria(int idMateria, const QString& nome, const QString& cor)
{
    // Conta quantas dúvidas tem nesta matéria
    QSqlQuery countQuery(dbConnection);
    countQuery.prepare("SELECT COUNT(*) FROM Duvidas_Forum WHERE id_materia = ?");
    countQuery.addBindValue(idMateria);
    int numDuvidas = 0;
    if (countQuery.exec() && countQuery.next()) {
        numDuvidas = countQuery.value(0).toInt();
    }

    QPushButton *btn = new QPushButton();
    btn->setText(QString("%1\n💬 %2 dúvida(s)").arg(nome).arg(numDuvidas));
    btn->setMinimumHeight(70);
    btn->setCursor(Qt::PointingHandCursor);

    QString corHover = cor;
    if (corHover.contains("87")) {
        corHover.replace("87", "A0");
    }

    btn->setStyleSheet(
        QString("QPushButton {"
                "   background-color: %1;"
                "   color: white;"
                "   border: none;"
                "   border-radius: 12px;"
                "   padding: 15px;"
                "   font-size: 15px;"
                "   font-weight: bold;"
                "   text-align: left;"
                "}"
                "QPushButton:hover {"
                "   background-color: %2;"
                "}").arg(cor).arg(corHover)
        );

    connect(btn, &QPushButton::clicked, [this, idMateria, nome]() {
        onMateriaClicked(idMateria, nome);
    });

    return btn;
}

// ============================================================================
// ON MATÉRIA CLICADA
// ============================================================================
void ForumDialog::onMateriaClicked(int idMateria, const QString& nomeMateria)
{
    materiaAtual = idMateria;
    carregarDuvidasDaMateria(idMateria);
}

// ============================================================================
// CARREGAR DÚVIDAS DA MATÉRIA
// ============================================================================
void ForumDialog::carregarDuvidasDaMateria(int idMateria)
{
    // Limpa o layout
    QLayoutItem *item;
    while ((item = layoutPrincipal->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    // Busca nome da matéria
    QSqlQuery nomeQuery(dbConnection);
    nomeQuery.prepare("SELECT nome FROM Materias_Forum WHERE id_materia = ?");
    nomeQuery.addBindValue(idMateria);
    QString nomeMateria = "";
    if (nomeQuery.exec() && nomeQuery.next()) {
        nomeMateria = nomeQuery.value(0).toString();
    }

    // Título
    QLabel *tituloLabel = new QLabel("💬 Dúvidas - " + nomeMateria);
    tituloLabel->setStyleSheet(
        "font-size: 18px; "
        "font-weight: bold; "
        "color: #F4B315; "
        "padding: 15px;"
        );
    tituloLabel->setAlignment(Qt::AlignCenter);
    layoutPrincipal->addWidget(tituloLabel);

    // Busca dúvidas
    QSqlQuery query(dbConnection);
    query.prepare(
        "SELECT d.id_duvida, d.titulo, d.descricao, d.data_criacao, "
        "d.num_curtidas, d.status, u.usuario, u.nome, u.Sobrenome, "
        "(SELECT COUNT(*) FROM Respostas_Forum WHERE id_duvida = d.id_duvida) as num_respostas "
        "FROM Duvidas_Forum d "
        "JOIN Usuario u ON d.id_usuario = u.id_usuario "
        "WHERE d.id_materia = ? "
        "ORDER BY d.data_criacao DESC"
        );
    query.addBindValue(idMateria);
    query.exec();

    int count = 0;
    while (query.next()) {
        int idDuvida = query.value(0).toInt();
        QString titulo = query.value(1).toString();
        QString descricao = query.value(2).toString();
        QString dataCriacao = query.value(3).toString();
        int numCurtidas = query.value(4).toInt();
        QString status = query.value(5).toString();
        QString nomeAutor = query.value(7).toString() + " " + query.value(8).toString();
        int numRespostas = query.value(9).toInt();

        // Preview da descrição (primeiras 100 caracteres)
        QString preview = descricao;
        if (preview.length() > 100) {
            preview = preview.left(97) + "...";
        }

        bool jaRespondida = (status == "respondida");

        QFrame *card = criarCardDuvida(idDuvida, titulo, preview, nomeAutor,
                                       dataCriacao, numRespostas, numCurtidas, jaRespondida);
        layoutPrincipal->addWidget(card);
        count++;
    }

    if (count == 0) {
        QLabel *emptyLabel = new QLabel("🔍 Nenhuma dúvida encontrada.\nSeja o primeiro a perguntar!");
        emptyLabel->setStyleSheet(
            "color: #8E6915; "
            "font-size: 16px; "
            "padding: 40px; "
            "background-color: #423738; "
            "border-radius: 10px; "
            "margin: 20px;"
            );
        emptyLabel->setAlignment(Qt::AlignCenter);
        layoutPrincipal->addWidget(emptyLabel);
    }

    layoutPrincipal->addStretch();
}

// ============================================================================
// CRIAR CARD DÚVIDA
// ============================================================================
QFrame* ForumDialog::criarCardDuvida(int idDuvida, const QString& titulo,
                                     const QString& preview, const QString& autor,
                                     const QString& data, int numRespostas,
                                     int numCurtidas, bool jaRespondida)
{
    QFrame *card = new QFrame();
    card->setFrameShape(QFrame::StyledPanel);
    card->setMinimumHeight(120);
    card->setCursor(Qt::PointingHandCursor);

    QString corBorda = jaRespondida ? "#27AE60" : "#F4B315";
    QString corFundo = "#423738";

    card->setStyleSheet(
        QString("QFrame {"
                "   background-color: %1;"
                "   border-left: 5px solid %2;"
                "   border-radius: 12px;"
                "   padding: 15px;"
                "   margin: 5px 0;"
                "}"
                "QFrame:hover {"
                "   background-color: #524447;"
                "   border-left: 6px solid %2;"
                "}").arg(corFundo).arg(corBorda)
        );

    QVBoxLayout *mainLayout = new QVBoxLayout(card);
    mainLayout->setSpacing(8);

    // Linha 1: Título
    QLabel *tituloLabel = new QLabel(titulo);
    tituloLabel->setStyleSheet(
        "font-size: 16px; "
        "font-weight: bold; "
        "color: #F4B315;"
        );
    tituloLabel->setWordWrap(true);
    mainLayout->addWidget(tituloLabel);

    // Linha 2: Preview
    QLabel *previewLabel = new QLabel(preview);
    previewLabel->setStyleSheet(
        "font-size: 12px; "
        "color: #D3AF35;"
        );
    previewLabel->setWordWrap(true);
    mainLayout->addWidget(previewLabel);

    // Linha 3: Informações (autor, data, respostas, curtidas)
    QHBoxLayout *infoLayout = new QHBoxLayout();

    QLabel *autorLabel = new QLabel("👤 " + autor);
    autorLabel->setStyleSheet("font-size: 11px; color: #8E6915;");

    QLabel *dataLabel = new QLabel("📅 " + data.left(10));
    dataLabel->setStyleSheet("font-size: 11px; color: #8E6915;");

    QLabel *respostasLabel = new QLabel(QString("💬 %1").arg(numRespostas));
    respostasLabel->setStyleSheet("font-size: 11px; color: #8E6915;");

    QLabel *curtidasLabel = new QLabel(QString("👍 %1").arg(numCurtidas));
    curtidasLabel->setStyleSheet("font-size: 11px; color: #8E6915;");

    if (jaRespondida) {
        QLabel *statusLabel = new QLabel("✅ Respondida");
        statusLabel->setStyleSheet(
            "font-size: 11px; "
            "color: #27AE60; "
            "font-weight: bold;"
            );
        infoLayout->addWidget(statusLabel);
    }

    infoLayout->addWidget(autorLabel);
    infoLayout->addWidget(dataLabel);
    infoLayout->addWidget(respostasLabel);
    infoLayout->addWidget(curtidasLabel);
    infoLayout->addStretch();

    mainLayout->addLayout(infoLayout);

    // Evento de clique no card
    card->installEventFilter(this);
    card->setProperty("idDuvida", idDuvida);

    return card;
}

// ============================================================================
// ON DÚVIDA CLICADA - Abre janela de detalhes com respostas
// ============================================================================
void ForumDialog::onDuvidaClicked(int idDuvida)
{
    qDebug() << "=== onDuvidaClicked CHAMADO ===" << idDuvida;

    ForumPostsDialog *detalhes = new ForumPostsDialog(this, idDuvida, loggedInUsername);
    detalhes->exec();
    delete detalhes;

    if (materiaAtual > 0) {
        carregarDuvidasDaMateria(materiaAtual);
    }
}
// ============================================================================
// BUSCA
// ============================================================================
void ForumDialog::on_searchLine_textChanged(const QString& texto)
{
    QString busca = texto.trimmed();

    if (busca.isEmpty()) {
        // Se busca vazia, volta para estado anterior
        if (materiaAtual > 0) {
            carregarDuvidasDaMateria(materiaAtual);
        } else if (!categoriaAtual.isEmpty()) {
            carregarMateriasDaCategoria(categoriaAtual);
        } else {
            carregarCategorias();
        }
        return;
    }

    // Busca dúvidas que contenham o texto
    QLayoutItem *item;
    while ((item = layoutPrincipal->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    QLabel *tituloLabel = new QLabel("🔍 Resultados da busca: \"" + busca + "\"");
    tituloLabel->setStyleSheet(
        "font-size: 18px; font-weight: bold; color: #F4B315; padding: 15px;"
        );
    tituloLabel->setAlignment(Qt::AlignCenter);
    layoutPrincipal->addWidget(tituloLabel);

    QSqlQuery query(dbConnection);
    query.prepare(
        "SELECT d.id_duvida, d.titulo, d.descricao, d.data_criacao, "
        "d.num_curtidas, d.status, u.nome, u.Sobrenome, "
        "(SELECT COUNT(*) FROM Respostas_Forum WHERE id_duvida = d.id_duvida) as num_respostas "
        "FROM Duvidas_Forum d "
        "JOIN Usuario u ON d.id_usuario = u.id_usuario "
        "WHERE d.titulo LIKE ? OR d.descricao LIKE ? "
        "ORDER BY d.data_criacao DESC"
        );
    query.addBindValue("%" + busca + "%");
    query.addBindValue("%" + busca + "%");

    int count = 0;
    if (query.exec()) {
        while (query.next()) {
            int idDuvida = query.value(0).toInt();
            QString titulo = query.value(1).toString();
            QString descricao = query.value(2).toString();
            QString dataCriacao = query.value(3).toString();
            int numCurtidas = query.value(4).toInt();
            QString status = query.value(5).toString();
            QString nomeAutor = query.value(6).toString() + " " + query.value(7).toString();
            int numRespostas = query.value(8).toInt();

            QString preview = descricao;
            if (preview.length() > 100) {
                preview = preview.left(97) + "...";
            }

            bool jaRespondida = (status == "respondida");

            QFrame *card = criarCardDuvida(idDuvida, titulo, preview, nomeAutor,
                                           dataCriacao, numRespostas, numCurtidas, jaRespondida);
            layoutPrincipal->addWidget(card);
            count++;
        }
    }

    if (count == 0) {
        QLabel *emptyLabel = new QLabel(
            "🔍 Nenhuma dúvida encontrada.\nTente usar outras palavras-chave."
            );
        emptyLabel->setStyleSheet(
            "color: #8E6915; font-size: 16px; padding: 40px; "
            "background-color: #423738; border-radius: 10px; margin: 20px;"
            );
        emptyLabel->setAlignment(Qt::AlignCenter);
        layoutPrincipal->addWidget(emptyLabel);
    }

    layoutPrincipal->addStretch();
}

// ============================================================================
// VOLTAR PARA CATEGORIAS
// ============================================================================
void ForumDialog::voltarParaCategorias()
{
    materiaAtual = 0;
    categoriaAtual.clear();
    carregarCategorias();
}

// ============================================================================
// EVENT FILTER - Para capturar cliques nos cards e botões de navegação
// ============================================================================
bool ForumDialog::eventFilter(QObject *obj, QEvent *event)
{
    // Clique em cards de dúvida
    if (event->type() == QEvent::MouseButtonPress) {
        QFrame *frame = qobject_cast<QFrame*>(obj);
        if (frame && frame->property("idDuvida").isValid()) {
            int idDuvida = frame->property("idDuvida").toInt();
            onDuvidaClicked(idDuvida);
            return true;
        }

        // Botão Home
        if (obj == ui->homeButton) {
            // Abre a MainWindow
            MainWindow *mainWin = new MainWindow();
            mainWin->show();
            this->close();
            return true;
        }

        // Botão Perfil
        if (obj == ui->perfilButton) {
            // Abre janela de perfil
            PerfilDialog *perfilDialog = new PerfilDialog(this, loggedInUsername);
            perfilDialog->exec();
            delete perfilDialog;
            return true;
        }
    }

    return QDialog::eventFilter(obj, event);
}
