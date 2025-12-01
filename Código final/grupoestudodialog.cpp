#include "grupoestudodialog.h"
#include "ui_grupoestudodialog.h"
#include "chatmateria.h"
#include "perfildialog.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QRandomGenerator>
#include <QEvent>
#include <QMouseEvent>

// ============================================================================
// CONSTRUTOR
// ============================================================================
GrupoEstudoDialog::GrupoEstudoDialog(QWidget *parent, const QString& username)
    : QDialog(parent)
    , ui(new Ui::GrupoEstudoDialog)
    , loggedInUsername(username)
    , telaAtual("materias")
{
    ui->setupUi(this);
    setWindowTitle("📚 Grupos de Estudo - EducaUTFPR");
    resize(1200, 800);

    setupDatabase();
    criarTabelasNecessarias();

    // Carrega as matérias no ComboBox de criar sala
    QSqlQuery query(dbConnection);
    query.exec("SELECT id_materia, nome FROM Materias ORDER BY nome");
    while (query.next()) {
        int id = query.value(0).toInt();
        QString nome = query.value(1).toString();
        ui->materiaComboBox->addItem(nome, id);
    }

    // Conecta o botão de confirmar criação
    connect(ui->confirmarCriarButton, &QPushButton::clicked,
            this, &GrupoEstudoDialog::on_confirmarCriarButton_clicked);

    // Conecta o botão de entrar em grupo privado
    connect(ui->entrarGrupoButton, &QPushButton::clicked,
            this, &GrupoEstudoDialog::onEntrarGrupoPrivado);

    // Conecta os botões de navegação
    connect(ui->materiasButton, &QPushButton::clicked,
            this, &GrupoEstudoDialog::on_materiasButton_clicked);
    connect(ui->gruposButton, &QPushButton::clicked,
            this, &GrupoEstudoDialog::on_gruposButton_clicked);
    connect(ui->criarButton, &QPushButton::clicked,
            this, &GrupoEstudoDialog::on_criarButton_clicked);

    // Filtro de categoria
    connect(ui->categoriaComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &GrupoEstudoDialog::on_categoriaComboBox_currentIndexChanged);

    // Configura os eventos de clique na barra de navegação
    setupNavigationBar();

    // Mostra a tela inicial (Matérias)
    mostrarTela("materias");
}

// ============================================================================
// DESTRUTOR
// ============================================================================
GrupoEstudoDialog::~GrupoEstudoDialog()
{
    delete ui;
}

// ============================================================================
// SETUP DATABASE
// ============================================================================
void GrupoEstudoDialog::setupDatabase()
{
    dbConnection = QSqlDatabase::database("qt_sql_default_connection");

    if (!dbConnection.isOpen()) {
        qDebug() << "[GrupoEstudoDialog] ERRO: Banco de dados não está aberto.";
    }
}

// ============================================================================
// CRIAR TABELAS NECESSÁRIAS
// ============================================================================
void GrupoEstudoDialog::criarTabelasNecessarias()
{
    QSqlQuery query(dbConnection);

    // Tabela de Matérias
    query.exec(
        "CREATE TABLE IF NOT EXISTS Materias ("
        "id_materia INTEGER PRIMARY KEY AUTOINCREMENT, "
        "nome TEXT NOT NULL, "
        "categoria TEXT NOT NULL, "
        "icone TEXT, "
        "cor TEXT)"
        );

    // Tabela de Salas de Estudo
    query.exec(
        "CREATE TABLE IF NOT EXISTS Salas_Estudo ("
        "id_sala INTEGER PRIMARY KEY AUTOINCREMENT, "
        "id_materia INTEGER NOT NULL, "
        "nome_sala TEXT NOT NULL, "
        "codigo_sala TEXT UNIQUE NOT NULL, "
        "tipo TEXT NOT NULL, "  // 'publica' ou 'privada'
        "senha TEXT, "
        "max_participantes INTEGER DEFAULT 10, "
        "data_criacao DATETIME DEFAULT CURRENT_TIMESTAMP, "
        "FOREIGN KEY (id_materia) REFERENCES Materias(id_materia))"
        );

    // Tabela de Participantes
    query.exec(
        "CREATE TABLE IF NOT EXISTS Participantes_Sala ("
        "id_participante INTEGER PRIMARY KEY AUTOINCREMENT, "
        "id_sala INTEGER NOT NULL, "
        "id_usuario INTEGER NOT NULL, "
        "data_entrada DATETIME DEFAULT CURRENT_TIMESTAMP, "
        "FOREIGN KEY (id_sala) REFERENCES Salas_Estudo(id_sala), "
        "FOREIGN KEY (id_usuario) REFERENCES Usuario(id_usuario), "
        "UNIQUE(id_sala, id_usuario))"
        );

    // Tabela de Mensagens do Chat
    query.exec(
        "CREATE TABLE IF NOT EXISTS Mensagens_Chat ("
        "id_mensagem INTEGER PRIMARY KEY AUTOINCREMENT, "
        "id_sala INTEGER NOT NULL, "
        "id_usuario INTEGER NOT NULL, "
        "mensagem TEXT NOT NULL, "
        "data_envio DATETIME DEFAULT CURRENT_TIMESTAMP, "
        "FOREIGN KEY (id_sala) REFERENCES Salas_Estudo(id_sala), "
        "FOREIGN KEY (id_usuario) REFERENCES Usuario(id_usuario))"
        );

    // Popula matérias de exemplo (se não existirem)
    query.exec("SELECT COUNT(*) FROM Materias");
    if (query.next() && query.value(0).toInt() == 0) {
        popularMateriasCompletas();
    }
}

// ============================================================================
// POPULAR MATÉRIAS COMPLETAS
// ============================================================================
void GrupoEstudoDialog::popularMateriasCompletas()
{
    QSqlQuery query(dbConnection);

    QList<QStringList> materias = {
        // MATEMÁTICA
        {"Cálculo Diferencial e Integral 1", "Matemática", "📐", "#FF6B6B"},
        {"Cálculo Diferencial e Integral 2", "Matemática", "📐", "#FF8787"},
        {"Cálculo Diferencial e Integral 3", "Matemática", "📐", "#FFA07A"},
        {"Álgebra Linear", "Matemática", "📊", "#FFB6B9"},
        {"Geometria Analítica", "Matemática", "📏", "#FFCCCC"},
        {"Matemática Discreta", "Matemática", "🔢", "#FF9999"},
        {"Equações Diferenciais Ordinárias", "Matemática", "📈", "#FFB347"},
        {"Cálculo Numérico", "Matemática", "🔢", "#FFAB73"},
        {"Probabilidade e Estatística", "Matemática", "📊", "#FFC9A0"},
        {"Análise de Sistemas Lineares", "Matemática", "📈", "#FFD9B3"},

        // FÍSICA
        {"Física Teórica 1", "Física", "⚛️", "#4ECDC4"},
        {"Física Teórica 2", "Física", "⚛️", "#45B7D1"},
        {"Física Teórica 3", "Física", "⚛️", "#5AD5E5"},
        {"Física Experimental 1", "Física", "🧪", "#70E0F0"},
        {"Física Experimental 2", "Física", "🧪", "#85EBFA"},

        // PROGRAMAÇÃO
        {"Fundamentos de Programação 1", "Programação", "💻", "#F38181"},
        {"Fundamentos de Programação 2", "Programação", "💻", "#FF8FA3"},
        {"Programação Orientada a Objetos", "Programação", "🎲", "#FFADAD"},
        {"Estrutura de Dados 1", "Programação", "📦", "#FFD6A5"},
        {"Estrutura de Dados 2", "Programação", "📦", "#FDFFB6"},
        {"Compiladores", "Programação", "📝", "#CAFFBF"},
        {"Desenvolvimento de Aplicações Web", "Programação", "🌐", "#9BF6FF"},

        // BANCO DE DADOS
        {"Banco de Dados", "Banco de Dados", "🗄️", "#A0C4FF"},

        // REDES E SISTEMAS
        {"Sistemas Operacionais", "Redes e Sistemas", "💾", "#BDB2FF"},
        {"Redes de Computadores", "Redes e Sistemas", "🌐", "#FFC6FF"},
        {"Comunicação de Dados", "Redes e Sistemas", "📡", "#E7C6FF"},
        {"Sistemas Distribuídos", "Redes e Sistemas", "☁️", "#C8B6FF"},
        {"Segurança e Auditoria de Sistemas", "Redes e Sistemas", "🔐", "#D4A5FF"},

        // ENGENHARIA
        {"Introdução à Engenharia de Computação", "Engenharia", "💡", "#FFE66D"},
        {"Arquitetura e Organização de Computadores", "Engenharia", "🖥️", "#FFEB99"},
        {"Circuitos Digitais", "Engenharia", "📌", "#FFF4B8"},
        {"Sistemas Digitais", "Engenharia", "🔧", "#FFFBD4"},
        {"Eletrônica A", "Engenharia", "📌", "#FDE4CF"},
        {"Eletrônica B", "Engenharia", "📌", "#FFCFD2"},
        {"Análise de Circuitos Elétricos 1", "Engenharia", "⚡", "#F1C0E8"},
        {"Materiais e Equipamentos Elétricos", "Engenharia", "⚡", "#CFBAF0"},
        {"Desenho Técnico", "Engenharia", "✏️", "#A3C4F3"},
        {"Fundamentos de Controle", "Engenharia", "🎛️", "#90DBF4"},
        {"Controle Digital", "Engenharia", "🎮", "#8EECF5"},
        {"Lógica Reconfigurável", "Engenharia", "🔧", "#98F5E1"},
        {"Sistemas Microcontrolados", "Engenharia", "⚙️", "#B9FBC0"},
        {"Sistemas Embarcados", "Engenharia", "🔧", "#C7CEEA"},
        {"Instrumentação Eletrônica", "Engenharia", "📡", "#FFDAB9"},
        {"Processamento Digital de Sinais", "Engenharia", "📊", "#FFDFBA"},
        {"Oficina de Integração 1", "Engenharia", "🛠️", "#FFFFBA"},
        {"Oficina de Integração 2", "Engenharia", "🛠️", "#BAFFC9"},

        // QUÍMICA
        {"Química Geral", "Química", "⚗️", "#95E1D3"},
        {"Química Experimental", "Química", "🧪", "#A8E6CF"},

        // COMPUTAÇÃO TEÓRICA
        {"Teoria da Computação", "Programação", "🧮", "#DCEDC1"},
        {"Introdução à Lógica para Computação", "Programação", "🧠", "#FFD3B6"},
        {"Sistemas Inteligentes 1", "Programação", "🤖", "#FFAAA5"},

        // ENGENHARIA DE SOFTWARE
        {"Engenharia de Software", "Programação", "🗂️", "#FF8B94"},

        // HUMANAS E SOCIAIS
        {"Comunicação Linguística", "Humanas e Sociais", "📝", "#A8DADC"},
        {"Inglês Instrumental", "Humanas e Sociais", "🌍", "#457B9D"},
        {"Metodologia de Pesquisa", "Humanas e Sociais", "📚", "#1D3557"},
        {"Ciências do Ambiente", "Humanas e Sociais", "🌱", "#2A9D8F"},
        {"Economia", "Humanas e Sociais", "💰", "#E76F51"},
        {"Empreendedorismo", "Humanas e Sociais", "💡", "#F4A261"},
        {"Relações Humanas e Liderança", "Humanas e Sociais", "🤝", "#E9C46A"},
        {"Meio Ambiente e Sociedade", "Humanas e Sociais", "🌍", "#2A9134"},
        {"Qualidade de Vida", "Humanas e Sociais", "💚", "#52B788"},
        {"Aptidão Física", "Humanas e Sociais", "🏃", "#74C69D"},
        {"Libras 1", "Humanas e Sociais", "👋", "#95D5B2"},

        // TCC E ESTÁGIO
        {"Trabalho de Conclusão de Curso 1", "Engenharia", "📄", "#B7E4C7"},
        {"Trabalho de Conclusão de Curso 2", "Engenharia", "📄", "#D8F3DC"},
        {"Estágio Curricular Obrigatório", "Engenharia", "💼", "#E8F5E9"},

        // ATIVIDADES COMPLEMENTARES
        {"Atividades Complementares", "Humanas e Sociais", "🎯", "#C7CEEA"}
    };

    for (const auto& mat : materias) {
        query.prepare("INSERT INTO Materias (nome, categoria, icone, cor) VALUES (?, ?, ?, ?)");
        query.addBindValue(mat[0]);
        query.addBindValue(mat[1]);
        query.addBindValue(mat[2]);
        query.addBindValue(mat[3]);
        query.exec();
    }

    qDebug() << "✅ Matérias populadas com sucesso!";
}

// ============================================================================
// CRIAR SALA (AÇÃO DO BOTÃO CONFIRMAR)
// ============================================================================
void GrupoEstudoDialog::on_confirmarCriarButton_clicked()
{
    if (!validarCriacaoSala()) {
        return;
    }

    QString nomeSala = ui->nomeSalaEdit->text().trimmed();
    int idMateria = ui->materiaComboBox->currentData().toInt();
    QString tipo = ui->tipoPublicaRadio->isChecked() ? "publica" : "privada";
    QString senha = ui->senhaEdit->text();
    int maxParticipantes = ui->maxParticipantesSpinBox->value();
    QString codigo = gerarCodigoSala();

    QSqlQuery query(dbConnection);
    query.prepare(
        "INSERT INTO Salas_Estudo (id_materia, nome_sala, codigo_sala, tipo, senha, max_participantes) "
        "VALUES (?, ?, ?, ?, ?, ?)"
        );

    // Binds das variáveis coletadas acima
    query.addBindValue(idMateria);
    query.addBindValue(nomeSala);
    query.addBindValue(codigo);
    query.addBindValue(tipo);
    query.addBindValue(tipo == "privada" ? senha : QVariant());
    query.addBindValue(maxParticipantes);

    if (query.exec()) {
        int idSala = query.lastInsertId().toInt();

        // Adiciona criador como participante
        int idUsuario = getIdUsuario(loggedInUsername);

        QSqlQuery pQuery(dbConnection);
        pQuery.prepare("INSERT INTO Participantes_Sala (id_sala, id_usuario) VALUES (?, ?)");
        pQuery.addBindValue(idSala);
        pQuery.addBindValue(idUsuario);
        pQuery.exec();

        QMessageBox::information(this, "✅ Grupo Criado!",
                                 QString("Código do grupo: %1\n%2")
                                     .arg(codigo)
                                     .arg(tipo == "privada" ? "Compartilhe com cuidado!" : ""));

        // Limpa os campos
        ui->nomeSalaEdit->clear();
        ui->senhaEdit->clear();

        // Volta para a tela de grupos
        mostrarTela("grupos");
    } else {
        QMessageBox::critical(this, "❌ Erro",
                              "Erro ao criar grupo: " + query.lastError().text());
    }
}

// ============================================================================
// AUXILIARES E NAVEGAÇÃO
// ============================================================================
int GrupoEstudoDialog::getIdUsuario(const QString& username)
{
    QSqlQuery query(dbConnection);
    query.prepare("SELECT id_usuario FROM Usuario WHERE usuario = ?");
    query.addBindValue(username);

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    return -1;
}

void GrupoEstudoDialog::mostrarTela(const QString& tela)
{
    telaAtual = tela;

    if (tela == "materias") {
        ui->stackedWidget->setCurrentIndex(0);
        carregarMateriasDaCategoria(ui->categoriaComboBox->currentText());
    } else if (tela == "grupos") {
        ui->stackedWidget->setCurrentIndex(1);
        carregarSalasPublicas();
    } else if (tela == "criar") {
        ui->stackedWidget->setCurrentIndex(2);
    }
}

void GrupoEstudoDialog::carregarMateriasDaCategoria(const QString& categoria)
{
    QWidget *container = ui->materiasContainer;
    QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(container->layout());

    if (!layout) {
        layout = new QVBoxLayout(container);
        layout->setSpacing(12);
        layout->setContentsMargins(15, 15, 15, 15);
        container->setLayout(layout);
    }

    // Limpa o layout
    QLayoutItem *item;
    while ((item = layout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    // Busca matérias
    QSqlQuery query(dbConnection);
    QString queryStr = "SELECT id_materia, nome, icone, cor FROM Materias";

    if (!categoria.contains("Todas")) {
        // Remove o emoji do início se houver (lógica simples)
        QString categoriaLimpa = categoria;
        int primeiroEspaco = categoriaLimpa.indexOf(' ');
        if (primeiroEspaco != -1) {
            categoriaLimpa = categoriaLimpa.mid(primeiroEspaco + 1).trimmed();
        }

        queryStr += " WHERE categoria = ?";
        query.prepare(queryStr);
        query.addBindValue(categoriaLimpa);
    } else {
        query.prepare(queryStr);
    }

    query.exec();

    while (query.next()) {
        int id = query.value(0).toInt();
        QString nome = query.value(1).toString();
        QString icone = query.value(2).toString();
        QString cor = query.value(3).toString();

        QPushButton *btn = criarBotaoMateria(id, nome, icone, cor);
        layout->addWidget(btn);
    }

    layout->addStretch();
}

QPushButton* GrupoEstudoDialog::criarBotaoMateria(int idMateria, const QString& nome,
                                                  const QString& icone, const QString& cor)
{
    QPushButton *btn = new QPushButton();
    btn->setText(QString("%1  %2").arg(icone).arg(nome));
    btn->setMinimumHeight(70);
    btn->setCursor(Qt::PointingHandCursor);

    QString corBase = cor;
    QString corHover = corBase;

    // Efeito simples de hover escurecendo levemente
    if (corHover.startsWith("#FF")) {
        corHover.replace(0, 3, "#DD");
    } else if (corHover.contains("FF")) {
        corHover.replace("FF", "DD");
    }

    btn->setStyleSheet(
        QString("QPushButton {"
                "   background-color: %1;"
                "   color: white;"
                "   border: none;"
                "   border-radius: 12px;"
                "   padding: 15px 25px;"
                "   font-size: 18px;"
                "   font-weight: bold;"
                "   text-align: left;"
                "}"
                "QPushButton:hover {"
                "   background-color: %2;"
                "   transform: scale(1.02);"
                "}").arg(corBase).arg(corHover)
        );

    connect(btn, &QPushButton::clicked, [this, idMateria]() {
        onMateriaClicked(idMateria);
    });

    return btn;
}

void GrupoEstudoDialog::onMateriaClicked(int idMateria)
{
    // Busca o nome da matéria
    QSqlQuery query(dbConnection);
    query.prepare("SELECT nome FROM Materias WHERE id_materia = ?");
    query.addBindValue(idMateria);

    if (!query.exec() || !query.next()) {
        return;
    }

    QString nomeMateria = query.value(0).toString();

    // Busca a sala geral da matéria (ou cria se não existir)
    query.prepare("SELECT id_sala, codigo_sala, nome_sala FROM Salas_Estudo WHERE id_materia = ? AND tipo = 'publica'");
    query.addBindValue(idMateria);

    int idSala = 0;
    QString nomeSala = "";

    if (query.exec() && query.next()) {
        idSala = query.value(0).toInt();
        nomeSala = query.value(2).toString();
    } else {
        // Cria sala geral automaticamente
        QString codigo = gerarCodigoSala();
        query.prepare(
            "INSERT INTO Salas_Estudo (id_materia, nome_sala, codigo_sala, tipo, max_participantes) "
            "VALUES (?, ?, ?, 'publica', 50)"
            );
        query.addBindValue(idMateria);
        query.addBindValue("Chat Geral - " + nomeMateria);
        query.addBindValue(codigo);
        query.exec();

        idSala = query.lastInsertId().toInt();
        nomeSala = "Chat Geral - " + nomeMateria;
    }

    // Adiciona usuário como participante
    int idUsuario = getIdUsuario(loggedInUsername);
    query.prepare("INSERT OR IGNORE INTO Participantes_Sala (id_sala, id_usuario) VALUES (?, ?)");
    query.addBindValue(idSala);
    query.addBindValue(idUsuario);
    query.exec();

    // Abre o chat
    ChatMateria *chat = new ChatMateria(this, loggedInUsername, idSala, nomeSala);
    chat->exec();
    delete chat;
}

void GrupoEstudoDialog::carregarSalasPublicas()
{
    QWidget *container = ui->salasContainer;
    QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(container->layout());

    if (!layout) {
        layout = new QVBoxLayout(container);
        layout->setSpacing(12);
        layout->setContentsMargins(15, 15, 15, 15);
        container->setLayout(layout);
    }

    // Limpa
    QLayoutItem *item;
    while ((item = layout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    // Busca salas públicas
    QSqlQuery query(dbConnection);
    query.exec(
        "SELECT s.id_sala, s.codigo_sala, s.nome_sala, s.tipo, s.max_participantes, "
        "COUNT(p.id_usuario) as num_participantes "
        "FROM Salas_Estudo s "
        "LEFT JOIN Participantes_Sala p ON s.id_sala = p.id_sala "
        "WHERE s.tipo = 'publica' "
        "GROUP BY s.id_sala "
        "ORDER BY num_participantes DESC"
        );

    while (query.next()) {
        int idSala = query.value(0).toInt();
        QString codigoSala = query.value(1).toString();
        QString nomeSala = query.value(2).toString();
        QString tipo = query.value(3).toString();
        int maxParticipantes = query.value(4).toInt();
        int numParticipantes = query.value(5).toInt();

        QFrame *card = criarCardSala(idSala, codigoSala, nomeSala, tipo,
                                     numParticipantes, maxParticipantes);
        layout->addWidget(card);
    }

    layout->addStretch();
}

QFrame* GrupoEstudoDialog::criarCardSala(int idSala, const QString& codigoSala,
                                         const QString& nomeSala, const QString& tipo,
                                         int numParticipantes, int maxParticipantes)
{
    QFrame *card = new QFrame();
    card->setFrameShape(QFrame::StyledPanel);
    card->setMinimumHeight(100);
    card->setStyleSheet(
        "QFrame {"
        "   background-color: #423738;"
        "   border-left: 5px solid #F4B315;"
        "   border-radius: 10px;"
        "   padding: 15px;"
        "   margin: 5px;"
        "}"
        "QFrame:hover {"
        "   background-color: #524447;"
        "}"
        );

    QHBoxLayout *mainLayout = new QHBoxLayout(card);

    // Informações
    QVBoxLayout *infoLayout = new QVBoxLayout();

    QLabel *nomeLabel = new QLabel(nomeSala);
    nomeLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #F4B315;");

    QLabel *infoLabel = new QLabel(
        QString("👥 %1/%2 participantes | 🔑 %3")
            .arg(numParticipantes)
            .arg(maxParticipantes)
            .arg(codigoSala)
        );
    infoLabel->setStyleSheet("font-size: 12px; color: #D3AF35;");

    infoLayout->addWidget(nomeLabel);
    infoLayout->addWidget(infoLabel);

    // Botão Entrar
    QPushButton *entrarBtn = new QPushButton("🔥 Entrar");
    entrarBtn->setMinimumWidth(100);
    entrarBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #F4B315;"
        "   color: #1A161A;"
        "   border: none;"
        "   border-radius: 8px;"
        "   padding: 10px 20px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: #D3AF35; }"
        );

    connect(entrarBtn, &QPushButton::clicked, [this, idSala]() {
        onSalaClicked(idSala);
    });

    mainLayout->addLayout(infoLayout, 1);
    mainLayout->addWidget(entrarBtn);

    return card;
}

void GrupoEstudoDialog::onSalaClicked(int idSala)
{
    // Busca informações da sala
    QSqlQuery query(dbConnection);
    query.prepare("SELECT nome_sala FROM Salas_Estudo WHERE id_sala = ?");
    query.addBindValue(idSala);

    if (!query.exec() || !query.next()) {
        return;
    }

    QString nomeSala = query.value(0).toString();

    // Adiciona usuário como participante
    int idUsuario = getIdUsuario(loggedInUsername);
    query.prepare("INSERT OR IGNORE INTO Participantes_Sala (id_sala, id_usuario) VALUES (?, ?)");
    query.addBindValue(idSala);
    query.addBindValue(idUsuario);
    query.exec();

    // Abre o chat
    ChatMateria *chat = new ChatMateria(this, loggedInUsername, idSala, nomeSala);
    chat->exec();
    delete chat;
}

// ============================================================================
// NAVEGAÇÃO E EVENTOS
// ============================================================================
void GrupoEstudoDialog::on_materiasButton_clicked()
{
    mostrarTela("materias");
}

void GrupoEstudoDialog::on_gruposButton_clicked()
{
    mostrarTela("grupos");
}

void GrupoEstudoDialog::on_criarButton_clicked()
{
    mostrarTela("criar");
}

void GrupoEstudoDialog::on_categoriaComboBox_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    carregarMateriasDaCategoria(ui->categoriaComboBox->currentText());
}

void GrupoEstudoDialog::setupNavigationBar()
{
    ui->homeButton->installEventFilter(this);
    ui->perfilButton->installEventFilter(this);
}

bool GrupoEstudoDialog::eventFilter(QObject *obj, QEvent *event)
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

void GrupoEstudoDialog::onEntrarGrupoPrivado()
{
    QString codigo = ui->codigoGrupoEdit->text().trimmed();
    QString senha = ui->senhaGrupoEdit->text();

    if (codigo.isEmpty()) {
        QMessageBox::warning(this, "⚠️ Campo Vazio",
                             "Por favor, digite o código do grupo!");
        return;
    }

    QSqlQuery query(dbConnection);
    query.prepare(
        "SELECT id_sala, tipo, senha, nome_sala FROM Salas_Estudo "
        "WHERE codigo_sala = ?"
        );
    query.addBindValue(codigo);

    if (!query.exec() || !query.next()) {
        QMessageBox::warning(this, "⚠️ Grupo Não Encontrado",
                             "Não existe nenhum grupo com este código!");
        return;
    }

    int idSala = query.value(0).toInt();
    QString tipo = query.value(1).toString();
    QString senhaCorreta = query.value(2).toString();
    QString nomeSala = query.value(3).toString();

    if (tipo == "privada" && senha != senhaCorreta) {
        QMessageBox::warning(this, "⚠️ Senha Incorreta",
                             "A senha digitada está incorreta!");
        return;
    }

    int idUsuario = getIdUsuario(loggedInUsername);
    query.prepare("INSERT OR IGNORE INTO Participantes_Sala (id_sala, id_usuario) VALUES (?, ?)");
    query.addBindValue(idSala);
    query.addBindValue(idUsuario);
    query.exec();

    ui->codigoGrupoEdit->clear();
    ui->senhaGrupoEdit->clear();

    ChatMateria *chat = new ChatMateria(this, loggedInUsername, idSala, nomeSala);
    chat->exec();
    delete chat;
}

QString GrupoEstudoDialog::gerarCodigoSala()
{
    QString codigo;
    for (int i = 0; i < 5; i++) {
        codigo += QString::number(QRandomGenerator::global()->bounded(10));
    }
    return codigo;
}

bool GrupoEstudoDialog::validarCriacaoSala()
{
    if (ui->nomeSalaEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "⚠️ Campo Vazio", "Digite o nome do grupo!");
        return false;
    }

    if (ui->tipoPrivadaRadio->isChecked() && ui->senhaEdit->text().isEmpty()) {
        QMessageBox::warning(this, "⚠️ Senha Obrigatória",
                             "Grupos privados precisam de senha!");
        return false;
    }

    return true;
}
