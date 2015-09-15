#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<ztpmanager.h>
#include<ztpprotocol.h>
#include<QHostAddress>
#include <QFile>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ztpm = new ZTPManager(1235,QHostAddress("224.124.0.1"));
    connect(ztpm,SIGNAL(readyRead()),this,SLOT(fun()));
}
void MainWindow::fun()
{
    ZTPprotocol ztpp;
    ztpm->getOneZtp(ztpp);

    if(ztpp.getPara("type") == "hellow")
    {
        QList<QString> paraList = ztpp.paras();
        for(int i = 0;i< paraList.length();i++)
        {
            QString text = "\n"+QString::number(i)+"---"+paraList[i]+" : "+ztpp.getPara(paraList[i]);
            qDebug()<<text;
            ui->textBrowser->textCursor().insertText(text);
        }
    }
    else if(ztpp.getPara("type") == "file")
    {
        QFile file("temp_"+ztpp.getPara("name"));
        file.open(QFile::Truncate|QFile::WriteOnly);
        file.write(ztpp.getPara("content"));
        file.close();
        ui->label->setPixmap(file.fileName());
    }
}

MainWindow::~MainWindow()
{
    delete ztpm;
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{

    ZTPprotocol ztpp;
//    ztpp.addPara("type","hellow");
//    ztpm->SendOneZtp(ztpp,QHostAddress("224.124.0.1"),1235);

    QUdpSocket udp;
    udp.bind(QHostAddress::Any);
    QFile file("aaa.png");
    file.open(QFile::ReadOnly);
    ztpp.clear();
    ztpp.addPara("type","file");
    ztpp.addPara("name","aaa.png");
    ztpp.addPara("content",file.readAll(),ZTPprotocol::FILE);
    file.close();
    //qDebug()<<"file readall :"<<file.readAll().length();
    //qDebug()<<"file readall :"<<file.readAll().length();
    //qDebug()<<"udp send len :"<<udp.writeDatagram(file.readAll().data(),65507,QHostAddress("224.124.0.1"),3333);


    ztpm->SendOneZtp(ztpp,QHostAddress("224.124.0.1"),1235);


}

void MainWindow::on_pushButton_2_clicked()
{
    ui->textBrowser->clear();
    ui->label->clear();
}
