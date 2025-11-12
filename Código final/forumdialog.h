#ifndef FORUMDIALOG_H
#define FORUMDIALOG_H

#include <QDialog>
#include <QSqlDatabase>
#include <QPushButton>
#include <QVBoxLayout>

namespace Ui {
class ForumDialog;
}

class ForumDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ForumDialog(QWidget *parent = nullptr, const QString& username = "");
    ~ForumDialog();

private slots:
    void abrirForum(int idForum, const QString& tituloForum);


private:
    Ui::ForumDialog *ui;
    QString loggedInUsername;
    QSqlDatabase dbConnection;
    QVBoxLayout *forumsLayout;

    void carregarForuns();
    void setupDatabase();
};

#endif // FORUMDIALOG_H
