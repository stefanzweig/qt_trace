#pragma once
#ifndef MULTITHREAD_H
#define MULTITHREAD_H
#include <QThread>
#include <QMutex>
#include "topicData/CanMessageDataWorkerSubscriber.h"

class multiThread :
    public QThread
{
    Q_OBJECT
public:
    multiThread();
    ~multiThread();
    void setSubscriber(CanMessageDataWorkerSubscriber* subscriber);

public slots:
    void stopThread();
    void restartThread();
    void pauseThread();
    void setQueryString(QString str);
    static std::time_t get_timestamp();

protected:
    virtual void run() override;

signals:
    void traceItemUpdate();

private:
    bool is_stop;
    bool is_paused;
    QMutex m_lock;
    //    TraceItemObject * traceItem;
    std::string m_last_realtime_id = "-1";
    std::string m_from_id = "-1";
    time_t from_t = 0;
    time_t to_t = 0;
    std::string to_id = "-1";
    QString query_string;
    CanMessageDataWorkerSubscriber* mysub_ = nullptr;

    void internalUpdateMongoData();
    void bindMongoDataToTraceTree();
};

#endif // MULTITHREAD_H