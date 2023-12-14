#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<iostream>
#include <QAction>
#include <QShortcut>
#include <QKeySequence>
#include <QDebug>
#include <QPlainTextEdit>
#include<thread>
#include<vector>
#include<QCheckBox>
#include<QMetaObject>
using namespace std;

std::map<QString, QString> chats;

MainWindow::MainWindow(QWidget *parent, QString name)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->user = new client(name.toStdString(), 12345, "192.168.1.104");
    this->name=name;
    qDebug()<<this->name;
    user->creatingSocket();
    user->initializeAddrAndConnect();

    std::thread clientThread(&MainWindow::receive, this);
    clientThread.detach();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_sendBut_clicked()
{
    if (getSelectedUserName() != "") {
        user->sendMessages(ui->SendMessage_3->toPlainText(), name, getSelectedUserName());
        ui->textBrowser_3->append(QString::fromStdString(user->name) + ": " + ui->SendMessage_3->toPlainText());
        chats[getSelectedUserName()] += QString::fromStdString(user->name) + ": " + ui->SendMessage_3->toPlainText()+"\n";
        ui->SendMessage_3->setPlainText("");
    }
}

void MainWindow::receive(){
    char get[1000];
    int bytesRead;

    while (true) {
        bytesRead = recv(user->clientSocket, get, sizeof(get) - 1, 0);
        get[bytesRead] = '\0';
        std::cout<<"Get: "<<get<<std::endl;
        std::string sender = parseJson(get, 's'), message = parseJson(get, 'm');

        if (bytesRead > 0) {
            {
                if(sender=="server"){
                    std::cout<<"im message from server\n";
                    user->usersNames.push_back(message);
                    std::cout<<"new client: "<<message<<std::endl;

                    int i=0;
                    for(auto it=chats.begin();it!=chats.end();it++){
                        if(it->first!=QString::fromStdString(message))
                            i++;
                    }
                    if(i==chats.size()){
                    for(int i=0;i<user->usersNames.size();i++){
                        ui->receiver_3->addItem(QString::fromStdString(user->usersNames[i]));
                        chats.insert(std::make_pair(QString::fromStdString(user->usersNames[i]), ""));
                    }
                    }
                }
                else{
                    QString s= QString::fromStdString(sender)+QString::fromStdString(": ")+QString::fromStdString(message);
                    chats[QString::fromStdString(sender)] += s;
                    if (getSelectedUserName() == QString::fromStdString(sender)) {
                        ui->textBrowser_4->append(s);
                    }
                }
            }
        }
        else if (bytesRead == 0) {
            std::cout << "Server is off. End of chat " << std::endl;
            break;
        }
        else {
            std::cerr << "Error of receiving data: " << WSAGetLastError() << std::endl;
            break;
        }
    }
}

QString MainWindow::getSelectedUserName() {
    if (ui->receiver_3->currentItem() == nullptr) {
        return QString();
    }

    QString selectedUserName = ui->receiver_3->currentItem()->text();
    return selectedUserName;
}

std::string MainWindow::parseJson(const char* jsonString, char t) {
    try {
        json jsonObject = json::parse(jsonString);
        std::string res;
        if (t == 'm') {
            res = jsonObject["message"].get<std::string>();
        }
        else if (t == 'r') {
            res = jsonObject["receiver"].get<std::string>();
        }
        else if (t == 's') {
            res = jsonObject["sender"].get<std::string>();
        }
        return res;
    }
    catch (const std::exception& e) {
        std::cerr << "Error parsing JSON";
    }
}

void MainWindow::on_receiver_itemClicked(QListWidgetItem *item)
{
    QString selectedUser = getSelectedUserName();
    ui->textBrowser_4->clear();
    ui->textBrowser_4->append(chats[selectedUser]);
}


void MainWindow::on_createGroup_clicked()
{
    QString group=ui->textBrowser_4->toPlainText();
    if (group != "") {
        user->sendMessages(group, name, "server");
        ui->receiver_3->addItem(group);
        chats.insert(std::make_pair(group, ""));
        ui->textBrowser_4->setPlainText("");
    }
}

