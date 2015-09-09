#ifndef ZTPMANAGER_H
#define ZTPMANAGER_H
#include<QList>
#include <QThread>
#include<QHostAddress>
#include<QUdpSocket>
#include<ztpprotocol.h>
class ZTPManager : public QObject
{

    Q_OBJECT
    QList<QByteArray> ztpList;
    QUdpSocket _Socketlistener;
signals:
    void readyRead();
public:
    enum ResultState
    {
        SUCCESS,
        TIMEOUT,
        FAILED
    };
    explicit ZTPManager(QHostAddress host = QHostAddress::Any,quint16 port = 0,
               QHostAddress groupAddress = QHostAddress::Any, QObject *parent = 0);
    explicit ZTPManager(quint16 port,
               QHostAddress groupAddress = QHostAddress::Any, QObject *parent = 0);
    ResultState getOneZtp(ZTPprotocol& ztp);//异步读取
    ResultState waitOneZtp(ZTPprotocol& ztp,int msecs = 3000);//同步读取
    ResultState SendOneZtp(ZTPprotocol& ztp,const QHostAddress& host,quint16 port);

signals:
    void new_ztp();
public slots:

};



#endif // ZTPMANAGER_H
