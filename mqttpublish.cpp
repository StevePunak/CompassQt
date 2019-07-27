#include "mqttpublish.h"
#include "klog.h"
#include <QByteArray>
#include <QDataStream>
#include <QDebug>

MqttPublish::MqttPublish(const QString& host, quint16 port, const QString& topic, QObject *parent) :
  QObject(parent),
  _client(nullptr),
  _host(host), _port(port), _topic(topic),
  _connected(false)
{
  _client = new QMqttClient();
  _client->setHostname(_host);
  _client->setPort(port);
  connect(_client, &QMqttClient::connected, this, &MqttPublish::handleClientConnected);
  connect(_client, &QMqttClient::stateChanged, this, &MqttPublish::handleStateChanged);
  connect(_client, &QMqttClient::errorChanged, this, &MqttPublish::handleErrorChanged);
  _client->connectToHost();
}

void MqttPublish::handleClientConnected()
{
    KLog::sysLogText(KLOG_DEBUG, tr("Mqtt Client connected"));
    _connected = true;
}

void MqttPublish::handleStateChanged(QMqttClient::ClientState state)
{
    if(state == QMqttClient::ClientState::Disconnected)
    {
        _connected = false;
        KLog::sysLogText(KLOG_INFO, "Mqtt client disconnected... will reconnect");
        _client->connectToHost();
    }
}

void MqttPublish::handleErrorChanged(QMqttClient::ClientError error)
{
    KLog::sysLogText(KLOG_INFO, tr("Mqtt client error: ").arg(error));
}

void MqttPublish::handleBearing(qreal bearing)
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

