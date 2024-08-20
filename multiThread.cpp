#include "multiThread.h"
#include <QDebug>

multiThread::multiThread()
    :is_stop(false),is_paused(false)
{
}

multiThread::~multiThread() {}

void multiThread::stopThread() {
    is_stop = true;
    is_paused = false;

    if (mysub_can_frames != nullptr) {
        delete mysub_can_frames;
        mysub_can_frames = nullptr;
    }
    if (mysub_can_parser != nullptr) {
        delete mysub_can_parser;
        mysub_can_parser = nullptr;
    }
}

void multiThread::stopFlag()
{
    is_stop = true;
    is_paused = false;
}

void multiThread::pauseThread() {
    if (!is_stop) {
        is_paused = true;
    }
}

void multiThread::restartThread() {
    is_stop = false;
    is_paused = false;
}

void multiThread::run() {
    // key function
    
    // from_t = get_timestamp();
    if (mysub_can_frames != nullptr) {
        qDebug() << "mysub is not nullptr";
            if (mysub_can_frames->init()) {
                qDebug() << "mysub before run";
                mysub_can_frames->run(samples_);
                qDebug() << "mysub after run";
                qDebug() << "debug: inited";
            }
    }


    if (mysub_can_parser != nullptr) {
        qDebug() << "mysubparser is not nullptr";
        if (mysub_can_parser->init()) {
            qDebug() << "mysubparser before run";
            mysub_can_parser->run(samples_);
            qDebug() << "mysubparser after run";
            qDebug() << "debug: mysubparser inited";
        }
    }

    while (true) {
        if (!is_stop) {
            qDebug() << "running...";
            msleep(1000); // every 1 second
        }
        else {
            break;
        }
    }
}

std::time_t multiThread::get_timestamp()
{
    std::chrono::time_point<std::chrono::system_clock,
        std::chrono::microseconds> tp =
        std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    std::time_t timestamp = tp.time_since_epoch().count() * 1000;
    return timestamp;
}

void multiThread::bindDataToTraceTree()
{

    try {
        m_from_id = to_id;
    }
    catch (...) {
        qDebug() << "mongodb problem from " << from_t << " to " << to_t;
    }

    try {
            m_from_id = to_id;

            m_lock.unlock();
            msleep(20);
    }
    catch (...) {
        qDebug() << "handling result problem.";
    }
}

void multiThread::setQueryString(QString str)
{
    this->query_string = str;
}

void multiThread::setSubscriber(ZoneMasterCanMessageDataSubscriber* subscriber, int samples, QTreeView* treeview)
{
    mysub_can_frames = subscriber;
    samples_ = samples;
    subscriber->setOuterThread(this, treeview);
}

void multiThread::setCanParserSubscriber(ZoneMasterCanParserSubscriber* subscriber, int samples, QTreeView* treeview)
{
    mysub_can_parser = subscriber;
    samples_ = samples;
    subscriber->setOuterThread(this, treeview);
}