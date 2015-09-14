#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<ztpmanager.h>
#include<ztpprotocol.h>
#include<QHostAddress>
#include<QFile>
void MainWindow::fun()
{
    qDebug("entry func !!\n");
    ZTPprotocol ztpp;
    ztpm->getOneZtp(ztpp);

    QList<QString> paraList = ztpp.paras();
    for(int i = 0;i< paraList.length();i++)
    {
        QString text = "\n"+QString::number(i)+"---"+paraList[i]+" : "+ztpp.getPara(paraList[i]);
        qDebug()<<text;
        ui->textBrowser->textCursor().insertText(text);
    }
    if(ztpp.getPara("reponse") == "hi")
        return;
    ztpp.clear();
    ztpp.addPara("reponse","hi");
    ztpm->SendOneZtp(ztpp,QHostAddress("224.124.0.1"),1235);
}

void MainWindow::fun2()
{
    qDebug()<<"1111111111111111111111111111111111";
    QFile file("test1.png");
    file.open(QFile::Truncate|QFile::WriteOnly);
    file.write(udp->readAll());
    file.close();
    ui->label->setPixmap(QPixmap("test.png"));
}
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ztpm = new ZTPManager(1235,QHostAddress("224.124.0.1"));
    connect(ztpm,SIGNAL(readyRead()),this,SLOT(fun()));
    ui->textBrowser->textCursor().insertText("wait data...");

    udp = new QUdpSocket;
    udp->bind(QHostAddress::Any,3333,QUdpSocket::ShareAddress);
    udp->joinMulticastGroup(QHostAddress("224.124.0.1"));
    connect(udp,SIGNAL(readyRead()),this,SLOT(fun2()));

}

MainWindow::~MainWindow()
{
    if(ztpm != NULL)
        delete ztpm;
    delete ui;
    delete udp;

}

void MainWindow::on_pushButton_clicked()
{
    //ui->label->setPixmap(QPixmap("test.png"));

}

void MainWindow::on_pushButton_2_clicked()
{
    ui->textBrowser->clear();
}
