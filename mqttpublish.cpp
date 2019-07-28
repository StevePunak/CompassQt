#include "mqttpublish.h"
#include "klog.h"
#include <QByteArray>
#include <QDataStream>
#include <QDebug>

MqttPublish::MqttPublish(const QString& addresses, QObject *parent) :
  QObject(parent)
{
    QStringList brokers = addresses.split("!");
    foreach(QString s, brokers)
    {
        BrokerInfo* broker =BrokerInfo::Create(s);
        if(broker != nullptr)
        {
            _brokerList.append(broker);
            connect(this, &MqttPublish::bearing, broker, &BrokerInfo::handleBearing);
        }
        else
        {
            KLog::sysLogText(KLOG_ERROR, tr("Could not parse broker '%1'").arg(s));
        }
    }
}

void MqttPublish::handleBearing(qreal degrees)
{
    emit bearing(degrees);
}

BrokerInfo *BrokerInfo::Create(const QString &parsable)
{
    BrokerInfo* broker = nullptr;
    QStringList parts = parsable.split(":");
    QString host, topic;
    quint16 port;

    if(parts.count() == 3)
    {
        host = parts[0];
        port = parts[1].toUInt();
        topic = parts[2];
        broker = new BrokerInfo(host, port, topic);
    }
    return broker;
}

BrokerInfo::BrokerInfo(const QString& host, quint16 port, const QString& topic) :
    _host(host),
    _port(port),
    _topic(topic),
    _connected(false)
{
    _client = new QMqttClient();
    _client->setHostname(host);
    _client->setPort(port);
    connect(_client, &QMqttClient::connected, this, &BrokerInfo::handleClientConnected);
    connect(_client, &QMqttClient::stateChanged, this, &BrokerInfo::handleStateChanged);
    connect(_client, &QMqttClient::errorChanged, this, &BrokerInfo::handleErrorChanged);
    _client->connectToHost();
}

void BrokerInfo::handleClientConnected()
{
    KLog::sysLogText(KLOG_DEBUG, tr("Mqtt Client connected to %1:%2 for topic %3")
                     .arg(_host)
                     .arg(_port)
                     .arg(_topic));
    _connected = true;
}

void BrokerInfo::handleStateChanged(QMqttClient::ClientState state)
{
    if(state == QMqttClient::ClientState::Disconnected)
    {
        _connected = false;
        KLog::sysLogText(KLOG_INFO, tr("Mqtt client disconnected from $1... will reconnect")
                         .arg(_host));
        _client->connectToHost();
    }
}

void BrokerInfo::handleErrorChanged(QMqttClient::ClientError error)
{
    KLog::sysLogText(KLOG_INFO, tr("Mqtt client error: ").arg(error));
}

void BrokerInfo::handleBearing(qreal bearing)
{
    if(_connected)
    {
        QByteArray serialized;
        QDataStream output(&serialized, QIODevice::WriteOnly);
        output.setByteOrder(QDataStream::ByteOrder::LittleEndian);
        output << bearing;
        _client->publish(_topic, serialized, 0, 1);
    }
}

