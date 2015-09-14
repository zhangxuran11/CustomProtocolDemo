#include "ztpmanager.h"
#include <QtAlgorithms>
#include <QEventLoop>
Fragment::Fragment(const QByteArray& bytes)
{
	rawPkg = bytes;
    memcpy(&identifier,bytes.data(),2);
    memcpy(&checksum,bytes.data()+2,2);
    memcpy(&fragment_count,bytes.data()+4,2);
    memcpy(&fragment_offset,bytes.data()+6,2);
    memcpy(&len,bytes.data()+8,4);
    data.append(bytes.data()+12,len);
}
       

void Fragment::generate()
{
    rawPkg = QByteArray(12,0);
    memcpy(rawPkg.data(),&identifier,2);
    memcpy(rawPkg.data()+4,&fragment_count,2);
    memcpy(rawPkg.data()+6,&fragment_offset,2);
    memcpy(rawPkg.data()+8,&len,4);
	rawPkg.append(data);
	checksum = generateChecksum();
    memcpy(rawPkg.data()+2,&checksum,2);
}

quint16 Fragment::generateChecksum(){
	quint16 sum = 0;
    for(quint32 i = 0;i<len+12;i++)
	{
		if(i>15 && i <32)
			continue;
		sum += data[i];
	}
	return sum;
}

FragmentList::FragmentList(quint16 identifier){
			this->identifier = identifier;
			connect(&timer,SIGNAL(timeout()),this,SLOT(send_timeout()));
		}
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
    connect(&_Socketlistener,SIGNAL(readyRead()),this,SLOT(onRead()));
    MTU = 65507;
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
    connect(&_Socketlistener,SIGNAL(readyRead()),this,SLOT(onRead()));
    MTU = 65507;
}
ZTPManager::ResultState ZTPManager::getOneZtp(ZTPprotocol& ztp)
{
    if(ztpList.isEmpty())
    {
        qDebug()<<"ZTP LIST is empty!!";
        return FAILED;
    }
    ZTPprotocol* pztp = ztpList.takeFirst();
    ztp = *pztp;
    delete pztp;
    return SUCCESS;
//    QHostAddress remoteHost;
//    quint16 remotePort;
//    QByteArray recvBuff(_Socketlistener.pendingDatagramSize(),'\0');
//    qint64 pendingLen = _Socketlistener.readDatagram(recvBuff.data(),recvBuff.length(),&remoteHost,&remotePort);
//    qint64 len;
//    memcpy(&len,recvBuff.data()+6,8);
//    if(pendingLen!=len)
//    {
//        qDebug("recv ZTP data error: has data %lld bytes and actually recv %lld bytes!!\n",
//               len,pendingLen);
//        return FAILED;
//    }
//    ztp.clear();
//    ztp.load(recvBuff);
//    ztp.addPara("RemoteHost",remoteHost.toString());
//    ztp.addPara("RemotePort",QString::number(remotePort));

//    return SUCCESS;
}
ZTPManager::ResultState ZTPManager::waitOneZtp(ZTPprotocol& ztp,int msecs)
{


    QEventLoop q;
    QTimer::singleShot(msecs,&q,SLOT(quit()));
    connect(this,SIGNAL(readyRead()),&q,SLOT(quit()));
    if(getOneZtp(ztp) == FAILED)
    {
        return TIMEOUT;
    }
    else
    {
        return SUCCESS;
    }
//    QHostAddress remoteHost;
//    quint16 remotePort;
//    if(!_Socketlistener.waitForReadyRead(msecs))
//    {
//        if(_Socketlistener.error() == QUdpSocket::SocketTimeoutError)
//            return TIMEOUT;
//        else
//        {
//            qDebug("Socket error!!\n");
//            return FAILED;
//        }
//    }
//    QByteArray recvBuff(_Socketlistener.pendingDatagramSize(),'\0');
//    qint64 pendingLen = _Socketlistener.readDatagram(recvBuff.data(),recvBuff.length(),&remoteHost,&remotePort);
//    qint64 len;
//    memcpy(&len,recvBuff.data()+6,8);
//    if(pendingLen!=len)
//    {
//        qDebug("recv ZTP data error: has data %lld bytes and actually recv %lld bytes!!\n",
//               len,pendingLen);
//        return FAILED;
//    }
//    ztp.clear();
//    ztp.load(recvBuff);
//    ztp.addPara("RemoteHost",remoteHost.toString());
//    ztp.addPara("RemotePort",QString::number(remotePort));

//    return SUCCESS;
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
        qDebug()<<"1 ztp.getRwaData len = "<<ztp.getRwaData().length() << "ztp.getRwaData()= "<<ztp.getRwaData();
        fragment.data = ztp.getRwaData().left(MTU);
        qDebug()<<"MTU = "<<MTU;
        ztp.getRwaData().remove(0,MTU);
        qDebug()<<"2 ztp.getRwaData len = "<<ztp.getRwaData().length() << "ztp.getRwaData()= "<<ztp.getRwaData();
        qDebug()<<"3 fragment.data len = "<<fragment.data.length() << "fragment.data()= "<<fragment.data;
		fragment.len = fragment.data.length();
        qDebug()<<"fragment.len = "<<fragment.len << "fragment.data= "<<fragment.data;
		fragment.generate();
        for(int i = 0;i < fragment.rawPkg.length();i++)
        {
            //qDebug("%2d %2x---%c\n",i,(unsigned char)fragment.rawPkg.data()[i],(unsigned char)fragment.rawPkg.data()[i]);
        }
		int sendLen = _Socketlistener.writeDatagram(fragment.rawPkg,host,port);
		if(sendLen != fragment.rawPkg.length())
		{
			qDebug("send ZTP data error: has data %d bytes and actually send %d bytes!!\n",
					ztp.getRwaData().length(),sendLen);
			return FAILED;
		}
	}

	return SUCCESS;
}

void ZTPManager::onRead()
{
    qDebug()<<"11111111111111111111111111";
    QHostAddress remoteHost;
    quint16 remotePort;
    QByteArray recvBuff(_Socketlistener.pendingDatagramSize(),'\0');
    qint64 pendingLen = _Socketlistener.readDatagram(recvBuff.data(),recvBuff.length(),&remoteHost,&remotePort);
    qDebug()<<"22222222222222222222222222 pendingLen = "<<pendingLen;
    Fragment* fragment = new Fragment(recvBuff);
    qDebug()<<"333333333333333333333333333";
    qDebug()<<"fragment :"<<fragment->identifier<<" "<<fragment->checksum<<" "<<fragment->fragment_count<<" "<<fragment->fragment_offset<<" "<<fragment->len<<" "<<fragment->data;
    if(!workMap.contains(fragment->identifier))
	{
        qDebug()<<"4444444444444444444444444444";
        workMap.insert(fragment->identifier,new FragmentList(fragment->identifier));
        qDebug()<<"55555555555555555555555555";
        //connect(workMap[fragment->identifier],SIGNAL(timeout(quint16)),this,SLOT(onTimeout(quint16)));
    }
    qDebug()<<"6666666666666666666666666666666";
    workMap[fragment->identifier]->timer.start(_timeout);
    qDebug()<<"7777777777777777777777777777";
    workMap[fragment->identifier]->fragment_list.append(fragment);
    qDebug()<<"88888888888888888888888888";
    if(workMap[fragment->identifier]->fragment_list.length() == fragment->fragment_count)
	{
        qDebug()<<"9999999999999999999999999999";
        qSort(workMap[fragment->identifier]->fragment_list);
        qDebug()<<"000000000000000000000000000000";
		QByteArray recvBuff;
        for(int i = 0; i < fragment->fragment_count;i++)
        {
            qDebug()<<"11 11 11 1111 11 11 11 11 11 11 11 11 11 11 11 11";
            recvBuff.append(workMap[fragment->identifier]->fragment_list[i]->data);
        }
        qDebug()<<"12 12 12 12 12 12 12 12 12 12 12 12 12 12 12 12 12 12 ";
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
