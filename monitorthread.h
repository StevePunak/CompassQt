#ifndef MONITORTHREAD_H
#define MONITORTHREAD_H
#include <QObject>
#include <QThread>
#include <QTimer>
#include <QString>
#include "pollthread.h"
#include "mqttpublish.h"

class MonitorThread : public QObject
{
    Q_OBJECT

public:
    MonitorThread(
            const QString& brokerAddress,
            quint16 brokerPort,
            const QString& publishTopic,
            quint32 sampleRate,
            qreal minx, qreal maxx, qreal miny, qreal maxy);
    void start();
    void stop();

private:
    PollThread* _pollThread;
    MqttPublish* _publisher;
    QTimer* _timer;
    QThread _thread;

    QString _brokerAddress;
    quint16 _brokerPort;
    QString _publishTopic;
    quint32 _sampleRate;
    qreal _minx, _maxx, _miny, _maxy;

private slots:
    void handleThreadStarted();
    void handleThreadStopped();
    void handleTimerExpired();
};

#endif // MONITORTHREAD_H
