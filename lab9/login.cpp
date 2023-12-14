#include "login.h"
#include "ui_login.h"
#include "mainwindow.h"
#include<QDebug>

login::login(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::login)
{
    ui->setupUi(this);
}

login::~login()
{
    delete ui;
}

void login::on_loginButton_clicked()
{
    QString name=ui->logName->toPlainText();
    qDebug()<<name;
    window=new MainWindow(nullptr, name);
    window->name=name;
    hide();
    window->show();
}
