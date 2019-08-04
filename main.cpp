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
    QString mqttBrokers = "myhost.sample.com:1883;my/bearing/topic";
    int verbosity = 0;
    qreal xadjust = 0;
    qreal yadjust = 0;
    qreal minx = 0;
    qreal maxx = 0;
    qreal miny = 0;
    qreal maxy = 0;
    bool calibrate = false;
    bool autoCalibrate = false;

    // default location
    QString confFile = "/etc/compass/compass.conf";
    QString logFile = "/tmp/compass.log";

    // keys for arguments / configuration file
    QString keyConfFile = "conf-file";
    QString keySampleRate = "sample-rate";
    QString keyMqttBrokers = "mqtt-brokers";
    QString keyLogFile = "log-file";
    QString keyVerbosity = "verbosity";
    QString keyXAdjust = "xadjust";
    QString keyYAdjust = "yadjust";
    QString keyMinX = "minx";
    QString keyMaxX = "maxx";
    QString keyMinY = "miny";
    QString keyMaxY = "maxy";
    QString keyCalibrate = "z";
    QString keyAutoCalibrate = "auto-calibrate";

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
                          {{ "b", keyMqttBrokers},
                              QCoreApplication::translate("main", "MQTT brokers to publish compass heading. This can be multiple brokers delimited by ';'."),
                              QCoreApplication::translate("main", "mqttBroker") },
                          {{ "o", keyLogFile},
                              QCoreApplication::translate("main", "Log file for output"),
                              QCoreApplication::translate("main", "logFile") },
                          {{  "x", keyXAdjust},
                              QCoreApplication::translate("main", "X adjust value from calibration"),
                              QCoreApplication::translate("main", "xadjust") },
                          {{  "y", keyYAdjust},
                              QCoreApplication::translate("main", "Y adjust value from calibration"),
                              QCoreApplication::translate("main", "yadjust") },
                          {{  keyMinX, keyMinX},
                              QCoreApplication::translate("main", "Minimum value observed on X axis"),
                              QCoreApplication::translate("main", "minx") },
                          {{  keyMaxX, keyMaxX},
                              QCoreApplication::translate("main", "Maximum value observed on X axis"),
                              QCoreApplication::translate("main", "maxx") },
                          {{  keyMinY, keyMinY},
                              QCoreApplication::translate("main", "Minimum value observed on Y axis"),
                              QCoreApplication::translate("main", "miny") },
                          {{  keyMaxY, keyMaxY},
                              QCoreApplication::translate("main", "Maximum value observed on Y axis"),
                              QCoreApplication::translate("main", "maxy") },
                          {{  "a", keyAutoCalibrate},
                              QCoreApplication::translate("main", "Do auto-calibration"),
                              QCoreApplication::translate("main", "autoCalibrate") },
                          {   keyCalibrate,
                              QCoreApplication::translate("main", "calibrate") },
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
        QVariant v = settings.value("Main/" + keyMqttBrokers);
        sampleRate = settings.value("Main/" + keySampleRate).toUInt();
        mqttBrokers = settings.value("Main/" + keyMqttBrokers).toString();
        logFile = settings.value("Main/" + keyLogFile).toString();
        verbosity = settings.value("Main/" + keyVerbosity, 0).toInt();
        xadjust = settings.value("Main/" + keyXAdjust, 0).toFloat();
        yadjust = settings.value("Main/" + keyYAdjust, 0).toFloat();
        minx = settings.value("Main/" + keyMinX, 0).toFloat();
        maxx = settings.value("Main/" + keyMaxX, 0).toFloat();
        miny = settings.value("Main/" + keyMinY, 0).toFloat();
        maxy = settings.value("Main/" + keyMaxY, 0).toFloat();
        autoCalibrate = settings.value("Main/" + keyAutoCalibrate, 0).toInt();
    }

    if(parser.isSet(keySampleRate))
        sampleRate = parser.value(keySampleRate).toUInt();

    // get MQTT broker address
    if(parser.isSet(keyMqttBrokers))
        mqttBrokers = parser.value(keyMqttBrokers);

    // get x/y-adjust
    if(parser.isSet(keyXAdjust))
        xadjust = parser.value(keyXAdjust).toFloat();
    if(parser.isSet(keyYAdjust))
        yadjust = parser.value(keyYAdjust).toFloat();

    // calibrate?
    if(parser.isSet(keyCalibrate))
        calibrate = true;

    // calibrate?
    if(parser.isSet(keyAutoCalibrate))
        autoCalibrate = true;

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
    standardOutput << QString("Opening compass... with xadjust=%1 yadjust=%2")
                      .arg(QString().number(xadjust, 'g', 10))
                      .arg(QString().number(yadjust, 'g', 10))
                      >> endl;

    if(true)
    {
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

        MonitorThread* monitor = new MonitorThread(mqttBrokers, sampleRate, xadjust, yadjust, minx, maxx, miny, maxy, autoCalibrate);

        result = coreApplication.exec();

        monitor->stop();

        KLog::sysLogText(KLOG_INFO, QString("-----------------------------------------------------------------------------------"));
        KLog::sysLogText(KLOG_INFO, QString("Compass daemon stopped at %1").
                         arg(QDateTime::currentDateTimeUtc().toString()));
        KLog::sysLogText(KLOG_INFO, QString("-----------------------------------------------------------------------------------\n\n\n"));
    }
#if STATIC_CALIBRATION
    else
    {
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
            standardOutput << QString("mqtt-brokers=%1").arg(mqttBrokers) << endl;
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
    }
#endif
    return result;
}
