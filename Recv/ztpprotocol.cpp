#include "ztpprotocol.h"
#include <QByteArray>
#include<QDebug>
ZTPprotocol::ZTPprotocol(QByteArray &bytes)
{
    load(bytes);
}

//"&head&64位无符号长度&|&paraName1:|:paraValue1&|&paraName2:|:paraValue2&end&"
void ZTPprotocol::load(QByteArray& bytes)
{
    int pos = bytes.indexOf("&head&");
    bytes.remove(pos,17);
    pos = bytes.indexOf("&end&");
    bytes.remove(pos,5);
    QStringList strList = QString::fromUtf8(bytes.data(),bytes.length()).split("&|&");
    for(int i = 0;i<strList.length();i++)
    {
        QStringList subList = strList[i].split(":|:");
        QString k = subList[0];
        QString v = subList[1];
        map.insert(k,v);
    }
}
void ZTPprotocol::clear()
{
    map.clear();
    rawData.clear();
}

void ZTPprotocol::addPara(const QString& paraName,const QString& paraValue)
{
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
        rawData.append(map[keyList[i]].toUtf8());
    }
    rawData.append("&end&");
    qint64 len = rawData.length();
    memcpy(rawData.data()+6,&len,8);
}

