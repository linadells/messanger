#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<iostream>
#include <QDebug>
#include <QPlainTextEdit>
#include<thread>
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

    //    std::thread updateChatThread(&MainWindow::updateChat, this);
    //    updateChatThread.detach();
    //updateUsersList();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_sendBut_clicked()
{
    if (getSelectedUserName() != "") {
        user->sendMessages(ui->SendMessage->toPlainText(), name, getSelectedUserName());
        ui->textBrowser->append(QString::fromStdString(user->name) + ": " + ui->SendMessage->toPlainText());
        ui->SendMessage->setPlainText("");
    }
}

void MainWindow::receive(){

    //     ui->chat->appendPlainText(s);
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

                    std::cout<<"all:\n";
                    for(int i=0;i<user->usersNames.size();i++){
                        std::cout<<user->usersNames[i]<<std::endl;
                        ui->receiver->addItem(QString::fromStdString(user->usersNames[i]));
                        chats.insert(std::make_pair(QString::fromStdString(user->usersNames[i]), ""));
                    }
                }
                else{
                    QString s= QString::fromStdString(sender)+QString::fromStdString(": ")+QString::fromStdString(message);
                    //ui->chat->appendPlainText(s);
                    //ui->textBrowser->append(s);
                    chats[QString::fromStdString(sender)] += s;
                    qDebug()<<s;
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
    if (ui->receiver->currentItem() == nullptr) {
        // No item is selected
        return QString();
    }

    // Get the text of the selected item
    QString selectedUserName = ui->receiver->currentItem()->text();
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
    ui->textBrowser->clear();
    ui->textBrowser->append(chats[selectedUser]);
}
