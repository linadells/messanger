#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "client.cpp"
#include<QListWidgetItem>

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
    void updateChat();
    QString getSelectedUserName();
    std::string parseJson(const char* jsonString, char t);
private:
    Ui::MainWindow *ui;
private slots:
    void on_sendBut_clicked();
    void on_receiver_itemClicked(QListWidgetItem *item);
};
#endif // MAINWINDOW_H
