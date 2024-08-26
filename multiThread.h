#pragma once
#ifndef MULTITHREAD_H
#define MULTITHREAD_H
#include <QThread>
#include <QMutex>
#include "topicData/ZoneMasterCanMessageDataSubscriber.h"
#include "topicData/ZoneMasterCanMessageDataListener.h"
#include "topicData/ZoneMasterCanParserSubscriber.h"
#include "topicData/ZoneMasterCanParserListener.h"

#include <QtWidgets/QTreeView>
#include <QTreeWidgetItem>

class multiThread :
    public QThread, public QObject, public SubListener
{
    Q_OBJECT

public:
    multiThread();
    ~multiThread();
    void setSubscriber(ZoneMasterCanMessageDataSubscriber* subscriber, int samples, QTreeView* treeview);
    void setCanParserSubscriber(ZoneMasterCanParserSubscriber* subscriber, int samples, QTreeView* treeview);
    uint64_t full_count_canparser = 0;

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
    ZoneMasterCanParserSubscriber* mysub_can_parser = nullptr;
    QTreeView* tree_;
    QVector<canframe> full_canparserdata;
    QVector<can_frame> full_canframes;
    uint64_t last_timestamp_canparser = 0;
    uint64_t last_timestamp_canframe = 0;
    uint64_t full_count_canframes = 0;

    void bindDataToTraceTree();
    void formatRow_canparser_thread(canframe frame);
    void formatRow_canframe_thread(can_frame frame);

signals:
    void popToRoot(QTreeWidgetItem* item);
};

#endif // MULTITHREAD_H