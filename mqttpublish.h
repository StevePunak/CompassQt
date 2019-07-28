#ifndef MQTTPUBLISH_H
#define MQTTPUBLISH_H
#include <QObject>
#include <QString>
#include <QtMqtt/QtMqtt>
#include <QByteArray>
#include <QList>

class BrokerInfo : public QObject
{
    Q_OBJECT

public:
    static BrokerInfo* Create(const QString& parsable);

private:
    BrokerInfo(const QString& host, quint16 port, const QString& topic);

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

class MqttPublish : public QObject
{
    Q_OBJECT

public:
    MqttPublish(const QString& adresses, QObject *parent = nullptr);

private:
    QList<BrokerInfo*> _brokerList;

signals:
    void bearing(qreal degrees);

public slots:
    void handleBearing(qreal bearing);
};

#endif // MQTTPUBLISH_H
