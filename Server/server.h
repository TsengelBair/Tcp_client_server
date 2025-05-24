#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>

class Server : public QTcpServer
{
    Q_OBJECT
public:
    Server();
    ~Server();

private slots:
    void incomingConnection(qintptr socketDescriptor) override;

private:
    void sendToClientTestMessage(qintptr socketDescriptor);
    QByteArray createPacketToSend(const QByteArray &data);
    uint8_t calcCRC(const QByteArray &data);
    void sendToClient(qintptr socketDescriptor, const QByteArray &packet);

private:
    QMap<qintptr, QTcpSocket*> m_sockets;
};

#endif // SERVER_H
