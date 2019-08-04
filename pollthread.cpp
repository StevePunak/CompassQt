#include "pollthread.h"
#include <QtMath>
#include <QDebug>
#include "klog.h"

#define DEFAULT_SAMPLE_RATE     250

PollThread::PollThread(quint32 sampleRate,
                       qreal xadjust,
                       qreal yadjust,
                       qreal minx,
                       qreal maxx,
                       qreal miny,
                       qreal maxy,
                       bool autoCalibrate) :
    _timer(nullptr),
    _compass(nullptr),
    _sampleRate(sampleRate),
    _xadjust(xadjust), _yadjust(yadjust), _zadjust(0),
    _minx(minx), _maxx(maxx), _miny(miny), _maxy(maxy),
    _autoCalibrate(autoCalibrate),
    _staticCalibrate(false)
{
    if(_minx == 0)      _minx = 100;
    if(_maxx == 0)      _maxx = -100;
    if(_miny == 0)      _miny = 100;
    if(_maxy == 0)      _maxy = -100;

    if(_sampleRate == 0)
        _sampleRate = DEFAULT_SAMPLE_RATE;

    connect(&_thread, &QThread::started, this, &PollThread::handleThreadStarted);

    moveToThread(&_thread);
}

void PollThread::start()
{
    _thread.start();
}

void PollThread::stop()
{
    _thread.exit();
}

void PollThread::handleThreadStarted()
{
    _compass = new LSM9DS1();
    int result;
    result = _compass->begin();
    if(result == 0)
    {
        KLog::sysLogText(KLOG_ERROR, "Compass failed to initialize!");
    }

    if(_staticCalibrate)
    {
        KLog::sysLogText(KLOG_INFO, "Calibrating!");
        _compass->calibrateMag();
    }

    KLog::sysLogText(KLOG_DEBUG, "Poll thread started");
    _timer = new QTimer();
    connect(_timer, &QTimer::timeout, this, &PollThread::handleTimerExpired);
    _timer->start(_sampleRate);
}

void PollThread::handleTimerExpired()
{
    if(_compass->magAvailable())
    {
        _compass->readMag();
        qreal mx = _compass->calcMag(_compass->mx);
        qreal my = _compass->calcMag(_compass->my);
        qreal mz = _compass->calcMag(_compass->mz);

        // emit raw value signal
        rawValues(mx, my, mz);

        if(_autoCalibrate && (mx < _minx || my < _miny || mx > _maxx || my > _maxy))
        {
            double oldminx = _minx;
            double oldmaxx = _maxx;
            double oldminy = _miny;
            double oldmaxy = _maxy;
            _minx = qMin(_minx, mx);
            _maxx = qMax(_maxx, mx);
            _miny = qMin(_miny, my);
            _maxy = qMax(_maxy, my);

            qreal xrange = (_maxx - _minx) / 2;
            _xadjust = -(_maxx - xrange);
            qreal yrange = (_maxy - _miny) / 2;
            _yadjust = -(_maxy - yrange);

            if(_minx != oldminx)     KLog::sysLogText(KLOG_DEBUG, tr("Changing minx from %1 to %2").arg(oldminx).arg(_minx));
            if(_maxx != oldmaxx)     KLog::sysLogText(KLOG_DEBUG, tr("Changing maxx from %1 to %2").arg(oldmaxx).arg(_maxx));
            if(_miny != oldminy)     KLog::sysLogText(KLOG_DEBUG, tr("Changing miny from %1 to %2").arg(oldminy).arg(_miny));
            if(_maxy != oldmaxy)     KLog::sysLogText(KLOG_DEBUG, tr("Changing maxy from %1 to %2").arg(oldmaxy).arg(_maxy));
            KLog::sysLogText(KLOG_DEBUG, tr("minx=%1\n"
                                            "maxx=%2\n"
                                            "miny=%3\n"
                                            "maxy=%4\n"
                                            "xadjust=%5  yadjust=%6")
                             .arg(QString().number(_minx, 'g', 10))
                             .arg(QString().number(_maxx, 'g', 10))
                             .arg(QString().number(_miny, 'g', 10))
                             .arg(QString().number(_maxy, 'g', 10))
                             .arg(QString().number(_xadjust, 'g', 10))
                             .arg(QString().number(_yadjust, 'g', 10)));
            KLog::sysLogText(KLOG_DEBUG, tr("Recalibrated to new:\n"
                                            "minx=%1\n"
                                            "maxx=%2\n"
                                            "miny=%3\n"
                                            "maxy=%4\n"
                                            "xadjust=%5\n"
                                            "yadjust=%6")
                             .arg(QString().number(_minx, 'g', 10))
                             .arg(QString().number(_maxx, 'g', 10))
                             .arg(QString().number(_miny, 'g', 10))
                             .arg(QString().number(_maxy, 'g', 10))
                             .arg(QString().number(_xadjust, 'g', 10))
                             .arg(QString().number(_yadjust, 'g', 10)));
        }

        qreal adX = mx + _xadjust;
        qreal adY = my + _yadjust;
//        qreal adZ = mz + _zadjust;

        qreal angle = qAtan2(adY, adX);
        qreal degrees = (angle * 180) / M_PI;

        // emit degrees
        if(degrees < 0)
            degrees += 360;

//        KLog::sysLogText(KLOG_DEBUG, tr("bearing %1").arg(degrees));
        bearing(degrees);
    }
    else
    {
        KLog::sysLogText(KLOG_INFO, tr("no mag available"));
    }

    _timer->start(_sampleRate);
}

