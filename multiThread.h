#pragma once
#ifndef MULTITHREAD_H
#define MULTITHREAD_H
#include <QThread>
#include <QMutex>

class multiThread :
    public QThread
{
    Q_OBJECT
public:
    multiThread();
    ~multiThread();

public slots:
    void stopThread();
    void restartThread();
    void setQueryString(QString str);
    static std::time_t get_timestamp();

protected:
    virtual void run() override;

signals:
    void traceItemUpdate();

private:
    bool is_stop;
    QMutex m_lock;
    //    TraceItemObject * traceItem;
    std::string m_last_realtime_id = "-1";
    std::string m_from_id = "-1";
    time_t from_t = 0;
    time_t to_t = 0;
    std::string to_id = "-1";
    QString query_string;

    void internalUpdateMongoData();
    void bindMongoDataToTraceTree();
};

#endif // MULTITHREAD_H