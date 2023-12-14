#include "groups.h"
#include "ui_groups.h"
#include<vector>
#include<string>
#include<QStringList>
#include<mainwindow.h>

groups::groups(QWidget *parent, std::vector<std::string> users)
    : QFrame(parent)
    , ui(new Ui::groups)
{
    ui->setupUi(this);

    for(std::string & user : users){
        QString user1 = QString::fromStdString(user);
        QCheckBox *checkBox = new QCheckBox(user1, this);
        checkBoxes.append(checkBox);
        layout->addWidget(checkBox);
    }
}

groups::~groups()
{
    delete ui;
}

QStringList groups::getSelectedParticipants() const {
    QStringList selectedParticipants;
    for (const QCheckBox *checkBox : checkBoxes) {
        if (checkBox->isChecked()) {
            selectedParticipants.append(checkBox->text());
        }
    }
    return selectedParticipants;
}

void groups::on_pushButton_clicked()
{
    MainWindow.listForGroup=getSelectedParticipants();
    hide();
}

