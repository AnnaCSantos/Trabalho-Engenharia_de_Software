#include "grupoestudodialog.h"
#include "ui_grupoestudodialog.h"
#include "chatmateria.h"
#include "perfildialog.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
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
#include <QColor>

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

    ui->categoriaComboBox->clear();
    ui->categoriaComboBox->addItem("📚 Todas as Matérias");
    ui->categoriaComboBox->addItem("📐 Matemática");
    ui->categoriaComboBox->addItem("⚛️ Física");
    ui->categoriaComboBox->addItem("💻 Programação");
    ui->categoriaComboBox->addItem("🗄️ Banco de Dados");
    ui->categoriaComboBox->addItem("🌐 Redes e Sistemas");
    ui->categoriaComboBox->addItem("🔧 Engenharia");
    ui->categoriaComboBox->addItem("⚗️ Química");
    ui->categoriaComboBox->addItem("📚 Humanas e Sociais");
    ui->categoriaComboBox->addItem("📄 TCC e Estágio");
    ui->categoriaComboBox->addItem("🎯 Atividades Extras");

    connect(ui->entrarGrupoButton, &QPushButton::clicked, this, &GrupoEstudoDialog::onEntrarGrupoPrivado);

    setupNavigationBar();
    mostrarTela("materias");
}

GrupoEstudoDialog::~GrupoEstudoDialog()
{
    delete ui;
}

void GrupoEstudoDialog::setupDatabase()
{
    dbConnection = QSqlDatabase::database("qt_sql_default_connection");
    if (!dbConnection.isOpen()) {
        qDebug() << "Erro: Banco de dados fechado.";
    }
}

void GrupoEstudoDialog::criarTabelasNecessarias()
{
    QSqlQuery query(dbConnection);

    query.exec(
        "CREATE TABLE IF NOT EXISTS Materias ("
        "id_materia INTEGER PRIMARY KEY AUTOINCREMENT, "
        "nome TEXT NOT NULL, "
        "categoria TEXT NOT NULL, "
        "icone TEXT, "
        "cor TEXT)"
        );

    query.exec(
        "CREATE TABLE IF NOT EXISTS Salas_Estudo ("
        "id_sala INTEGER PRIMARY KEY AUTOINCREMENT, "
        "id_materia INTEGER NOT NULL, "
        "nome_sala TEXT NOT NULL, "
        "codigo_sala TEXT UNIQUE NOT NULL, "
        "tipo TEXT NOT NULL, "
        "senha TEXT, "
        "max_participantes INTEGER DEFAULT 10, "
        "data_criacao DATETIME DEFAULT CURRENT_TIMESTAMP, "
        "FOREIGN KEY (id_materia) REFERENCES Materias(id_materia))"
        );

    query.exec(
        "CREATE TABLE IF NOT EXISTS Participantes_Sala ("
        "id_participante INTEGER PRIMARY KEY AUTOINCREMENT, "
        "id_sala INTEGER NOT NULL, "
        "id_usuario INTEGER NOT NULL, "
        "data_entrada DATETIME DEFAULT CURRENT_TIMESTAMP, "
        "FOREIGN KEY (id_sala) REFERENCES Salas_Estudo(id_sala), "
        "FOREIGN KEY (id_usuario) REFERENCES USUARIOS(id_usuario), "
        "UNIQUE(id_sala, id_usuario))"
        );

    query.exec(
        "CREATE TABLE IF NOT EXISTS Mensagens_Chat ("
        "id_mensagem INTEGER PRIMARY KEY AUTOINCREMENT, "
        "id_sala INTEGER NOT NULL, "
        "id_usuario INTEGER NOT NULL, "
        "mensagem TEXT NOT NULL, "
        "data_envio DATETIME DEFAULT CURRENT_TIMESTAMP, "
        "FOREIGN KEY (id_sala) REFERENCES Salas_Estudo(id_sala), "
        "FOREIGN KEY (id_usuario) REFERENCES USUARIOS(id_usuario))"
        );

    query.exec("SELECT COUNT(*) FROM Materias");
    if (query.next() && query.value(0).toInt() == 0) {
        popularMateriasCompletas();
    }
}

void GrupoEstudoDialog::popularMateriasCompletas()
{
    QSqlQuery query(dbConnection);
    QList<QStringList> materias = {
        {"Cálculo Diferencial e Integral 1", "Matemática", "📐", "#F4B315"},
        {"Cálculo Diferencial e Integral 2", "Matemática", "📐", "#E5A314"},
        {"Cálculo Diferencial e Integral 3", "Matemática", "📐", "#D69313"},
        {"Álgebra Linear", "Matemática", "📊", "#C78312"},
        {"Geometria Analítica", "Matemática", "📏", "#B87311"},
        {"Matemática Discreta", "Matemática", "🔢", "#A96310"},
        {"Equações Diferenciais Ordinárias", "Matemática", "📈", "#9A530F"},
        {"Cálculo Numérico", "Matemática", "🔢", "#8B430E"},
        {"Probabilidade e Estatística", "Matemática", "📊", "#7C330D"},
        {"Análise de Sistemas Lineares", "Matemática", "📈", "#8E6915"},
        {"Física Teórica 1", "Física", "⚛️", "#4A90E2"},
        {"Física Teórica 2", "Física", "⚛️", "#3A80D2"},
        {"Física Teórica 3", "Física", "⚛️", "#2A70C2"},
        {"Física Experimental 1", "Física", "🧪", "#5BA0F2"},
        {"Física Experimental 2", "Física", "🧪", "#6BB0FF"},
        {"Fundamentos de Programação 1", "Programação", "💻", "#E74C3C"},
        {"Fundamentos de Programação 2", "Programação", "💻", "#C0392B"},
        {"Programação Orientada a Objetos", "Programação", "🎲", "#A93226"},
        {"Estrutura de Dados 1", "Programação", "📦", "#922B21"},
        {"Estrutura de Dados 2", "Programação", "📦", "#7B241C"},
        {"Compiladores", "Programação", "📝", "#641E16"},
        {"Teoria da Computação", "Programação", "🧮", "#943126"},
        {"Introdução à Lógica para Computação", "Programação", "🧠", "#A43B2B"},
        {"Banco de Dados", "Banco de Dados", "🗄️", "#9B59B6"},
        {"Sistemas Operacionais", "Redes e Sistemas", "💾", "#16A085"},
        {"Redes de Computadores", "Redes e Sistemas", "🌐", "#138D75"},
        {"Comunicação de Dados", "Redes e Sistemas", "📡", "#117A65"},
        {"Sistemas Distribuídos", "Redes e Sistemas", "☁️", "#0E6655"},
        {"Segurança e Auditoria de Sistemas", "Redes e Sistemas", "🔒", "#0B5345"},
        {"Introdução à Engenharia de Computação", "Engenharia", "💡", "#F39C12"},
        {"Arquitetura e Organização de Computadores", "Engenharia", "🖥️", "#E67E22"},
        {"Circuitos Digitais", "Engenharia", "🔌", "#D68910"},
        {"Sistemas Digitais", "Engenharia", "🔧", "#CA6F1E"},
        {"Eletrônica A", "Engenharia", "🔌", "#BA4A00"},
        {"Eletrônica B", "Engenharia", "🔌", "#A04000"},
        {"Análise de Circuitos Elétricos 1", "Engenharia", "⚡", "#873600"},
        {"Materiais e Equipamentos Elétricos", "Engenharia", "⚡", "#6E2C00"},
        {"Desenho Técnico", "Engenharia", "✏️", "#DC7633"},
        {"Fundamentos de Controle", "Engenharia", "🎛️", "#28B463"},
        {"Controle Digital", "Engenharia", "🎮", "#239B56"},
        {"Lógica Reconfigurável", "Engenharia", "🔧", "#1E8449"},
        {"Sistemas Microcontrolados", "Engenharia", "⚙️", "#196F3D"},
        {"Sistemas Embarcados", "Engenharia", "🔧", "#145A32"},
        {"Instrumentação Eletrônica", "Engenharia", "📡", "#0E4B26"},
        {"Processamento Digital de Sinais", "Engenharia", "📊", "#7DCEA0"},
        {"Oficina de Integração 1", "Engenharia", "🛠️", "#5DADE2"},
        {"Oficina de Integração 2", "Engenharia", "🛠️", "#3498DB"},
        {"Química Geral", "Química", "⚗️", "#1ABC9C"},
        {"Química Experimental", "Química", "🧪", "#17A589"},
        {"Sistemas Inteligentes 1", "Programação", "🤖", "#E74C3C"},
        {"Engenharia de Software", "Programação", "🗂️", "#95A5A6"},
        {"Desenvolvimento de Aplicações Web", "Programação", "🌐", "#7F8C8D"},
        {"Comunicação Linguística", "Humanas e Sociais", "📝", "#34495E"},
        {"Inglês Instrumental", "Humanas e Sociais", "🌍", "#2C3E50"},
        {"Metodologia de Pesquisa", "Humanas e Sociais", "📚", "#566573"},
        {"Ciências do Ambiente", "Humanas e Sociais", "🌱", "#52BE80"},
        {"Economia", "Humanas e Sociais", "💰", "#F4D03F"},
        {"Empreendedorismo", "Humanas e Sociais", "💡", "#F7DC6F"},
        {"Relações Humanas e Liderança", "Humanas e Sociais", "🤝", "#F8C471"},
        {"Meio Ambiente e Sociedade", "Humanas e Sociais", "🌍", "#58D68D"},
        {"Qualidade de Vida", "Humanas e Sociais", "💚", "#82E0AA"},
        {"Aptidão Física", "Humanas e Sociais", "🏃", "#ABEBC6"},
        {"Libras 1", "Humanas e Sociais", "👋", "#D5F4E6"},
        {"Trabalho de Conclusão de Curso 1", "TCC e Estágio", "📄", "#85C1E2"},
        {"Trabalho de Conclusão de Curso 2", "TCC e Estágio", "📄", "#5DADE2"},
        {"Estágio Curricular Obrigatório", "TCC e Estágio", "💼", "#3498DB"},
        {"Atividades Complementares", "Atividades Extras", "🎯", "#AED6F1"}
    };

    dbConnection.transaction();
    QSqlQuery queryIns(dbConnection);
    queryIns.prepare("INSERT INTO Materias (nome, categoria, icone, cor) VALUES (?, ?, ?, ?)");

    for (const auto& mat : materias) {
        queryIns.addBindValue(mat[0]);
        queryIns.addBindValue(mat[1]);
        queryIns.addBindValue(mat[2]);
        queryIns.addBindValue(mat[3]);
        queryIns.exec();
    }
    dbConnection.commit();
}

bool GrupoEstudoDialog::validarCriacaoSala()
{
    QString nomeSala = ui->nomeSalaEdit->text().trimmed();

    if (nomeSala.isEmpty()) {
        QMessageBox::warning(this, "⚠️ Campo Vazio", "Digite o nome do grupo!");
        return false;
    }

    int idMateria = ui->materiaComboBox->currentData().toInt();
    if (idMateria <= 0) {
        QMessageBox::warning(this, "⚠️ Erro", "Selecione uma matéria válida!");
        return false;
    }

    if (ui->tipoPrivadaRadio->isChecked() && ui->senhaEdit->text().isEmpty()) {
        QMessageBox::warning(this, "⚠️ Senha Obrigatória", "Grupos privados precisam de senha!");
        return false;
    }

    return true;
}

void GrupoEstudoDialog::on_confirmarCriarButton_clicked()
{
    if (!validarCriacaoSala()) return;

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

    query.addBindValue(idMateria);
    query.addBindValue(nomeSala);
    query.addBindValue(codigo);
    query.addBindValue(tipo);
    query.addBindValue(tipo == "privada" ? senha : QVariant());
    query.addBindValue(maxParticipantes);

    if (query.exec()) {
        int idSala = query.lastInsertId().toInt();
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

        ui->nomeSalaEdit->clear();
        ui->senhaEdit->clear();
        mostrarTela("grupos");
    } else {
        QMessageBox::critical(this, "❌ Erro", "Erro ao criar grupo: " + query.lastError().text());
    }
}

int GrupoEstudoDialog::getIdUsuario(const QString& username)
{
    QSqlQuery query(dbConnection);
    query.prepare("SELECT id_usuario FROM USUARIOS WHERE usuario = ?");
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
        popularComboMateriasCriacao();
        ui->nomeSalaEdit->clear();
        ui->senhaEdit->clear();
        ui->tipoPublicaRadio->setChecked(true);
        ui->maxParticipantesSpinBox->setValue(10);
    }
}

void GrupoEstudoDialog::popularComboMateriasCriacao()
{
    ui->materiaComboBox->clear();
    QSqlQuery query(dbConnection);
    query.exec("SELECT id_materia, nome FROM Materias ORDER BY nome ASC");
    while (query.next()) {
        ui->materiaComboBox->addItem(query.value(1).toString(), query.value(0).toInt());
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

    QLayoutItem *item;
    while ((item = layout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    QSqlQuery query(dbConnection);
    QString queryStr = "SELECT id_materia, nome, icone, cor FROM Materias";

    if (!categoria.contains("Todas")) {
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
        QPushButton *btn = criarBotaoMateria(
            query.value(0).toInt(),
            query.value(1).toString(),
            query.value(2).toString(),
            query.value(3).toString()
            );
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

    QColor corObj(cor);
    QString corBase = corObj.name();
    QString corHover = corObj.darker(120).name();

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
    QSqlQuery query(dbConnection);
    query.prepare("SELECT nome FROM Materias WHERE id_materia = ?");
    query.addBindValue(idMateria);

    if (!query.exec() || !query.next()) return;

    QString nomeMateria = query.value(0).toString();

    query.prepare("SELECT id_sala, codigo_sala, nome_sala FROM Salas_Estudo WHERE id_materia = ? AND tipo = 'publica'");
    query.addBindValue(idMateria);

    int idSala = 0;
    QString nomeSala = "";

    if (query.exec() && query.next()) {
        idSala = query.value(0).toInt();
        nomeSala = query.value(2).toString();
    } else {
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

    int idUsuario = getIdUsuario(loggedInUsername);
    query.prepare("INSERT OR IGNORE INTO Participantes_Sala (id_sala, id_usuario) VALUES (?, ?)");
    query.addBindValue(idSala);
    query.addBindValue(idUsuario);
    query.exec();

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

    QLayoutItem *item;
    while ((item = layout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

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
        QFrame *card = criarCardSala(
            query.value(0).toInt(),
            query.value(1).toString(),
            query.value(2).toString(),
            query.value(3).toString(),
            query.value(5).toInt(),
            query.value(4).toInt()
            );
        layout->addWidget(card);
    }

    layout->addStretch();
}

QFrame* GrupoEstudoDialog::criarCardSala(int idSala, const QString& codigoSala,
                                         const QString& nomeSala, const QString& tipo,
                                         int numParticipantes, int maxParticipantes)
{
    QFrame *card = new QFrame();
    card->setObjectName(QString::number(idSala));
    card->setFrameShape(QFrame::StyledPanel);
    card->setMinimumHeight(110);
    card->setStyleSheet(
        "QFrame {"
        "   background-color: #423738;"
        "   border-left: 6px solid #F4B315;"
        "   border-radius: 10px;"
        "   padding: 10px;"
        "   margin: 5px 0px;"
        "}"
        "QFrame:hover { background-color: #524447; }"
        );

    QHBoxLayout *mainLayout = new QHBoxLayout(card);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(15, 10, 15, 10);

    QWidget *textContainer = new QWidget();
    textContainer->setStyleSheet("background: transparent; border: none;");
    textContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QVBoxLayout *infoLayout = new QVBoxLayout(textContainer);
    infoLayout->setContentsMargins(0, 0, 0, 0);
    infoLayout->setSpacing(5);

    QLabel *nomeLabel = new QLabel(nomeSala.isEmpty() ? "⚠️ Sem Nome" : nomeSala);
    nomeLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #F4B315; border: none;");
    nomeLabel->setWordWrap(true);

    QLabel *infoLabel = new QLabel(
        QString("👥 %1/%2 participantes | 🔑 Código: %3")
            .arg(numParticipantes)
            .arg(maxParticipantes)
            .arg(codigoSala)
        );
    infoLabel->setStyleSheet("font-size: 13px; color: #D3AF35; border: none;");

    infoLayout->addWidget(nomeLabel);
    infoLayout->addWidget(infoLabel);
    infoLayout->addStretch();

    QPushButton *entrarBtn = new QPushButton("🔥 Entrar");
    entrarBtn->setCursor(Qt::PointingHandCursor);
    entrarBtn->setFixedSize(110, 45);
    entrarBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #F4B315;"
        "   color: #1A161A;"
        "   border: none;"
        "   border-radius: 8px;"
        "   font-weight: bold;"
        "   font-size: 14px;"
        "}"
        "QPushButton:hover { background-color: #D3AF35; }"
        );

    connect(entrarBtn, &QPushButton::clicked, [this, idSala]() {
        onSalaClicked(idSala);
    });

    mainLayout->addWidget(textContainer, 1);
    mainLayout->addWidget(entrarBtn);

    return card;
}

void GrupoEstudoDialog::onSalaClicked(int idSala)
{
    QSqlQuery query(dbConnection);
    query.prepare("SELECT nome_sala FROM Salas_Estudo WHERE id_sala = ?");
    query.addBindValue(idSala);

    if (!query.exec() || !query.next()) return;

    QString nomeSala = query.value(0).toString();
    int idUsuario = getIdUsuario(loggedInUsername);

    query.prepare("INSERT OR IGNORE INTO Participantes_Sala (id_sala, id_usuario) VALUES (?, ?)");
    query.addBindValue(idSala);
    query.addBindValue(idUsuario);
    query.exec();

    ChatMateria *chat = new ChatMateria(this, loggedInUsername, idSala, nomeSala);
    chat->exec();
    delete chat;
}

void GrupoEstudoDialog::on_materiasButton_clicked() { mostrarTela("materias"); }
void GrupoEstudoDialog::on_gruposButton_clicked() { mostrarTela("grupos"); }
void GrupoEstudoDialog::on_criarButton_clicked() { mostrarTela("criar"); }
void GrupoEstudoDialog::on_categoriaComboBox_currentIndexChanged(int) {
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
        QMessageBox::warning(this, "⚠️ Campo Vazio", "Por favor, digite o código do grupo!");
        return;
    }

    QSqlQuery query(dbConnection);
    query.prepare("SELECT id_sala, tipo, senha, nome_sala FROM Salas_Estudo WHERE codigo_sala = ?");
    query.addBindValue(codigo);

    if (!query.exec() || !query.next()) {
        QMessageBox::warning(this, "⚠️ Grupo Não Encontrado", "Não existe nenhum grupo com este código!");
        return;
    }

    int idSala = query.value(0).toInt();
    QString tipo = query.value(1).toString();
    QString senhaCorreta = query.value(2).toString();
    QString nomeSala = query.value(3).toString();

    if (tipo == "privada" && senha != senhaCorreta) {
        QMessageBox::warning(this, "⚠️ Senha Incorreta", "A senha digitada está incorreta!");
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
