#include "ztpmanager.h"
#include <QtAlgorithms>
#include <QEventLoop>
#include<QThread>


ZTPprotocol::ZTPprotocol(QByteArray &bytes)
{
    load(bytes);
}
static QList<QByteArray> split(const QByteArray& bytes,const QByteArray & sep)
{
    QList<QByteArray> resList;
    int off = 0,pos = 0;
    while(true)
    {
        pos = bytes.indexOf(sep,off);
        if(pos == -1)
        {
            resList.append(bytes.mid(off,-1));
            break;
        }
        else
        {
            resList.append(bytes.mid(off,pos-off));
        }
        off = pos + sep.length();
    }
    return resList;

}
//"&head&64位无符号长度&|&paraName1:|:paraValue1&|&paraName2:|:paraValue2&end&"
void ZTPprotocol::load(QByteArray& bytes)
{
    int pos = bytes.indexOf("&head&");
    bytes.remove(pos,17);
    pos = bytes.indexOf("&end&");
    bytes.remove(pos,5);
    QList<QByteArray> strList = split(bytes,"&|&");
    for(int i = 0;i<strList.length();i++)
    {
        QList<QByteArray> subList = split(strList[i],":|:");
        QString k = subList[0];
        QByteArray v = subList[1];
        map.insert(k,v);
    }
}
void ZTPprotocol::clear()
{
    map.clear();
    rawData.clear();
}

void ZTPprotocol::removePara(const QString& paraName)
{
    map.remove(paraName);
}

void ZTPprotocol::addPara(const QString& paraName,const QString& paraValue)
{
    map.insert(paraName,paraValue.toUtf8());
}

void ZTPprotocol::addPara(const QString& paraName,const QByteArray& paraValue,EType type )
{
    if(type == FILE)
        map.insert(paraName,paraValue);
}
void ZTPprotocol::genarate()
{
    QList<QString> keyList = map.keys();
    rawData = "&head&00000000";
    for(int i = 0;i<keyList.length();i++)
    {
        rawData.append("&|&");
        rawData.append(keyList[i].toUtf8());
        rawData.append(":|:");
        rawData.append(map[keyList[i]]);
    }
    rawData.append("&end&");
    qint64 len = rawData.length();
    memcpy(rawData.data()+6,&len,8);
}






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
