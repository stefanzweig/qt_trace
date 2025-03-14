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
#include "../zm_struct.h"

using namespace eprosima::fastdds::dds;

class SomeipStateListener : public virtual QObject, public DataReaderListener
{
    Q_OBJECT
public:

    SomeipStateListener()
    {
    }

    ~SomeipStateListener()
    {
    }

    void on_subscription_matched(DataReader*,const SubscriptionMatchedStatus& info) override
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

    void on_data_available(DataReader* reader) override
    {
    SampleInfo info;
    if (reader->take_next_sample(&lin_messages_, &info) == ReturnCode_t::RETCODE_OK)
    {
        if (info.valid_data)
        {
        samples_++;
        for (int i = 0; i < lin_messages_.len(); i++) {
            linMessage msg = lin_messages_.linMsgs()[i];
            QString repr = QString::number(msg.id());
            qDebug() << "LIN ID -> " << repr;
            lin_Frame lf;
            lf.ID = msg.id();
            emit ItemUpdate_internal_someip_state(msg);
        }
        }
    }
    }

    linMessages lin_messages_;
    std::atomic_int samples_;
    QThread* outerThread = nullptr;
    QTreeView* tree_ = nullptr;

signals:
    void ItemUpdate_internal_someip_state(linMessage lm);

};
