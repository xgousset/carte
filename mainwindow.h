#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager> // Ajouter cette ligne
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void handleSearch();
    void onSearchFinished(QNetworkReply *reply); // Ajouter cette ligne

private:
    Ui::MainWindow *ui;
    QNetworkAccessManager *networkManager; // Ajouter cette ligne
};

#endif // MAINWINDOW_H
