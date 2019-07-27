#include <QCoreApplication>
#include <QString>
#include <QtGlobal>
#include <QCommandLineParser>
#include <QTextStream>
#include <QSettings>
#include <QFile>
#include <QDateTime>
#include "klog.h"
#include "monitorthread.h"
#include "compassdaemon.h"
#include "calibration.h"

int main(int argc, char *argv[])
{
    quint32 sampleRate = 125;
    quint16 mqttPort = 1883;
    QString publishTopic = "compass/topic";
    QString mqttBroker = "myhost.sample.com";
    int verbosity = 0;
    qreal minx = 100;
    qreal miny = 100;
    qreal maxx = -100;
    qreal maxy = -100;

    // default location
    QString confFile = "/etc/compass/compass.conf";
    QString logFile = "/tmp/compass.log";

    // keys for arguments / configuration file
    QString keyConfFile = "conf-file";
    QString keySampleRate = "sample-rate";
    QString keyMqttPort = "mqtt-port";
    QString keyMqttBroker = "mqtt-broker";
    QString keyPublishTopic = "mqtt-topic";
    QString keyLogFile = "log-file";
    QString keyVerbosity = "verbosity";
    QString keyMinX = "minx";
    QString keyMaxX = "maxx";
    QString keyMinY = "miny";
    QString keyMaxY = "maxy";

    // create the application
    QCoreApplication coreApplication(argc, argv);

    QTextStream standardOutput(stdout);

    // parse command line
    QCommandLineParser parser;
    parser.setApplicationDescription("RP Lidar Daemon");
    parser.addHelpOption();

    parser.addOptions({
                          {{ "r", keySampleRate},
                              QCoreApplication::translate("main", "Sample rate in milliseconds (default is 250)."),
                              QCoreApplication::translate("main", "sampleRate")},
                          {{ "v", keyVerbosity},
                              QCoreApplication::translate("main", "Logging output verbosity"),
                              QCoreApplication::translate("main", "verbosity")},
                          {{ "c", keyConfFile},
                              QCoreApplication::translate("main", "Configuration File"),
                              QCoreApplication::translate("main", "configurationFile") },
                          {{ "p", keyMqttPort},
                              QCoreApplication::translate("main", "Port to connect to broker (default 1883)."),
                              QCoreApplication::translate("main", "mqttPort") },
                          {{ "t", keyPublishTopic},
                              QCoreApplication::translate("main", "Topic on thich to publish compass heading."),
                              QCoreApplication::translate("main", "publishTopic") },
                          {{ "b", keyMqttBroker},
                              QCoreApplication::translate("main", "MQTT broker to publish compass heading."),
                              QCoreApplication::translate("main", "mqttBroker") },
                          {{ "o", keyLogFile},
                              QCoreApplication::translate("main", "Log file for output"),
                              QCoreApplication::translate("main", "logFile") },
                          {{  "minx", keyMinX},
                              QCoreApplication::translate("main", "Min X value from calibration"),
                              QCoreApplication::translate("main", "minx") },
                          {{  "maxx", keyMaxX},
                              QCoreApplication::translate("main", "Max X value from calibration"),
                              QCoreApplication::translate("main", "maxx") },
                          {{  "miny", keyMinY},
                              QCoreApplication::translate("main", "Min Y value from calibration"),
                              QCoreApplication::translate("main", "miny") },
                          {{  "maxy", keyMaxY},
                              QCoreApplication::translate("main", "Max Y value from calibration"),
                              QCoreApplication::translate("main", "maxy") },
    });
    parser.process(coreApplication);

    // configuration file location
    if(parser.isSet(keyConfFile))
    {
        confFile = parser.value(keyConfFile);
        if(QFile::exists(confFile) == false)
        {
            standardOutput << "Configuration file at " << confFile << " does not exist";
            exit(-1);
        }
    }

    // load the configuration file if it exists
    QSettings settings(confFile, QSettings::Format::IniFormat);
    if(QFile::exists(confFile))
    {
        sampleRate = settings.value("Main/" + keySampleRate).toUInt();
        mqttPort = settings.value("Main/" + keyMqttPort).toInt();
        mqttBroker = settings.value("Main/" + keyMqttBroker).toString();
        publishTopic = settings.value("Main/" + keyPublishTopic).toString();
        logFile = settings.value("Main/" + keyLogFile).toString();
        verbosity = settings.value("Main/" + keyVerbosity, 0).toInt();
        minx = settings.value("Main/" + keyMinX, 0).toFloat();
        maxx = settings.value("Main/" + keyMaxX, 0).toFloat();
        miny = settings.value("Main/" + keyMinY, 0).toFloat();
        maxy = settings.value("Main/" + keyMaxY, 0).toFloat();
    }

    if(parser.isSet(keySampleRate))
        sampleRate = parser.value(keySampleRate).toUInt();

    if(parser.isSet(keyMqttPort))
        mqttPort = parser.value(keyMqttPort).toUInt();

    // get MQTT topic name
    if(parser.isSet(keyPublishTopic))
        publishTopic = parser.value(keyPublishTopic);

    // get MQTT broker address
    if(parser.isSet(keyMqttBroker))
        mqttBroker = parser.value(keyMqttBroker);

    // get x/y min/max
    if(parser.isSet(keyMinX))
        minx = parser.value(keyMinX).toFloat();
    if(parser.isSet(keyMaxX))
        maxx = parser.value(keyMaxX).toFloat();
    if(parser.isSet(keyMinY))
        miny = parser.value(keyMinY).toFloat();
    if(parser.isSet(keyMaxY))
        maxy = parser.value(keyMaxY).toFloat();

    // Verbosity
    if(parser.isSet(keyVerbosity))
    {
        bool success;
        verbosity = parser.value(keyVerbosity).toInt(&success);
        if(success == false || verbosity < 0 || verbosity > 10)
        {
            qDebug() << "Illegal verbosity level. Must be between 0 and 10";
            exit(-1);
        }
    }

    int result = 0;
    standardOutput << QString("Opening compass... witch minx=%1 maxx=%2 miny=%3 maxy=%4")
                      .arg(QString().number(minx, 'g', 10))
                      .arg(QString().number(maxx, 'g', 10))
                      .arg(QString().number(miny, 'g', 10))
                      .arg(QString().number(maxy, 'g', 10))
                      >> endl;

    KLog::setSystemLogFile(logFile);
    KLog::setSystemVerbosity(verbosity);
    //KLog::setSystemOutputFlags((KLog::OutputFlags)(KLog::systemOutputFlags() & ~KLog::OutputFlags::Console));
    KLog::sysLogText(KLOG_INFO, "");
    KLog::sysLogText(KLOG_INFO, "");
    KLog::sysLogText(KLOG_INFO, "");
    KLog::sysLogText(KLOG_INFO, "");
    KLog::sysLogText(KLOG_INFO, QString("-----------------------------------------------------------------------------------"));
    KLog::sysLogText(KLOG_INFO, QString("Compass daemon started at %1. Log verbosity %2").
                     arg(QDateTime::currentDateTimeUtc().toString()).
                     arg(KLog::systemVerbosity()));
    KLog::sysLogText(KLOG_INFO, QString("-----------------------------------------------------------------------------------"));

    MonitorThread* monitor = new MonitorThread(mqttBroker, mqttPort, publishTopic, sampleRate, minx, maxx, miny, maxy);

    result = coreApplication.exec();

    monitor->stop();

    KLog::sysLogText(KLOG_INFO, QString("-----------------------------------------------------------------------------------"));
    KLog::sysLogText(KLOG_INFO, QString("Compass daemon stopped at %1").
                     arg(QDateTime::currentDateTimeUtc().toString()));
    KLog::sysLogText(KLOG_INFO, QString("-----------------------------------------------------------------------------------\n\n\n"));

#ifdef DO_STATIC_CALIBRATION
        // begin 20 second calibration cycle
        Calibration calibration(20);

        // let the calibration run... it will call quit when done and allow exec to drop through
        coreApplication.exec();
        if(calibration.success())
        {
            standardOutput << QString("Success!") << endl;
            standardOutput << QString("Adjust or replace %1 as follows:").arg(confFile) << endl;
            standardOutput << QString("[Main]") << endl;
            standardOutput << QString("sample-rate=%1").arg(sampleRate) << endl;
            standardOutput << QString("mqtt-port=%1").arg(mqttPort) << endl;
            standardOutput << QString("mqtt-broker=%1").arg(mqttBroker) << endl;
            standardOutput << QString("mqtt-topic=%1").arg(publishTopic) << endl;
            standardOutput << QString("log-file=%1").arg(logFile) << endl;
            standardOutput << QString("verbosity=%1").arg(verbosity) << endl;
            standardOutput << QString("xadjust=%1").arg(QString::number(calibration.xadjust(), 'g', 10)) << endl;
            standardOutput << QString("yadjust=%1").arg(QString::number(calibration.yadjust(), 'g', 10)) << endl;
            standardOutput << endl;
            result = 0;
        }
        else
        {
            standardOutput << "Calibration failed" << endl;
            result = 1;
        }
#endif
    return result;
}
