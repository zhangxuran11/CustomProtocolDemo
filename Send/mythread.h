#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <QThread>
#include"ztpmanager.h"
class MyThread : public QThread
{
    Q_OBJECT
public:
    ZTPManager *ztpm;
    explicit MyThread(ZTPManager *_ztpm,QObject *parent = 0);
    
signals:
    
public slots:
    void run();
    
};

#endif // MYTHREAD_H
