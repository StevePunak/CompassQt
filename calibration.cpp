#include "calibration.h"
#include <QTextStream>
#include <QtMath>
#include <QCoreApplication>
#include "klog.h"

Calibration::Calibration(int seconds) :
    QObject(),
    _pollThread(nullptr),
    _timer(nullptr),
    _seconds(seconds),
    _sampleRate(125),
    _startTime(QDateTime::currentDateTimeUtc()),
    _minx(100), _maxx(-100),
    _miny(100), _maxy(-100),
    _xadjust(0), _yadjust(0),
    _success(false)
{
    _pollThread = new PollThread(_sampleRate, 100, -100, 10, -100);
    connect(_pollThread, &PollThread::rawValues, this, &Calibration::handleRawValues);

    _pollThread->start();

    _timer = new QTimer();
    connect(_timer, &QTimer::timeout, this, &Calibration::handleTimerExpired);
    _timer->start(1000);

}

void Calibration::handleRawValues(qreal mx, qreal my)
{
    _minx = qMin(_minx, mx);
    _maxx = qMax(_maxx, mx);
    _miny = qMin(_miny, my);
    _maxy = qMax(_maxy, my);

    QTextStream standardOutput(stdout);
    standardOutput << ".";
}

void Calibration::handleTimerExpired()
{
    QTextStream standardOutput(stdout);
    if(QDateTime::currentDateTimeUtc() >= _startTime.addSecs(_seconds))
    {
        qreal xrange = (_maxx - _minx) / 2;
        _xadjust = -(_maxx - xrange);
        qreal yrange = (_maxy - _miny) / 2;
        _yadjust = -(_maxy - yrange);
        _success = true;

        QTextStream standardOutput(stdout);
        standardOutput << endl;

        QCoreApplication::quit();
    }
}
