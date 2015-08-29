#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<ztpmanager.h>
#include<ztpprotocol.h>
#include<QHostAddress>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ztpm = new ZTPManager(1235,QHostAddress("224.124.0.1"));
}

MainWindow::~MainWindow()
{
    delete ztpm;
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{

    ZTPprotocol ztpp;
    ztpp.addPara("type","hellow");
    ztpm->SendOneZtp(ztpp,QHostAddress("224.124.0.1"),1235);
    ztpp.clear();
    while(1)
    {
        if(ztpm->waitOneZtp(ztpp,100) != ZTPManager::SUCCESS)
            return;
        QList<QString> paraList = ztpp.paras();
        for(int i = 0;i<ztpp.count();i++)
        {
            QString text = "\n"+QString::number(i) + "--" + paraList[i]+" : "+ztpp.getPara(paraList[i]);
            ui->textBrowser->textCursor().insertText(text);
        }
    }

}