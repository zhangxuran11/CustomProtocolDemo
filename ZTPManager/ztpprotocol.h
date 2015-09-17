#ifndef ZTPPROTOCOL_H
#define ZTPPROTOCOL_H
#include<QByteArray>
#include<QMap>
#include<QString>
class ZTPprotocol
{
    QMap<QString, QByteArray> map;
    QByteArray rawData;
public:
    enum EType{FILE};
    ZTPprotocol(){}
    explicit ZTPprotocol(QByteArray& bytes);
    QByteArray getPara(const QString& paraName,EType type){return type == FILE?map[paraName]:"";}
    QString getPara(const QString& paraName){return map[paraName];}
    void addPara(const QString& paraName,const QString& paraValue);
    void addPara(const QString& paraName,const QByteArray& paraValue,EType type);
    void removePara(const QString& paraName);

    int count()const{return map.count();}
    QList<QString> paras(){return map.keys();}

public:
    void genarate();
    QByteArray& getRwaData(){return rawData;}
    void load(QByteArray& bytes);
    void clear();
};

#endif // ZTPPROTOCOL_H
