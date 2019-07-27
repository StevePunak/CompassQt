#ifndef POLLTHREAD_H
#define POLLTHREAD_H
#include <QObject>
#include <QThread>
#include <QtGlobal>
#include <QTimer>
#include "lsm9ds1.h"

class PollThread : public QObject
{
    Q_OBJECT

public:
    PollThread(quint32 sampleRate, qreal minx, qreal maxx, qreal miny, qreal maxy);

    void setSampleRate(quint32 value) { _sampleRate = value; }
    void start();
    void stop();

private:
    QThread _thread;
    QTimer* _timer;
    LSM9DS1* _compass;

    quint32 _sampleRate;
    qreal _bearing;
    qreal _xadjust, _yadjust;
    qreal _minx, _maxx, _miny, _maxy;

signals:
    void bearing(qreal bearing);
    void rawValues(qreal mx, qreal my);

private slots:
    void handleThreadStarted();
    void handleTimerExpired();
};

#endif // POLLTHREAD_H
