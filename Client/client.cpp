#include <QDataStream>

#include "client.h"

Client::Client()
{
    m_socket = new QTcpSocket(this);
    connect(m_socket, &QTcpSocket::readyRead, this, &Client::onReadyRead);
}

void Client::connectToServer()
{
    m_socket->connectToHost("127.0.0.1", 5000);
}

void Client::onReadyRead()
{
    /// добавляем данные в буфер
    m_buffer.append(m_socket->readAll());

    /// обрабатываем пакет только в том случае, если у него есть хотя бы заголовок (5 байт)
    while (m_buffer.size() >= 5) {
        /// извлекаем указанную длину данных в первых 4 байтах пришедшего пакета
        quint32 dataSize;
        QDataStream ds(m_buffer.left(4));
        ds >> dataSize;

        /// Полный размер пакета = заголовок (5 байт) + dataSize (т.е. непосредственные данные)
        quint32 totalPacketSize = 5 + dataSize;

        /// данные пришли не полностью ожидаем весь пакет, предварительно сохранив то что пришло в буфер в начале метода
        if (m_buffer.size() < totalPacketSize) {
            qDebug() << "Пакет пришел неполностью";
            break;
        }

        /// данные пришли полностью, перед очисткой буфера сохраняем их
        QByteArray packet = m_buffer.left(totalPacketSize);
        m_buffer.remove(0, totalPacketSize);

        /// валидируем: сверяем размер и считаем контрольную сумму
        if (!validatePacket(packet)) {
            qDebug() << "Invalid packet!";
            continue;
        }

        qDebug() << "Пакет пришел полностью";
        /// возможно добавить десериализацию и отобразит в консоли
    }
}

bool Client::validatePacket(const QByteArray &packet)
{
    /// Извлекаем первые 5 байт (4 - размер, 1 - CRC)
    QByteArray header = packet.left(5);
    QDataStream ds(&header, QIODevice::ReadOnly);

    qint32 dataSizeFromPacket;
    qint8 crcFromPacket;

    ds >> dataSizeFromPacket >> crcFromPacket;

    /// Получаем данные (всё, что после 5 байт)
    QByteArray packetData = packet.mid(5);
    if (packetData.size() != dataSizeFromPacket) {
        qDebug() << "Указанный в заголовке размер данных не совпал с фактическим размером";
        return false;
    }

    /// Проверяем контрольную сумму
    uint8_t crc = calcCRC(packetData);
    if (static_cast<uint8_t>(crcFromPacket) != crc) {
        qDebug() << "CRC mismatch!";
        return false;
    }

    return true;
}

uint8_t Client::calcCRC(const QByteArray &data)
{
    uint8_t crc = 0;
    for (const char c : data) {
        crc ^= static_cast<uint8_t>(c);
    }
    return crc;
}
