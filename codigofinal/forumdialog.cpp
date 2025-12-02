#include "forumdialog.h"
#include "ui_forumdialog.h"
#include "perfildialog.h"
#include "mainwindow.h"
#include "forumpostsdialog.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
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
    , disciplinaAtual(0)
{
    ui->setupUi(this);
    setWindowTitle("📚 Central de Dúvidas - EducaUTFPR");
    resize(1200, 800);

    setupDatabase();

    criarTabelasDeCurtidas();

    layoutPrincipal = new QVBoxLayout(ui->containerBotoes);
    layoutPrincipal->setSpacing(12);
    layoutPrincipal->setContentsMargins(15, 15, 15, 15);
    ui->containerBotoes->setLayout(layoutPrincipal);

    ui->homeButton->installEventFilter(this);
    ui->perfilButton->installEventFilter(this);

    connect(ui->line_login, &QLineEdit::textChanged, this, &ForumDialog::on_searchLine_textChanged);
    connect(ui->voltarButton, &QPushButton::clicked, this, &ForumDialog::voltarParaCategorias);

    carregarCategorias();
    atualizarEstatisticas();
}

ForumDialog::~ForumDialog()
{
    delete ui;
}

void ForumDialog::setupDatabase()
{
    dbConnection = QSqlDatabase::database("qt_sql_default_connection");
    if (!dbConnection.isOpen()) {
        qDebug() << "[ForumDialog] ERRO: Banco de dados não está aberto.";
    }
}

// ============================================================================
// TABELAS DE CURTIDAS (Integradas com 'Duvidas' e 'Respostas_Duvidas')
// ============================================================================
void ForumDialog::criarTabelasDeCurtidas()
{
    QSqlQuery query(dbConnection);

    // Tabela de Curtidas em Dúvidas (Usa 'Duvidas')
    query.exec(
        "CREATE TABLE IF NOT EXISTS Curtidas_Duvidas ("
        "id_curtida INTEGER PRIMARY KEY AUTOINCREMENT, "
        "id_duvida INTEGER NOT NULL, "
        "id_usuario INTEGER NOT NULL, "
        "FOREIGN KEY (id_duvida) REFERENCES Duvidas(id_duvida), "
        "FOREIGN KEY (id_usuario) REFERENCES USUARIOS(id_usuario), "
        "UNIQUE(id_duvida, id_usuario))"
        );

    // Tabela de Curtidas em Respostas (Usa 'Respostas_Duvidas')
    query.exec(
        "CREATE TABLE IF NOT EXISTS Curtidas_Respostas ("
        "id_curtida INTEGER PRIMARY KEY AUTOINCREMENT, "
        "id_resposta INTEGER NOT NULL, "
        "id_usuario INTEGER NOT NULL, "
        "FOREIGN KEY (id_resposta) REFERENCES Respostas_Duvidas(id_resposta), "
        "FOREIGN KEY (id_usuario) REFERENCES USUARIOS(id_usuario), "
        "UNIQUE(id_resposta, id_usuario))"
        );
}

int ForumDialog::getIdUsuario(const QString& username)
{
    QSqlQuery query(dbConnection);
    query.prepare("SELECT id_usuario FROM USUARIOS WHERE usuario = ?");
    query.addBindValue(username);

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    return -1;
}

void ForumDialog::atualizarEstatisticas()
{
    QSqlQuery query(dbConnection);

    // Busca na tabela oficial 'Duvidas'
    query.exec("SELECT COUNT(*) FROM Duvidas");
    int totalDuvidas = 0;
    if (query.next()) totalDuvidas = query.value(0).toInt();

    query.exec("SELECT COUNT(*) FROM Duvidas WHERE status = 'Respondida'");
    int duvidasRespondidas = 0;
    if (query.next()) duvidasRespondidas = query.value(0).toInt();

    ui->estatisticasLabel->setText(QString("📊 %1 dúvidas | ✅ %2 respondidas").arg(totalDuvidas).arg(duvidasRespondidas));
}

// ============================================================================
// CARREGAR CATEGORIAS (Botões fixos que filtram a tabela Disciplinas)
// ============================================================================
void ForumDialog::carregarCategorias()
{
    QLayoutItem *item;
    while ((item = layoutPrincipal->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    ui->voltarButton->setVisible(false);

    QLabel *tituloLabel = new QLabel("Selecione uma Área de Conhecimento");
    tituloLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #FFFFFF; padding: 20px; background: transparent;");
    tituloLabel->setAlignment(Qt::AlignCenter);
    layoutPrincipal->addWidget(tituloLabel);

    QFrame *line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("background-color: #F4B315; margin: 10px 20px;");
    layoutPrincipal->addWidget(line);

    // Lista fixa de categorias para navegação
    QStringList categorias = {
        "Matemática", "Física", "Química", "Programação",
        "Banco de Dados", "Redes e Sistemas", "Engenharia",
        "Humanas e Sociais", "TCC e Estágio"
    };

    for (const QString &cat : categorias) {
        QPushButton *btn = criarBotaoCategoria(cat);
        layoutPrincipal->addWidget(btn);
    }

    layoutPrincipal->addStretch();
    atualizarEstatisticas();
}

// ============================================================================
// AUXILIAR: Mapeia Nome da Disciplina -> Categoria
// ============================================================================
QString ForumDialog::identificarCategoria(const QString& nome)
{
    // Lógica simples para categorizar as disciplinas do banco
    if (nome.contains("Cálculo") || nome.contains("Álgebra") || nome.contains("Matemática") || nome.contains("Estatística") || nome.contains("Equações") || nome.contains("Geometria")) return "Matemática";
    if (nome.contains("Física") || nome.contains("Lineares")) return "Física";
    if (nome.contains("Química")) return "Química";
    if (nome.contains("Programação") || nome.contains("Algoritmos") || nome.contains("Estrutura") || nome.contains("Compiladores") || nome.contains("Teoria da Computação") || nome.contains("Lógica para") || nome.contains("Software") || nome.contains("Web") || nome.contains("Inteligentes")) return "Programação";
    if (nome.contains("Banco de Dados")) return "Banco de Dados";
    if (nome.contains("Sistemas Operacionais") || nome.contains("Redes") || nome.contains("Comunicação") || nome.contains("Distribuídos") || nome.contains("Segurança")) return "Redes e Sistemas";
    if (nome.contains("Engenharia") || nome.contains("Arquitetura") || nome.contains("Circuitos") || nome.contains("Eletrônica") || nome.contains("Digitais") || nome.contains("Elétricos") || nome.contains("Desenho") || nome.contains("Controle") || nome.contains("Microcontrolados") || nome.contains("Embarcados") || nome.contains("Instrumentação") || nome.contains("Sinais") || nome.contains("Oficina")) return "Engenharia";
    if (nome.contains("TCC") || nome.contains("Estágio") || nome.contains("Trabalho de Conclusão")) return "TCC e Estágio";

    return "Humanas e Sociais"; // O resto cai aqui (Inglês, Economia, etc)
}

QPair<QString, QString> ForumDialog::getEstiloCategoria(const QString& categoria)
{
    if (categoria == "Matemática") return {"📐", "#F4B315"};
    if (categoria == "Física") return {"⚛️", "#4A90E2"};
    if (categoria == "Química") return {"⚗️", "#1ABC9C"};
    if (categoria == "Programação") return {"💻", "#E74C3C"};
    if (categoria == "Banco de Dados") return {"🗄️", "#9B59B6"};
    if (categoria == "Redes e Sistemas") return {"🌐", "#16A085"};
    if (categoria == "Engenharia") return {"🔧", "#E67E22"};
    if (categoria == "TCC e Estágio") return {"📄", "#5DADE2"};
    return {"📚", "#34495E"}; // Humanas e Outros
}

QPushButton* ForumDialog::criarBotaoCategoria(const QString& categoria)
{
    QPushButton *btn = new QPushButton();
    QPair<QString, QString> estilo = getEstiloCategoria(categoria);

    btn->setText(QString("%1  %2").arg(estilo.first).arg(categoria));
    btn->setMinimumHeight(80);
    btn->setCursor(Qt::PointingHandCursor);

    btn->setStyleSheet(
        QString("QPushButton { background-color: %1; color: white; border: none; border-radius: 15px; padding: 20px; font-size: 20px; font-weight: bold; text-align: left; } QPushButton:hover { transform: scale(1.02); }").arg(estilo.second)
        );

    connect(btn, &QPushButton::clicked, [this, categoria]() {
        onCategoriaClicked(categoria);
    });

    return btn;
}

void ForumDialog::onCategoriaClicked(const QString& categoria)
{
    categoriaAtual = categoria;
    carregarDisciplinasDaCategoria(categoria);
}

// ============================================================================
// CARREGAR DISCIPLINAS (Lendo da tabela 'Disciplinas')
// ============================================================================
void ForumDialog::carregarDisciplinasDaCategoria(const QString& categoria)
{
    QLayoutItem *item;
    while ((item = layoutPrincipal->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    ui->voltarButton->setVisible(true);

    QPair<QString, QString> estilo = getEstiloCategoria(categoria);
    QLabel *tituloLabel = new QLabel(estilo.first + " " + categoria);
    tituloLabel->setStyleSheet("font-size: 22px; font-weight: bold; color: #F4B315; padding: 15px;");
    tituloLabel->setAlignment(Qt::AlignCenter);
    layoutPrincipal->addWidget(tituloLabel);

    // Busca TODAS as disciplinas e filtra pelo código C++ (já que o banco não tem coluna categoria)
    QSqlQuery query(dbConnection);
    query.exec("SELECT id_disciplina, nome FROM Disciplinas ORDER BY nome");

    int count = 0;
    while (query.next()) {
        int id = query.value(0).toInt();
        QString nome = query.value(1).toString();

        // Filtra usando a função auxiliar
        if (identificarCategoria(nome) == categoria) {
            QPushButton *btn = criarBotaoDisciplina(id, nome);
            layoutPrincipal->addWidget(btn);
            count++;
        }
    }

    if (count == 0) {
        QLabel *empty = new QLabel("Nenhuma disciplina encontrada nesta área.");
        empty->setStyleSheet("color: #8E6915; padding: 20px;");
        empty->setAlignment(Qt::AlignCenter);
        layoutPrincipal->addWidget(empty);
    }

    layoutPrincipal->addStretch();
}

QPushButton* ForumDialog::criarBotaoDisciplina(int id, const QString& nome)
{
    // Conta dúvidas desta disciplina na tabela REAL 'Duvidas'
    QSqlQuery countQuery(dbConnection);
    countQuery.prepare("SELECT COUNT(*) FROM Duvidas WHERE id_disciplina = ?");
    countQuery.addBindValue(id);
    int num = 0;
    if (countQuery.exec() && countQuery.next()) num = countQuery.value(0).toInt();

    QPushButton *btn = new QPushButton();
    btn->setText(QString("📘 %1\n📝 %2 dúvida(s)").arg(nome).arg(num));
    btn->setMinimumHeight(70);
    btn->setCursor(Qt::PointingHandCursor);

    btn->setStyleSheet(
        "QPushButton { background-color: #423738; color: white; border: 2px solid #F4B315; border-radius: 12px; padding: 15px; font-size: 15px; font-weight: bold; text-align: left; } QPushButton:hover { background-color: #524447; }"
        );

    connect(btn, &QPushButton::clicked, [this, id, nome]() {
        onMateriaClicked(id, nome);
    });

    return btn;
}

void ForumDialog::onMateriaClicked(int idDisciplina, const QString& nomeDisciplina)
{
    disciplinaAtual = idDisciplina;
    carregarDuvidasDaDisciplina(idDisciplina);
}

// ============================================================================
// CARREGAR DÚVIDAS (Da tabela 'Duvidas')
// ============================================================================
void ForumDialog::carregarDuvidasDaDisciplina(int idDisciplina)
{
    QLayoutItem *item;
    while ((item = layoutPrincipal->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    // Busca nome (apenas para exibir no título)
    QSqlQuery nomeQuery(dbConnection);
    nomeQuery.prepare("SELECT nome FROM Disciplinas WHERE id_disciplina = ?");
    nomeQuery.addBindValue(idDisciplina);
    QString nomeDisc = "Disciplina";
    if (nomeQuery.exec() && nomeQuery.next()) nomeDisc = nomeQuery.value(0).toString();

    QLabel *tituloLabel = new QLabel("💬 Dúvidas - " + nomeDisc);
    tituloLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #F4B315; padding: 15px;");
    tituloLabel->setAlignment(Qt::AlignCenter);
    layoutPrincipal->addWidget(tituloLabel);

    // Query na tabela 'Duvidas'
    QSqlQuery query(dbConnection);
    query.prepare(
        "SELECT d.id_duvida, d.titulo, d.descricao, d.data_criacao, "
        "d.status, u.nome, u.Sobrenome, "
        "(SELECT COUNT(*) FROM Respostas_Duvidas WHERE id_duvida = d.id_duvida) as num_respostas "
        "FROM Duvidas d "
        "JOIN USUARIOS u ON d.id_usuario = u.id_usuario "
        "WHERE d.id_disciplina = ? "
        "ORDER BY d.data_criacao DESC"
        );
    query.addBindValue(idDisciplina);
    query.exec();

    int count = 0;
    while (query.next()) {
        int id = query.value("id_duvida").toInt();
        QString titulo = query.value("titulo").toString();
        QString desc = query.value("descricao").toString();
        QString data = query.value("data_criacao").toString();
        QString status = query.value("status").toString();
        QString autor = query.value("nome").toString() + " " + query.value("Sobrenome").toString();
        int resps = query.value("num_respostas").toInt();

        // Simulação de curtidas (já que a tabela Duvidas original não tem essa coluna, pegamos da tabela auxiliar se existir ou mostramos 0)
        // Para simplificar e evitar erros de SQL, vamos mostrar 0 ou implementar um count na tabela Curtidas_Duvidas
        QSqlQuery likeQuery(dbConnection);
        likeQuery.prepare("SELECT COUNT(*) FROM Curtidas_Duvidas WHERE id_duvida = ?");
        likeQuery.addBindValue(id);
        int curtidas = 0;
        if(likeQuery.exec() && likeQuery.next()) curtidas = likeQuery.value(0).toInt();

        if (desc.length() > 80) desc = desc.left(77) + "...";

        QFrame *card = criarCardDuvida(id, titulo, desc, autor, data, resps, curtidas, status == "Respondida");
        layoutPrincipal->addWidget(card);
        count++;
    }

    if (count == 0) {
        QLabel *empty = new QLabel("Nenhuma dúvida encontrada nesta disciplina.");
        empty->setStyleSheet("color: #8E6915; font-size: 16px; padding: 40px; background: #423738; border-radius: 10px; margin: 20px;");
        empty->setAlignment(Qt::AlignCenter);
        layoutPrincipal->addWidget(empty);
    }

    layoutPrincipal->addStretch();
}

QFrame* ForumDialog::criarCardDuvida(int idDuvida, const QString& titulo, const QString& preview,
                                     const QString& autor, const QString& data,
                                     int numRespostas, int numCurtidas, bool jaRespondida)
{
    QFrame *card = new QFrame();
    card->setFrameShape(QFrame::StyledPanel);
    card->setMinimumHeight(120);
    card->setCursor(Qt::PointingHandCursor);

    QString corBorda = jaRespondida ? "#27AE60" : "#F4B315";

    card->setStyleSheet(QString(
                            "QFrame { background-color: #423738; border-left: 5px solid %1; border-radius: 12px; padding: 15px; margin: 5px 0; }"
                            "QFrame:hover { background-color: #524447; }"
                            ).arg(corBorda));

    QVBoxLayout *mainLayout = new QVBoxLayout(card);

    QLabel *lblTitulo = new QLabel(titulo);
    lblTitulo->setStyleSheet("font-size: 16px; font-weight: bold; color: #F4B315; border: none;");
    lblTitulo->setWordWrap(true);

    QLabel *lblPreview = new QLabel(preview);
    lblPreview->setStyleSheet("font-size: 12px; color: #D3AF35; border: none;");
    lblPreview->setWordWrap(true);

    QLabel *lblInfo = new QLabel(QString("👤 %1 | 📅 %2 | 💬 %3 | 👍 %4")
                                     .arg(autor).arg(data.left(10)).arg(numRespostas).arg(numCurtidas));
    lblInfo->setStyleSheet("font-size: 11px; color: #8E6915; border: none;");

    mainLayout->addWidget(lblTitulo);
    mainLayout->addWidget(lblPreview);
    mainLayout->addWidget(lblInfo);

    card->installEventFilter(this);
    card->setProperty("idDuvida", idDuvida);

    return card;
}

void ForumDialog::onDuvidaClicked(int idDuvida)
{
    ForumPostsDialog *detalhes = new ForumPostsDialog(this, idDuvida, loggedInUsername);
    detalhes->exec();
    delete detalhes;

    if (disciplinaAtual > 0) carregarDuvidasDaDisciplina(disciplinaAtual);
}

void ForumDialog::on_searchLine_textChanged(const QString& texto) {
    if (texto.isEmpty()) {
        if (disciplinaAtual > 0) carregarDuvidasDaDisciplina(disciplinaAtual);
        else if (!categoriaAtual.isEmpty()) carregarDisciplinasDaCategoria(categoriaAtual);
        else carregarCategorias();
        return;
    }

    QLayoutItem *item;
    while ((item = layoutPrincipal->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    QSqlQuery query(dbConnection);
    query.prepare(
        "SELECT d.id_duvida, d.titulo, d.descricao, d.data_criacao, "
        "d.status, u.nome, u.Sobrenome, "
        "(SELECT COUNT(*) FROM Respostas_Duvidas WHERE id_duvida = d.id_duvida) as num_respostas "
        "FROM Duvidas d "
        "JOIN USUARIOS u ON d.id_usuario = u.id_usuario "
        "WHERE d.titulo LIKE ? OR d.descricao LIKE ? "
        "ORDER BY d.data_criacao DESC"
        );
    query.addBindValue("%" + texto + "%");
    query.addBindValue("%" + texto + "%");
    query.exec();

    while (query.next()) {
        int id = query.value("id_duvida").toInt();
        // Recalcula curtidas na busca também
        QSqlQuery likeQuery(dbConnection);
        likeQuery.prepare("SELECT COUNT(*) FROM Curtidas_Duvidas WHERE id_duvida = ?");
        likeQuery.addBindValue(id);
        int curtidas = 0;
        if(likeQuery.exec() && likeQuery.next()) curtidas = likeQuery.value(0).toInt();

        QFrame *card = criarCardDuvida(id, query.value("titulo").toString(),
                                       query.value("descricao").toString(),
                                       query.value("nome").toString() + " " + query.value("Sobrenome").toString(),
                                       query.value("data_criacao").toString(),
                                       query.value("num_respostas").toInt(),
                                       curtidas,
                                       query.value("status").toString() == "Respondida");
        layoutPrincipal->addWidget(card);
    }
    layoutPrincipal->addStretch();
}

void ForumDialog::voltarParaCategorias() {
    disciplinaAtual = 0;
    categoriaAtual.clear();
    carregarCategorias();
}

bool ForumDialog::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::MouseButtonPress) {
        QFrame *frame = qobject_cast<QFrame*>(obj);
        if (frame && frame->property("idDuvida").isValid()) {
            onDuvidaClicked(frame->property("idDuvida").toInt());
            return true;
        }
        if (obj == ui->homeButton) {
            MainWindow *mainWin = new MainWindow();
            mainWin->show();
            this->close();
            return true;
        }
        if (obj == ui->perfilButton) {
            PerfilDialog *pf = new PerfilDialog(this, loggedInUsername);
            pf->exec();
            delete pf;
            return true;
        }
    }
    return QDialog::eventFilter(obj, event);
}
