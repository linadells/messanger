#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "client.cpp"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr, QString name="");
    ~MainWindow();
    QString name;
    client* user;
    void receive();
    void updateUsersList();
    QString getSelectedUserName();
private:
    Ui::MainWindow *ui;
private slots:
    void on_sendBut_clicked();
};
#endif // MAINWINDOW_H
