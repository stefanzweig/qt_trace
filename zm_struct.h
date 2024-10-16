#ifndef ZMSTRUCT_H
#define ZMSTRUCT_H

#include <cstdint>
#include <QVector>
#include <QString>

class BaseStruct {
public:
    virtual ~BaseStruct() {}
};

struct can_frame : public BaseStruct {
    uint64_t Timestamp;
    uint32_t Chn;                                                                 //通道
    uint32_t ID;                                               //ID
    QString Name;
    QString Dir;
    uint8_t DLC;                                                                          //DLC
    std::vector<uint8_t> Data;
    QString Data_Str;
    QString EventType;
    uint8_t DataLength;
    QString BusType;
    QVector<canpdu> pdus;                                                       //名称
};

struct lin_Frame : public BaseStruct {
    uint64_t Timestamp;
    uint32_t Chn;                                                                 //通道
    uint32_t ID;                                               //ID
};


/*

struct cansignal
{
    QString name;
    uint64_t raw_value;
    QString phy_value;
};

//CAN接口数据PDU结构体
struct canpdu
{
    QString name;
    uint32_t id;
    QVector<uint8_t> data;
    QVector<cansignal> csignals;
};

//CAN接口数据结构体
struct ethFrame : public BaseStruct
{
    uint16_t channle;
    QString dir;
    uint8_t sourceMACAddress[6];
    uint8_t destinationMACAddress[6];
    uint8_t sourceIPAddress[6];
    uint8_t destinationIPAddress[6];
    QString protocol;
    QVector<uint8_t> payLoad;
    uint16_t VLANID;
    QString TransportLayer;
    QString service;
    QString ServiceInsurence;
    QString Method;
    QString MessageType;
    QString ports;
    uint8_t VLANPriority;
};

struct linSignal
{
    QString name;
    uint64_t raw_value;
    QString phy_value;
    QString logical_value;
};

struct linFrame : public BaseStruct
{
    uint32_t channel;
    uint32_t id;
    QString Framename;
    QString dir;
    uint8_t length;
    QVector<uint8_t> data;
    QString EventType;
    QString BusType;
    QVector<linSignal> lsignals;
};
*/
#endif // ZMSTRUCT_H
