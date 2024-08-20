#pragma once
#ifndef MULTITHREAD_H
#define MULTITHREAD_H
#include <QThread>
#include <QMutex>
#include "topicData/ZoneMasterCanMessageDataSubscriber.h"
#include "topicData/ZoneMasterCanMessageDataListener.h"

#include <QtWidgets/QTreeView>

class multiThread :
    public QThread, public SubListener
{
public:
    multiThread();
    ~multiThread();
    void setSubscriber(ZoneMasterCanMessageDataSubscriber* subscriber, int samples, QTreeView* treeview);

public slots:
    void stopThread();
    void stopFlag();
    void restartThread();
    void pauseThread();
    void setQueryString(QString str);
    static std::time_t get_timestamp();

protected:
    virtual void run() override;

private:
    bool is_stop;
    bool is_paused;
    QMutex m_lock;
    std::string m_last_realtime_id = "-1";
    std::string m_from_id = "-1";
    time_t from_t = 0;
    time_t to_t = 0;
    std::string to_id = "-1";
    QString query_string;
    int samples_ = 100;
    ZoneMasterCanMessageDataSubscriber* mysub_can_frames = nullptr;
    QTreeView* tree_;

    void bindDataToTraceTree();
};

#endif // MULTITHREAD_H