#pragma once

#include "topicData/ZoneMasterDataPubSubTypes.h"

#include <chrono>
#include <thread>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include <QDebug>
#include <QThread>
#include <QtWidgets/QTreeView>
#include <QDateTime>
#include <QVector>
#include <QByteArray>
#include "..\zm_struct.h"

using namespace eprosima::fastdds::dds;

class SubListenerL : public QObject, public DataReaderListener
{
    Q_OBJECT
public:

    SubListenerL()
    {
    }

    ~SubListenerL()
    {
    }

    void on_subscription_matched(
        DataReader*,
        const SubscriptionMatchedStatus& info) override
    {
        if (info.current_count_change == 1)
        {
            qDebug() << "Subscriber matched.";
        }
        else if (info.current_count_change == -1)
        {
            qDebug() << "Subscriber unmatched.";
        }
        else
        {
            qDebug() << info.current_count_change;
        }
    }

    void on_data_available(
        DataReader* reader) override
    {
    }

    //canMessages can_messages_;
    QThread* outerThread = nullptr;
    QTreeView* tree_ = nullptr;

//signals:
//    void traceItemUpdate_internal_lin_frame(linFrame lf);
};