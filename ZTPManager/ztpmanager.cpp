#include "ztpmanager.h"
#include"ztpprotocol.h"
#include "fragment.h"
#include <QtAlgorithms>
#include <QEventLoop>
#include<QThread>












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
    connect(&_Socketlistener,SIGNAL(readyRead()),this,SLOT(onRead()),Qt::AutoConnection);
    MTU = 60000;
    _timeout = 3000;
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
    connect(&_Socketlistener,SIGNAL(readyRead()),this,SLOT(onRead()),Qt::AutoConnection);
    MTU = 60000;
    _timeout = 3000;

}
ZTPManager::~ZTPManager()
{
    for(QMap<quint16,FragmentList*>::iterator it = workMap.begin();it !=workMap.end();it++)
    {
        delete it.value();
    }
    for(QList<ZTPprotocol*>::iterator it = ztpList.begin();it !=ztpList.end();it++)
    {
        delete *it;
    }
}
ZTPManager::ResultState ZTPManager::getOneZtp(ZTPprotocol& ztp)
{
    if(ztpList.isEmpty())
    {
//        qDebug()<<"ZTP LIST is empty!!";
        return FAILED;
    }
    ZTPprotocol* pztp = ztpList.takeFirst();
    ztp = *pztp;
    delete pztp;
    return SUCCESS;
}
ZTPManager::ResultState ZTPManager::waitOneZtp(ZTPprotocol& ztp,int msecs)
{
    QEventLoop q;
    QTimer::singleShot(msecs,&q,SLOT(quit()));
    connect(this,SIGNAL(readyRead()),&q,SLOT(quit()));
    q.exec();
    if(getOneZtp(ztp) == FAILED)
    {
        return TIMEOUT;
    }
    else
    {
        return SUCCESS;
    }
}

ZTPManager::ResultState ZTPManager::SendOneZtp(ZTPprotocol& ztp,const QHostAddress &host,quint16 port)
{
	ztp.genarate();
	quint16 identifier = QDateTime::currentMSecsSinceEpoch()&0xffff; //用utc的低16位作为分片标识
	quint16 fragment_count = ztp.getRwaData().length()/MTU+1;

	quint16 fragment_offset = 1;
	for(int i = 0;i < fragment_count;i++)
	{
		Fragment fragment;
		fragment.identifier = identifier; //用utc的低16位作为分片标识
		fragment.fragment_count = fragment_count;
        fragment.fragment_offset = fragment_offset++;
        fragment.data = ztp.getRwaData().left(MTU);
        ztp.getRwaData().remove(0,MTU);
        fragment.len = fragment.data.length();
        fragment.generate();
        int sendLen = _Socketlistener.writeDatagram(fragment.rawPkg,host,port);
		if(sendLen != fragment.rawPkg.length())
		{
			qDebug("send ZTP data error: has data %d bytes and actually send %d bytes!!\n",
                    fragment.len,sendLen);
			return FAILED;
		}
//        qDebug()<<"send---fragment : "<<fragment.identifier<<" "<<fragment.checksum<<" "<<fragment.fragment_count<<" "<<fragment.fragment_offset<<" "<<fragment.data.length();
        msleep(1);
    }

	return SUCCESS;
}
static bool lessThan(const Fragment* frag1, const Fragment* frag2)
 {
     return frag1->fragment_offset < frag2->fragment_offset;
 }
void ZTPManager::msleep(int msecs)
{
    QEventLoop q;
    QTimer::singleShot(msecs,&q,SLOT(quit()));
    q.exec();
}
void ZTPManager::onRead()
{
    QHostAddress remoteHost;
    quint16 remotePort;
    QByteArray recvBuff(_Socketlistener.pendingDatagramSize(),'\0');
    qint64 pendingLen = _Socketlistener.readDatagram(recvBuff.data(),recvBuff.length(),&remoteHost,&remotePort);
    Fragment* fragment = new Fragment(recvBuff);
//    qDebug()<<"fragment :"<<fragment->identifier<<" "<<fragment->checksum<<" "<<fragment->fragment_count<<" "<<fragment->fragment_offset<<" "<<fragment->len;
//    qDebug()<<"fragment isvalid:"<<fragment->isValid();
    if(!workMap.contains(fragment->identifier))
    {
        workMap.insert(fragment->identifier,new FragmentList(fragment->identifier));
        connect(workMap[fragment->identifier],SIGNAL(timeout(quint16)),this,SLOT(onTimeout(quint16)));
    }
    workMap[fragment->identifier]->timer.start(_timeout);
    workMap[fragment->identifier]->fragment_list.append(fragment);
    if(workMap[fragment->identifier]->fragment_list.length() == fragment->fragment_count)
    {
        qSort(workMap[fragment->identifier]->fragment_list.begin(),workMap[fragment->identifier]->fragment_list.end(),lessThan);
		QByteArray recvBuff;
        for(int i = 0; i < fragment->fragment_count;i++)
        {
            recvBuff.append(workMap[fragment->identifier]->fragment_list[i]->data);
        }
        FragmentList* node = workMap[fragment->identifier];
        workMap.remove(fragment->identifier);
        delete node;
		qint64 len;
    	memcpy(&len,recvBuff.data()+6,8);
        if(recvBuff.length()!=len)
    	{
       		qDebug("recv ZTP data error: has data %lld bytes and actually recv %lld bytes!!\n",
                len,pendingLen);
    	}
        ZTPprotocol* ztp = new ZTPprotocol(recvBuff);
        ztp->addPara("RemoteHost",remoteHost.toString());
        ztp->addPara("RemotePort",QString::number(remotePort));
        ztpList.append(ztp);
		emit readyRead();
    }
}

void ZTPManager::onTimeout(quint16 identifier){
    FragmentList* node = workMap[identifier];
    workMap.remove(identifier);
    delete node;
}
