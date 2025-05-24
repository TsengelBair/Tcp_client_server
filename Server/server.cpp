#include <QTcpSocket>
#include <QDataStream>
#include <QTimer>

#include "server.h"
#include "../ITestMessage.pb.h"

Server::Server()
{
    if (this->listen(QHostAddress::LocalHost, 5000)){
        qDebug() << "Listen";
    } else {
        qDebug() << "Error: " << errorString();
    }
}

Server::~Server()
{
    this->close();
}

void Server::incomingConnection(qintptr socketDescriptor)
{
    QTcpSocket* socket = new QTcpSocket();
    socket->setSocketDescriptor(socketDescriptor);

    qDebug() << "Client connected " << socketDescriptor;

    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
    m_sockets.insert(socketDescriptor, socket);

    sendToClientTestMessage(socketDescriptor);
}

void Server::sendToClientTestMessage(qintptr socketDescriptor)
{
    /// тестовые данные, которые серилизуем
    QString testLogin = "mail@mail.ru";
    QString testPassword = "12345678";

    /// создаем объект сообщения, заполняя поля
    ITestMessage message;

    message.set_login(testLogin.toStdString());
    message.set_password(testPassword.toStdString());

    /// сериализуем
    std::string serializedData;
    if (!message.SerializeToString(&serializedData)) {
        qDebug() << "Error while serialize";
        return;
    }

    QByteArray data(serializedData.c_str(), serializedData.size());

    /// формируем пакет, пакет = заголовок (5 байт) + непосредственные данные (в данном случае serializedData)
    QByteArray packet = createPacketToSend(data);

    /// разбить пакет на две части, отправив первую и через qtimer вторую
    int mid = packet.size() / 2;
    QByteArray firstPart = packet.left(mid);
    QByteArray secondPart = packet.mid(mid);

    /// отправляем первую часть
    sendToClient(socketDescriptor, firstPart);
    qDebug() << "Sent first part of size" << firstPart.size() << "bytes";

    /// Отправляем вторую часть с задержкой через QTimer::singleShot
    QTimer::singleShot(2000, this, [this, socketDescriptor, secondPart]() {
        sendToClient(socketDescriptor, secondPart);
        qDebug() << "Sent second part of size" << secondPart.size() << "bytes";
    });
}

QByteArray Server::createPacketToSend(const QByteArray &data)
{
    QByteArray packet;

    quint32 packetSize = data.size();

    /// записываем в первые 4 байта размер непосредственных данных (без заголовка)
    QDataStream out(&packet, QIODevice::WriteOnly);
    out << quint32(packetSize);

    /// пятый байт контрольная сумма
    uint8_t crc = calcCRC(data);
    packet.append(static_cast<char>(crc));

    /// непосредственные данные
    packet.append(data);
    return packet;
}

uint8_t Server::calcCRC(const QByteArray &data)
{
    uint8_t crc = 0;
    for (const char c : data) {
        crc ^= static_cast<uint8_t>(c);
    }
    return crc;
}

void Server::sendToClient(qintptr socketDescriptor, const QByteArray &packet)
{
    QTcpSocket* socket = m_sockets.value(socketDescriptor);
    if (socket) {
        socket->write(packet);
        socket->flush();
    }
}
