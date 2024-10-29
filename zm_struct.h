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

#endif // ZMSTRUCT_H
