#include "multiThread.h"
#include <QDebug>

multiThread::multiThread()
    :is_stop(false)
{
}

multiThread::~multiThread() {}

void multiThread::stopThread() {
    is_stop = true;
}

void multiThread::restartThread() {
    is_stop = false;
}

void multiThread::run() {
    from_t = get_timestamp();
    while (true) {
        if (!is_stop) {
            internalUpdateMongoData();
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

void multiThread::internalUpdateMongoData()
{
    try {
        m_last_realtime_id = "";/*MongoAPI::getLastId_(REALTIME_COLLECTION);*/
    }
    catch (...) {
        qDebug() << "get last id error";
    }

    to_t = get_timestamp();
    if (from_t == to_t) {
    }
    else {
        bindMongoDataToTraceTree();
    }
}

void multiThread::bindMongoDataToTraceTree()
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