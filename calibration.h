#ifndef CALIBRATION_H
#define CALIBRATION_H
#include <QObject>
#include <QTimer>
#include <QDateTime>
#include "pollthread.h"

class Calibration : public QObject
{
    Q_OBJECT

public:
    Calibration(int seconds);

    bool success() const { return _success; }
    qreal xadjust() const { return _xadjust; }
    qreal yadjust() const { return _yadjust; }

private:
    PollThread* _pollThread;
    QTimer* _timer;
    int _seconds;
    quint32 _sampleRate;
    QDateTime _startTime;
    qreal _minx, _maxx, _miny, _maxy;
    qreal _xadjust, _yadjust;
    bool _success;

private slots:
    void handleRawValues(qreal mx, qreal my, qreal mz);
    void handleTimerExpired();
};

#endif // CALIBRATION_H
