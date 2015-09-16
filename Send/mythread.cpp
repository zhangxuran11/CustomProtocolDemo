#include "mythread.h"

MyThread::MyThread(ZTPManager *_ztpm,QObject *parent) :
    QThread(parent)
{
    ztpm = _ztpm;
}
void MyThread::run()
{

    while(1)
    {
        ZTPprotocol ztpp;
        ztpm->waitOneZtp(ztpp);
        QList<QString> paraList = ztpp.paras();
        for(int i = 0;i< paraList.length();i++)
        {
            QString text = "\n"+QString::number(i)+"---"+paraList[i]+" : "+ztpp.getPara(paraList[i]);
            qDebug()<<text;
            //ui->textBrowser->textCursor().insertText(text);
        }
    }

}
