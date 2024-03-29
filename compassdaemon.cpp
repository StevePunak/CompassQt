#include <QCoreApplication>
#include <QDebug>
#include <QDateTime>

// for signal handlers
#include <initializer_list>
#include <signal.h>
#include <unistd.h>
#include "compassdaemon.h"
#include <sys/types.h>
#include <sys/socket.h>
#include "klog.h"

int CompassDaemon::sighupFd[2];
int CompassDaemon::sigtermFd[2];

static int setup_unix_signal_handlers()
{
    struct sigaction hup, term;

    hup.sa_handler = CompassDaemon::hupSignalHandler;
    sigemptyset(&hup.sa_mask);
    hup.sa_flags = 0;
    hup.sa_flags |= SA_RESTART;

    if (sigaction(SIGHUP, &hup, 0))
       return 1;

    term.sa_handler = CompassDaemon::termSignalHandler;
    sigemptyset(&term.sa_mask);
    term.sa_flags |= SA_RESTART;

    if (sigaction(SIGTERM, &term, 0))
       return 2;

    return 0;
}

CompassDaemon::CompassDaemon(QObject *parent)
        : QObject(parent),
          _monitorThread(nullptr)
{
    setup_unix_signal_handlers();

    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sighupFd))
       qFatal("Couldn't create HUP socketpair");

    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sigtermFd))
       qFatal("Couldn't create TERM socketpair");
    snHup = new QSocketNotifier(sighupFd[1], QSocketNotifier::Read, this);
    connect(snHup, SIGNAL(activated(int)), this, SLOT(handleSigHup()));
    snTerm = new QSocketNotifier(sigtermFd[1], QSocketNotifier::Read, this);
    connect(snTerm, SIGNAL(activated(int)), this, SLOT(handleSigTerm()));
}

CompassDaemon::~CompassDaemon(){}

void CompassDaemon::hupSignalHandler(int)
{
    char a = 1;
    ::write(sighupFd[0], &a, sizeof(a));
}

void CompassDaemon::termSignalHandler(int)
{
    char a = 1;
    ::write(sigtermFd[0], &a, sizeof(a));
}

void CompassDaemon::handleSigHup()
{
    snHup->setEnabled(false);
     char tmp;
     ::read(sighupFd[1], &tmp, sizeof(tmp));

     snHup->setEnabled(true);
}

void CompassDaemon::handleSigTerm()
{
    snTerm->setEnabled(false);
     char tmp;
     ::read(sigtermFd[1], &tmp, sizeof(tmp));

    if(_monitorThread != nullptr)
        _monitorThread->stop();
     QCoreApplication::quit();
     snTerm->setEnabled(true);
}
