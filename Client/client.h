#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QTcpSocket>

class Client : public QObject
{
    Q_OBJECT
public:
    Client();

    void connectToServer();

private slots:
    void onReadyRead();

private:
    bool validatePacket(const QByteArray &packet);
    uint8_t calcCRC(const QByteArray &data);

private:
    QTcpSocket* m_socket;
    QByteArray m_buffer;
};

#endif // CLIENT_H
