#pragma once
#include <QObject>

class DataEmitter : public QObject {
    Q_OBJECT
public:
    explicit DataEmitter(QObject* parent = nullptr) : QObject(parent) {}

signals:
    void dataReceived(const int& data);
};

