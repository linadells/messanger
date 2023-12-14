#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<iostream>
#include <QDebug>
#include <QPlainTextEdit>
#include<thread>
using namespace std;

MainWindow::MainWindow(QWidget *parent, QString name)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->user = new client(name.toStdString(), 12345, "192.168.1.106");
    this->name=name;
    qDebug()<<this->name;
    user->creatingSocket();
    user->initializeAddrAndConnect();
    std::thread clientThread(&MainWindow::receive, this);
    clientThread.detach();

    //std::thread updateUsersListThread(&MainWindow::updateUsersList, this);
    //updateUsersListThread.detach();
    //updateUsersList();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_sendBut_clicked()
{
    // QString selectedUserName = getSelectedUserName();
    // qDebug()<<selectedUserName;
   // QString message = ui->SendMessage->toPlainText();
   // qDebug()<<message;

    //user->sendMessages(message, name, selectedUserName);
    user->sendMessages(ui->SendMessage->toPlainText(), name, ui->temp->toPlainText());
    ui->chat->appendPlainText(ui->SendMessage->toPlainText());
    ui->chat->appendPlainText("\n");
    ui->SendMessage->setPlainText("");
}

void MainWindow::receive(){
    qDebug()<<"i am in rerceive";
    QString s=user->receiveMessages();
        ui->chat->appendPlainText(s);
    qDebug()<<"hi: ";
    qDebug()<<s;
}

void MainWindow::updateUsersList() {
    //while(true) {
    ui->receiver->clear();
    std::vector<std::string> userNamesVec = user->getUsers();
    // for(int i=0;i<userNamesVec.size();i++){
    //     qDebug()<<userNamesVec[i];
    // }
    QStringList userNames;
    for (const auto &nameOfNewUser : userNamesVec) {
        userNames.append(QString::fromStdString(nameOfNewUser));
    }
    for (const auto &nameOfNewUser : userNames) {
        //nameOfNewUser.toStdString();
        ui->receiver->addItem(nameOfNewUser);
    }
    //}
}

QString MainWindow::getSelectedUserName() {
    // Check if any item is selected
    if (ui->receiver->currentItem() == nullptr) {
        // No item is selected
        return QString();
    }

    // Get the text of the selected item
    QString selectedUserName = ui->receiver->currentItem()->text();
    return selectedUserName;
}
