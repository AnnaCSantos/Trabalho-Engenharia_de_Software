#include "avaliacaomateriasdialog.h"
#include "ui_avaliacaomateriasdialog.h"
#include "mainwindow.h"
#include "perfildialog.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QProgressBar>
#include <QButtonGroup>
#include <QRadioButton>
#include <QEvent>
#include <QMouseEvent>
#include <QComboBox>

AvaliacaoMateriasDialog::AvaliacaoMateriasDialog(QWidget *parent, const QString& username)
    : QDialog(parent)
    , ui(new Ui::AvaliacaoMateriasDialog)
    , loggedInUsername(username)
{
    ui->setupUi(this);
    setWindowTitle("Avaliação de Dificuldade das Matérias");
    resize(1200, 800);

    setupDatabase();
    criarTabelasNecessarias();
    carregarCategorias();

    // Configura layout principal
    layoutPrincipal = new QVBoxLayout(ui->containerMaterias);
    layoutPrincipal->setSpacing(15);
    layoutPrincipal->setContentsMargins(15, 15, 15, 15);
    ui->containerMaterias->setLayout(layoutPrincipal);

    // Event filters para navegação
    ui->homeButton->installEventFilter(this);
    ui->perfilButton->installEventFilter(this);

    // Conecta mudança de categoria
    connect(ui->categoriaComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AvaliacaoMateriasDialog::onCategoriaChanged);

    // Carrega primeira categoria
    if (ui->categoriaComboBox->count() > 0) {
        onCategoriaChanged(0);
    }
}

AvaliacaoMateriasDialog::~AvaliacaoMateriasDialog()
{
    delete ui;
}

void AvaliacaoMateriasDialog::setupDatabase()
{
    dbConnection = QSqlDatabase::database("qt_sql_default_connection");
    if (!dbConnection.isOpen()) {
        qDebug() << "[AvaliacaoMaterias] ERRO: Banco não está aberto.";
    }
}

void AvaliacaoMateriasDialog::criarTabelasNecessarias()
{
    QSqlQuery query(dbConnection);

    // Tabela de votos de dificuldade
    query.exec(
        "CREATE TABLE IF NOT EXISTS Avaliacoes_Dificuldade ("
        "id_avaliacao INTEGER PRIMARY KEY AUTOINCREMENT, "
        "id_materia INTEGER NOT NULL, "
        "id_usuario INTEGER NOT NULL, "
        "nivel TEXT NOT NULL CHECK(nivel IN ('facil', 'medio', 'dificil')), "
        "data_avaliacao DATETIME DEFAULT CURRENT_TIMESTAMP, "
        "FOREIGN KEY (id_materia) REFERENCES Materias_Forum(id_materia), "
        "FOREIGN KEY (id_usuario) REFERENCES Usuario(id_usuario), "
        "UNIQUE(id_materia, id_usuario))"
        );

    // Tabela de notas (1 a 5 estrelas)
    query.exec(
        "CREATE TABLE IF NOT EXISTS Notas_Materias ("
        "id_nota INTEGER PRIMARY KEY AUTOINCREMENT, "
        "id_materia INTEGER NOT NULL, "
        "id_usuario INTEGER NOT NULL, "
        "nota INTEGER NOT NULL CHECK(nota BETWEEN 1 AND 5), "
        "data_nota DATETIME DEFAULT CURRENT_TIMESTAMP, "
        "FOREIGN KEY (id_materia) REFERENCES Materias_Forum(id_materia), "
        "FOREIGN KEY (id_usuario) REFERENCES Usuario(id_usuario), "
        "UNIQUE(id_materia, id_usuario))"
        );
}

void AvaliacaoMateriasDialog::carregarCategorias()
{
    QSqlQuery query(dbConnection);
    query.exec("SELECT id_categoria, nome FROM Categorias_Materias ORDER BY nome");

    while (query.next()) {
        int idCat = query.value(0).toInt();
        QString nomeCat = query.value(1).toString();
        ui->categoriaComboBox->addItem(nomeCat, idCat);
    }
}

int AvaliacaoMateriasDialog::getIdUsuario(const QString& username)
{
    QSqlQuery query(dbConnection);
    query.prepare("SELECT id_usuario FROM Usuario WHERE usuario = ?");
    query.addBindValue(username);

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    return -1;
}

bool AvaliacaoMateriasDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        if (obj == ui->homeButton) {
            voltarParaHome();
            return true;
        }

        if (obj == ui->perfilButton) {
            PerfilDialog *perfilDialog = new PerfilDialog(this, loggedInUsername);
            perfilDialog->exec();
            delete perfilDialog;
            return true;
        }
    }
    return QDialog::eventFilter(obj, event);
}

void AvaliacaoMateriasDialog::voltarParaHome()
{
    MainWindow *mainWin = new MainWindow();
    mainWin->setLoggedInUser(loggedInUsername);
    mainWin->show();
    this->close();
}

void AvaliacaoMateriasDialog::onCategoriaChanged(int index)
{
    if (index < 0) return;

    int idCategoria = ui->categoriaComboBox->currentData().toInt();
    carregarMaterias(idCategoria);
}

void AvaliacaoMateriasDialog::carregarMaterias(int idCategoria)
{
    // Limpa layout
    QLayoutItem *item;
    while ((item = layoutPrincipal->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    // Busca matérias da categoria
    QSqlQuery query(dbConnection);
    query.prepare(
        "SELECT m.id_materia, m.nome, "
        "(SELECT COUNT(*) FROM Avaliacoes_Dificuldade WHERE id_materia = m.id_materia AND nivel = 'facil') as facil, "
        "(SELECT COUNT(*) FROM Avaliacoes_Dificuldade WHERE id_materia = m.id_materia AND nivel = 'medio') as medio, "
        "(SELECT COUNT(*) FROM Avaliacoes_Dificuldade WHERE id_materia = m.id_materia AND nivel = 'dificil') as dificil, "
        "(SELECT AVG(CAST(nota AS FLOAT)) FROM Notas_Materias WHERE id_materia = m.id_materia) as media_notas, "
        "(SELECT COUNT(*) FROM Notas_Materias WHERE id_materia = m.id_materia) as total_notas, "
        "(SELECT COUNT(*) FROM Duvidas_Forum WHERE id_materia = m.id_materia) as total_duvidas "
        "FROM Materias_Forum m "
        "WHERE m.id_categoria = ? "
        "ORDER BY m.nome"
        );
    query.addBindValue(idCategoria);

    if (!query.exec()) {
        qDebug() << "Erro ao carregar matérias:" << query.lastError().text();
        return;
    }

    int count = 0;
    while (query.next()) {
        int idMateria = query.value(0).toInt();
        QString nome = query.value(1).toString();
        int facil = query.value(2).toInt();
        int medio = query.value(3).toInt();
        int dificil = query.value(4).toInt();
        double mediaNotas = query.value(5).toDouble();
        int totalNotas = query.value(6).toInt();
        int totalDuvidas = query.value(7).toInt();

        criarCardMateria(idMateria, nome, facil, medio, dificil, mediaNotas, totalNotas, totalDuvidas);
        count++;
    }

    if (count == 0) {
        QLabel *emptyLabel = new QLabel("Nenhuma matéria encontrada nesta categoria.");
        emptyLabel->setStyleSheet(
            "color: #8E6915; font-size: 14px; padding: 40px; "
            "background-color: rgba(66, 55, 56, 0.7); border-radius: 10px;"
            );
        emptyLabel->setAlignment(Qt::AlignCenter);
        layoutPrincipal->addWidget(emptyLabel);
    }

    layoutPrincipal->addStretch();
}

void AvaliacaoMateriasDialog::criarCardMateria(int idMateria, const QString& nome,
                                               int facil, int medio, int dificil,
                                               double mediaNotas, int totalNotas, int totalDuvidas)
{
    QFrame *card = new QFrame();
    card->setFrameShape(QFrame::StyledPanel);
    card->setStyleSheet(
        "QFrame {"
        "   background-color: rgba(66, 55, 56, 0.95);"
        "   border-left: 5px solid #F4B315;"
        "   border-radius: 12px;"
        "   padding: 20px;"
        "   margin: 5px 0;"
        "}"
        );

    QVBoxLayout *mainLayout = new QVBoxLayout(card);
    mainLayout->setSpacing(15);

    // Título da matéria
    QLabel *tituloLabel = new QLabel(nome);
    tituloLabel->setStyleSheet(
        "font-size: 18px; font-weight: bold; color: #FFFFFF; background: transparent;"
        );
    tituloLabel->setWordWrap(true);
    mainLayout->addWidget(tituloLabel);

    // ========== SEÇÃO DE NOTAS (1-5 ESTRELAS) ==========
    QFrame *notasFrame = new QFrame();
    notasFrame->setStyleSheet(
        "QFrame { background-color: rgba(26, 22, 26, 0.6); border-radius: 8px; padding: 15px; }"
        );
    QVBoxLayout *notasLayout = new QVBoxLayout(notasFrame);

    // Média de notas
    QHBoxLayout *mediaLayout = new QHBoxLayout();
    QLabel *mediaLabel = new QLabel(QString("★ Nota Média: %1/5.0")
                                        .arg(mediaNotas, 0, 'f', 1));
    mediaLabel->setStyleSheet(
        QString("color: %1; font-size: 16px; font-weight: bold; background: transparent;")
            .arg(mediaNotas >= 4.0 ? "#27AE60" :
                     mediaNotas >= 3.0 ? "#F39C12" : "#E74C3C")
        );

    QLabel *totalNotasLabel = new QLabel(QString("(%1 avaliações)").arg(totalNotas));
    totalNotasLabel->setStyleSheet("color: #8E6915; font-size: 12px; background: transparent;");

    mediaLayout->addWidget(mediaLabel);
    mediaLayout->addWidget(totalNotasLabel);
    mediaLayout->addStretch();
    notasLayout->addLayout(mediaLayout);

    // Verifica se usuário já avaliou
    int idUsuario = getIdUsuario(loggedInUsername);
    QSqlQuery notaQuery(dbConnection);
    notaQuery.prepare("SELECT nota FROM Notas_Materias WHERE id_materia = ? AND id_usuario = ?");
    notaQuery.addBindValue(idMateria);
    notaQuery.addBindValue(idUsuario);

    bool jaAvaliou = notaQuery.exec() && notaQuery.next();
    int notaAtual = jaAvaliou ? notaQuery.value(0).toInt() : 0;

    // Botões de avaliação (1 a 5 estrelas)
    QLabel *avalieLabel = new QLabel(jaAvaliou ? "Sua nota:" : "Avalie esta matéria:");
    avalieLabel->setStyleSheet("color: #F4B315; font-weight: bold; margin-top: 8px; background: transparent;");
    notasLayout->addWidget(avalieLabel);

    QHBoxLayout *estrelasLayout = new QHBoxLayout();

    for (int i = 1; i <= 5; i++) {
        QPushButton *estrelaBt= new QPushButton(QString("★ %1").arg(i));
        estrelaBt->setMinimumSize(70, 35);

        QString cor = i <= 2 ? "#E74C3C" : i <= 3 ? "#F39C12" : "#27AE60";

        if (jaAvaliou) {
            estrelaBt->setEnabled(false);
            if (i == notaAtual) {
                estrelaBt->setStyleSheet(
                    QString("QPushButton { background-color: %1; color: white; "
                            "border: none; border-radius: 6px; font-weight: bold; "
                            "font-size: 14px; }").arg(cor)
                    );
            } else {
                estrelaBt->setStyleSheet(
                    "QPushButton { background-color: rgba(66, 55, 56, 0.5); "
                    "color: #8E6915; border: none; border-radius: 6px; }"
                    );
            }
        } else {
            estrelaBt->setStyleSheet(
                QString("QPushButton {"
                        "   background-color: rgba(66, 55, 56, 0.8);"
                        "   color: white; border: 2px solid %1;"
                        "   border-radius: 6px; font-weight: bold; font-size: 14px;"
                        "}"
                        "QPushButton:hover { background-color: %1; }").arg(cor)
                );

            connect(estrelaBt, &QPushButton::clicked, [this, idMateria, i]() {
                avaliarNota(idMateria, i);
            });
        }

        estrelasLayout->addWidget(estrelaBt);
    }

    estrelasLayout->addStretch();
    notasLayout->addLayout(estrelasLayout);
    mainLayout->addWidget(notasFrame);

    // ========== SEÇÃO DE DIFICULDADE ==========
    int totalVotos = facil + medio + dificil;

    QLabel *dificuldadeTitle = new QLabel(QString("Nível de Dificuldade (%1 votos | %2 dúvidas)")
                                              .arg(totalVotos).arg(totalDuvidas));
    dificuldadeTitle->setStyleSheet("color: #D3AF35; font-size: 13px; font-weight: bold; margin-top: 10px; background: transparent;");
    mainLayout->addWidget(dificuldadeTitle);

    // Barras de dificuldade
    if (totalVotos > 0) {
        QFrame *barsFrame = new QFrame();
        barsFrame->setStyleSheet("background: transparent;");
        QVBoxLayout *barsLayout = new QVBoxLayout(barsFrame);
        barsLayout->setSpacing(8);

        // Fácil
        QHBoxLayout *facilLayout = new QHBoxLayout();
        QLabel *facilLabel = new QLabel("Fácil:");
        facilLabel->setStyleSheet("color: #27AE60; font-weight: bold; min-width: 60px; background: transparent;");
        QProgressBar *facilBar = new QProgressBar();
        facilBar->setMaximum(totalVotos);
        facilBar->setValue(facil);
        facilBar->setTextVisible(true);
        facilBar->setFormat(QString("%1 (%2%)").arg(facil).arg((facil * 100) / totalVotos));
        facilBar->setStyleSheet(
            "QProgressBar { background-color: rgba(39, 174, 96, 0.2); border: none; "
            "border-radius: 5px; text-align: center; color: white; height: 20px; }"
            "QProgressBar::chunk { background-color: #27AE60; border-radius: 5px; }"
            );
        facilLayout->addWidget(facilLabel);
        facilLayout->addWidget(facilBar);
        barsLayout->addLayout(facilLayout);

        // Médio
        QHBoxLayout *medioLayout = new QHBoxLayout();
        QLabel *medioLabel = new QLabel("Médio:");
        medioLabel->setStyleSheet("color: #F39C12; font-weight: bold; min-width: 60px; background: transparent;");
        QProgressBar *medioBar = new QProgressBar();
        medioBar->setMaximum(totalVotos);
        medioBar->setValue(medio);
        medioBar->setTextVisible(true);
        medioBar->setFormat(QString("%1 (%2%)").arg(medio).arg((medio * 100) / totalVotos));
        medioBar->setStyleSheet(
            "QProgressBar { background-color: rgba(243, 156, 18, 0.2); border: none; "
            "border-radius: 5px; text-align: center; color: white; height: 20px; }"
            "QProgressBar::chunk { background-color: #F39C12; border-radius: 5px; }"
            );
        medioLayout->addWidget(medioLabel);
        medioLayout->addWidget(medioBar);
        barsLayout->addLayout(medioLayout);

        // Difícil
        QHBoxLayout *dificilLayout = new QHBoxLayout();
        QLabel *dificilLabel = new QLabel("Difícil:");
        dificilLabel->setStyleSheet("color: #E74C3C; font-weight: bold; min-width: 60px; background: transparent;");
        QProgressBar *dificilBar = new QProgressBar();
        dificilBar->setMaximum(totalVotos);
        dificilBar->setValue(dificil);
        dificilBar->setTextVisible(true);
        dificilBar->setFormat(QString("%1 (%2%)").arg(dificil).arg((dificil * 100) / totalVotos));
        dificilBar->setStyleSheet(
            "QProgressBar { background-color: rgba(231, 76, 60, 0.2); border: none; "
            "border-radius: 5px; text-align: center; color: white; height: 20px; }"
            "QProgressBar::chunk { background-color: #E74C3C; border-radius: 5px; }"
            );
        dificilLayout->addWidget(dificilLabel);
        dificilLayout->addWidget(dificilBar);
        barsLayout->addLayout(dificilLayout);

        mainLayout->addWidget(barsFrame);
    }

    // Botões de votação de dificuldade
    QSqlQuery votoQuery(dbConnection);
    votoQuery.prepare("SELECT nivel FROM Avaliacoes_Dificuldade WHERE id_materia = ? AND id_usuario = ?");
    votoQuery.addBindValue(idMateria);
    votoQuery.addBindValue(idUsuario);

    bool jaVotou = votoQuery.exec() && votoQuery.next();
    QString votoAtual = jaVotou ? votoQuery.value(0).toString() : "";

    QLabel *voteLabel = new QLabel(jaVotou ? "Seu voto de dificuldade:" : "Vote na dificuldade:");
    voteLabel->setStyleSheet("color: #F4B315; font-size: 12px; font-weight: bold; margin-top: 5px; background: transparent;");
    mainLayout->addWidget(voteLabel);

    QHBoxLayout *voteBtnsLayout = new QHBoxLayout();

    QPushButton *facilBtn = new QPushButton("Fácil");
    QPushButton *medioBtn = new QPushButton("Médio");
    QPushButton *dificilBtn = new QPushButton("Difícil");

    QString btnStyle =
        "QPushButton {"
        "   background-color: rgba(66, 55, 56, 0.8);"
        "   color: white; border: 2px solid %1;"
        "   border-radius: 8px; padding: 8px 15px;"
        "   font-weight: bold; font-size: 12px;"
        "}"
        "QPushButton:hover { background-color: %1; }"
        "QPushButton:disabled { opacity: 0.5; }";

    facilBtn->setStyleSheet(QString(btnStyle).arg("#27AE60"));
    medioBtn->setStyleSheet(QString(btnStyle).arg("#F39C12"));
    dificilBtn->setStyleSheet(QString(btnStyle).arg("#E74C3C"));

    if (jaVotou) {
        facilBtn->setEnabled(false);
        medioBtn->setEnabled(false);
        dificilBtn->setEnabled(false);

        if (votoAtual == "facil") facilBtn->setText("✓ Fácil");
        else if (votoAtual == "medio") medioBtn->setText("✓ Médio");
        else if (votoAtual == "dificil") dificilBtn->setText("✓ Difícil");
    } else {
        connect(facilBtn, &QPushButton::clicked, [this, idMateria]() {
            votarDificuldade(idMateria, "facil");
        });
        connect(medioBtn, &QPushButton::clicked, [this, idMateria]() {
            votarDificuldade(idMateria, "medio");
        });
        connect(dificilBtn, &QPushButton::clicked, [this, idMateria]() {
            votarDificuldade(idMateria, "dificil");
        });
    }

    voteBtnsLayout->addWidget(facilBtn);
    voteBtnsLayout->addWidget(medioBtn);
    voteBtnsLayout->addWidget(dificilBtn);
    voteBtnsLayout->addStretch();

    mainLayout->addLayout(voteBtnsLayout);

    layoutPrincipal->addWidget(card);
}

void AvaliacaoMateriasDialog::votarDificuldade(int idMateria, const QString& nivel)
{
    int idUsuario = getIdUsuario(loggedInUsername);

    QSqlQuery insertQuery(dbConnection);
    insertQuery.prepare(
        "INSERT OR REPLACE INTO Avaliacoes_Dificuldade (id_materia, id_usuario, nivel) "
        "VALUES (?, ?, ?)"
        );
    insertQuery.addBindValue(idMateria);
    insertQuery.addBindValue(idUsuario);
    insertQuery.addBindValue(nivel);

    if (insertQuery.exec()) {
        QMessageBox::information(this, "Sucesso", "Seu voto de dificuldade foi registrado!");
        onCategoriaChanged(ui->categoriaComboBox->currentIndex());
    } else {
        QMessageBox::critical(this, "Erro", "Erro ao registrar voto: " +
                                                insertQuery.lastError().text());
    }
}

void AvaliacaoMateriasDialog::avaliarNota(int idMateria, int nota)
{
    int idUsuario = getIdUsuario(loggedInUsername);

    QSqlQuery insertQuery(dbConnection);
    insertQuery.prepare(
        "INSERT OR REPLACE INTO Notas_Materias (id_materia, id_usuario, nota) "
        "VALUES (?, ?, ?)"
        );
    insertQuery.addBindValue(idMateria);
    insertQuery.addBindValue(idUsuario);
    insertQuery.addBindValue(nota);

    if (insertQuery.exec()) {
        QMessageBox::information(this, "Sucesso",
                                 QString("Você avaliou com %1 estrela(s)!").arg(nota));
        onCategoriaChanged(ui->categoriaComboBox->currentIndex());
    } else {
        QMessageBox::critical(this, "Erro", "Erro ao registrar nota: " +
                                                insertQuery.lastError().text());
    }
}
