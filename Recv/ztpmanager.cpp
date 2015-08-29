#include "ztpmanager.h"

ZTPManager::ZTPManager(QHostAddress host,quint16 port,
                       QHostAddress groupAddress, QObject *parent):
    QObject(parent)
{

    _Socketlistener.bind(host,port,QUdpSocket::ShareAddress);
    if(groupAddress.toIPv4Address()>0xe0000000 && groupAddress.toIPv4Address()<0xf0000000)
    {
        _Socketlistener.joinMulticastGroup(groupAddress);
        //_Socketlistener.setSocketOption(QAbstractSocket::MulticastTtlOption, 5);
    }
    connect(&_Socketlistener,SIGNAL(readyRead()),this,SIGNAL(readyRead()));
}
ZTPManager::ZTPManager(quint16 port,
           QHostAddress groupAddress, QObject *parent):
    QObject(parent)
{
    _Socketlistener.bind(QHostAddress::Any,port,QUdpSocket::ShareAddress);
    if(groupAddress.toIPv4Address()>0xe0000000 && groupAddress.toIPv4Address()<0xf0000000)
    {
        _Socketlistener.joinMulticastGroup(groupAddress);
        //_Socketlistener.setSocketOption(QAbstractSocket::MulticastTtlOption, 5);
    }
    connect(&_Socketlistener,SIGNAL(readyRead()),this,SIGNAL(readyRead()));
}
ZTPManager::ResultState ZTPManager::getOneZtp(ZTPprotocol& ztp)
{
    QHostAddress remoteHost;
    quint16 remotePort;
    QByteArray recvBuff(_Socketlistener.pendingDatagramSize(),'\0');
    qint64 pendingLen = _Socketlistener.readDatagram(recvBuff.data(),recvBuff.length(),&remoteHost,&remotePort);
    qint64 len;
    memcpy(&len,recvBuff.data()+6,8);
    if(pendingLen!=len)
    {
        qDebug("recv ZTP data error: has data %lld bytes and actually recv %lld bytes!!\n",
               len,pendingLen);
        return FAILED;
    }
    ztp.clear();
    ztp.load(recvBuff);
    ztp.addPara("RemoteHost",remoteHost.toString());
    ztp.addPara("RemotePort",QString::number(remotePort));

    return SUCCESS;
}
ZTPManager::ResultState ZTPManager::waitOneZtp(ZTPprotocol& ztp,int msecs)
{

    QHostAddress remoteHost;
    quint16 remotePort;
    if(!_Socketlistener.waitForReadyRead(msecs))
    {
        if(_Socketlistener.error() == QUdpSocket::SocketTimeoutError)
            return TIMEOUT;
        else
        {
            qDebug("Socket error!!\n");
            return FAILED;
        }
    }
    QByteArray recvBuff(_Socketlistener.pendingDatagramSize(),'\0');
    qint64 pendingLen = _Socketlistener.readDatagram(recvBuff.data(),recvBuff.length(),&remoteHost,&remotePort);
    qint64 len;
    memcpy(&len,recvBuff.data()+6,8);
    if(pendingLen!=len)
    {
        qDebug("recv ZTP data error: has data %lld bytes and actually recv %lld bytes!!\n",
               len,pendingLen);
        return FAILED;
    }
    ztp.clear();
    ztp.load(recvBuff);
    ztp.addPara("RemoteHost",remoteHost.toString());
    ztp.addPara("RemotePort",QString::number(remotePort));

    return SUCCESS;
}

ZTPManager::ResultState ZTPManager::SendOneZtp(ZTPprotocol& ztp,const QHostAddress &host,quint16 port)
{
    ztp.genarate();
    int sendLen = _Socketlistener.writeDatagram(ztp.getRwaData(),host,port);
    if(sendLen != ztp.getRwaData().length())
    {
        qDebug("send ZTP data error: has data %d bytes and actually send %d bytes!!\n",
               ztp.getRwaData().length(),sendLen);
        return FAILED;
    }
    return SUCCESS;
}
