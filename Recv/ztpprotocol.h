#ifndef ZTPPROTOCOL_H
#define ZTPPROTOCOL_H
#include <QByteArray>
#include <QMap>
#include <QStringList>
class QString;
class ZTPprotocol
{
    QMap<QString, QString> map;
    //QMap<QString, QByteArray> map;
    QByteArray rawData;
public:
    ZTPprotocol(){}
    explicit ZTPprotocol(QByteArray& bytes);
    QString getPara(const QString& paraName){return map[paraName];}
    void addPara(const QString& paraName,const QString& paraValue);
    void removePara(const QString& paraName);

    int count()const{return map.count();}
    QList<QString> paras(){return map.keys();}

public:
    void genarate();
    const QByteArray& getRwaData()const{return rawData;}
    void load(QByteArray& bytes);
    void clear();
};

#endif // ZTPPROTOCOL_H
