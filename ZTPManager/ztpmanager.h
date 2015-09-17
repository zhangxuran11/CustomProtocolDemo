#ifndef ZTPMANAGER_H
#define ZTPMANAGER_H
#include<QList>
#include<QThread>
#include<QHostAddress>
#include<QUdpSocket>
#include<QDateTime>
#include<QByteArray>
#include<QMap>
#include <QStringList>
#include<QTimer>
#include "ztpprotocol.h"

class ZTPprotocol;
class Fragment;
class FragmentList;
class ZTPManager : public QObject
{

    Q_OBJECT

    QList<ZTPprotocol*> ztpList;
    QMap<quint16,FragmentList*> workMap;
    QUdpSocket _Socketlistener;
    int MTU;
    int _timeout;
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
    explicit ZTPManager(quint16 port,QHostAddress groupAddress = QHostAddress::Any, QObject *parent = 0);
    ResultState getOneZtp(ZTPprotocol& ztp);//异步读取
    ResultState waitOneZtp(ZTPprotocol& ztp,int msecs = 3000);//同步读取
    ResultState SendOneZtp(ZTPprotocol& ztp,const QHostAddress& host,quint16 port);

    ~ZTPManager();

private slots:
    void onRead();
    void onTimeout(quint16 identifier);
    void msleep(int msecs);
};



#endif // ZTPMANAGER_H
