#ifndef COMPASSDAEMON_H
#define COMPASSDAEMON_H
#include <QObject>
#include <QSocketNotifier>
#include "monitorthread.h"

class CompassDaemon : public QObject
{
    Q_OBJECT

public:
    CompassDaemon(QObject *parent = 0);
    ~CompassDaemon();

    // Unix signal handlers.
    static void hupSignalHandler(int unused);
    static void termSignalHandler(int unused);

    void setMonitorThread(MonitorThread* monitorThread);

public slots:
    // Qt signal handlers.
    void handleSigHup();
    void handleSigTerm();

private:
    static int sighupFd[2];
    static int sigtermFd[2];

    QSocketNotifier *snHup;
    QSocketNotifier *snTerm;

    MonitorThread* _monitorThread;
};

#endif // COMPASSDAEMON_H
