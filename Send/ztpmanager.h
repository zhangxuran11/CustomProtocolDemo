#ifndef ZTPMANAGER_H
#define ZTPMANAGER_H
#include<QList>
#include <QThread>
#include<QHostAddress>
#include<QUdpSocket>
#include<ztpprotocol.h>
#include<QDateTime>
#include<QByteArray>
#include<QMap>
#include<QTimer>

class Fragment//分片
{
    /******************************************************************

      |ˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉ|
     0|  identifier     | checksum          |31
    32|fragment count   | fragment offset   |63
    64|                len                  |95
    96|                data                 |
      |                ....                 |
      |_____________________________________|


    ******************************************************************/
public:
    quint16 identifier;
    quint16 checksum;
    quint16 fragment_count;
    quint16 fragment_offset;
    quint32 len;//仅数据正文长度
    QByteArray data;
    QByteArray rawPkg;
private:

    quint16 generateChecksum();
public:
    Fragment(){}
    explicit Fragment(const QByteArray& bytes);
    bool isValid(){return checksum == generateChecksum();}
    void generate();
    bool operator <(const Fragment& frag){return fragment_offset < frag.fragment_offset;}
};
class FragmentList :public QObject
{
    Q_OBJECT
signals:
    void timeout(quint16 identifier);
public:
    quint16 identifier;
    QTimer timer;
    QList<Fragment*> fragment_list;
    FragmentList(quint16 identifier);
    ~FragmentList()
    {
        for(QList<Fragment*>::iterator it = fragment_list.begin();it !=fragment_list.end();it++)
        {
            delete *it;
        }
    }
private slots:
    void send_timeout(){emit timeout(identifier);}

};
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

    ~ZTPManager()
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

private slots:
    void onRead();
	void onTimeout(quint16 identifier){
        FragmentList* node = workMap[identifier];
		workMap.remove(identifier);
        delete node;
    }

};



#endif // ZTPMANAGER_H
