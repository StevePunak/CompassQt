#include "monitorthread.h"
#include "klog.h"

MonitorThread::MonitorThread(const QString& brokerAddresses,
                             quint32 sampleRate,
                             qreal xadjust,
                             qreal yadjust,
                             qreal minx,
                             qreal maxx,
                             qreal miny,
                             qreal maxy,
                             bool autoCalibrate) :
    _pollThread(nullptr),
    _publisher(nullptr),
    _brokerAddresses(brokerAddresses),
    _sampleRate(sampleRate),
    _xadjust(xadjust), _yadjust(yadjust),
    _minx(minx), _maxx(maxx), _miny(miny), _maxy(maxy),
    _autoCalibrate(autoCalibrate)
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
    _pollThread = new PollThread(_sampleRate, _xadjust, _yadjust, _minx, _maxx, _miny, _maxy, _autoCalibrate);
    _publisher = new MqttPublish(_brokerAddresses);

    // connect the device poller to the mqtt publisher
    connect(_pollThread, &PollThread::bearing, _publisher, &MqttPublish::handleBearing);
    connect(_pollThread, &PollThread::rawValues, _publisher, &MqttPublish::handleRawValues);

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

