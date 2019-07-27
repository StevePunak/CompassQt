#include "monitorthread.h"
#include "klog.h"

MonitorThread::MonitorThread(
        const QString& brokerAddress,
        quint16 brokerPort,
        const QString& publishTopic,
        quint32 sampleRate,
        qreal minx, qreal maxx, qreal miny, qreal maxy) :
    _pollThread(nullptr),
    _publisher(nullptr),
    _brokerAddress(brokerAddress),
    _brokerPort(brokerPort),
    _publishTopic(publishTopic),
    _sampleRate(sampleRate),
    _minx(minx), _maxx(maxx), _miny(miny), _maxy(maxy)
{
    _thread.setObjectName("Monitor Thread");
    connect(&_thread, &QThread::started, this, &MonitorThread::handleThreadStarted);
    moveToThread(&_thread);
    _thread.start();
}

void MonitorThread::start()
{
    _thread.start();
}

void MonitorThread::stop()
{
    _thread.quit();
}

void MonitorThread::handleThreadStarted()
{
    _pollThread = new PollThread(_sampleRate, _minx, _maxx, _miny, _maxy);
    _publisher = new MqttPublish(_brokerAddress, _brokerPort, _publishTopic);

    // connect the device poller to the mqtt publisher
    connect(_pollThread, &PollThread::bearing, _publisher, &MqttPublish::handleBearing);

    _pollThread->start();

    _timer = new QTimer();
    connect(_timer, &QTimer::timeout, this, &MonitorThread::handleTimerExpired);

    _timer->start(1000);
}

void MonitorThread::handleThreadStopped()
{

}

void MonitorThread::handleTimerExpired()
{
}

