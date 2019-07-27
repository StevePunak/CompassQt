#ifndef MQTTPUBLISH_H
#define MQTTPUBLISH_H
#include <QObject>
#include <QString>
#include <QtMqtt/QtMqtt>
#include <QByteArray>

class MqttPublish : public QObject
{
    Q_OBJECT

public:
    MqttPublish(const QString& host, quint16 port, const QString& topic, QObject *parent = nullptr);

private:
    QMqttClient* _client;
    QString _host;
    quint16 _port;
    QString _topic;
    bool _connected;

public slots:
    void handleClientConnected();
    void handleStateChanged(QMqttClient::ClientState state);
    void handleErrorChanged(QMqttClient::ClientError error);
    void handleBearing(qreal bearing);
};

#endif // MQTTPUBLISH_H
