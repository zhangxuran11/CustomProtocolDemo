#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<ztpmanager.h>
#include<ztpprotocol.h>
#include<QHostAddress>
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
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ztpm = new ZTPManager(1235,QHostAddress("224.124.0.1"));
    connect(ztpm,SIGNAL(readyRead()),this,SLOT(fun()));
    ui->textBrowser->textCursor().insertText("wait data...");


}

MainWindow::~MainWindow()
{
    if(ztpm != NULL)
        delete ztpm;
    delete ui;

}

void MainWindow::on_pushButton_clicked()
{

}

void MainWindow::on_pushButton_2_clicked()
{
    ui->textBrowser->clear();
}
