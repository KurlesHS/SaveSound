#include "ringiodevice.h"
#include "QDebug"
class RingIoDevicePrivate
{
public:
    RingIoDevicePrivate(const int bufferSize);
    ~RingIoDevicePrivate();
    int getChar();
    int correctPosition(const int delta);
    void removeChar(int pos);
    void insertChar(int pos, char ch);
    char readCharAtPos(const int pos);
    void writeCharAtPos(const char ch, const int pos);



public:
    QByteArray *buffer;
    int currentPosition;
};

RingIODevice::RingIODevice(const int &bufferSize, QObject *parent) :
    QIODevice (parent)
{
    d = new RingIoDevicePrivate(bufferSize);
    setOpenMode(QIODevice::ReadOnly);
}

RingIoDevicePrivate::RingIoDevicePrivate(const int bufferSize) :
    buffer(new QByteArray(bufferSize, char(0x80))),
    currentPosition(0)
{

}

RingIoDevicePrivate::~RingIoDevicePrivate()
{
    delete buffer;
}

RingIODevice::~RingIODevice()
{
    delete d;
}

qint64 RingIODevice::size() const
{
    return d->buffer->size();
}

QByteArray RingIODevice::readByteArray(int size)
{
    QByteArray ret;
    while (size != 0) {
        ret.append((char)d->getChar());
        --size;
    }
    return ret;
}

void RingIODevice::writeAtPosition(const QByteArray &data, const int position)
{
    auto realPosition = position % d->buffer->length();
    for (int i = 0; i < data.length(); ++i) {
        if (realPosition >= d->buffer->length()) realPosition = 0;
        d->buffer->operator [](realPosition++) = data.at(i);
    }
}

void RingIODevice::addOrRemoveBytesFromEnd(int bytesCount, const int bytesInWindows)
{
    if (bytesCount < 0 ) {
        auto absBytesCount = abs(bytesCount);
        if (absBytesCount > bytesInWindows) {
            bytesCount = 0 - bytesInWindows;
            absBytesCount = bytesInWindows;
        }
        int step = bytesInWindows / absBytesCount;
        auto tmpCurrentPostion = d->currentPosition;
        while (absBytesCount != 0) {
            tmpCurrentPostion -= step;
            d->removeChar(tmpCurrentPostion);
            --absBytesCount;
        }
    } else {
        double step = (double)bytesInWindows / (double)bytesCount;
        for (int x = 1; x <= bytesCount; ++x) {
            int pos = d->buffer->length() - (int)step *x;
            if (pos >= d->buffer->length())
                pos = d->buffer->length() - 1;
            d->insertChar(pos, 0x80);
        }
    }
}

void RingIODevice::addOrRemoveBytesFromCurrentPosition(int bytesCount)
{
    if (bytesCount == 0)
        return;
    if (bytesCount < 0) {
        bytesCount = abs(bytesCount) % d->buffer->length();
        int bytesNeedToMove = d->buffer->length() - bytesCount;
        if (bytesCount <= 0) return;
        int toPos = d->currentPosition;
        int fromPos = toPos + bytesCount;
        while (bytesNeedToMove != 0) {
            d->writeCharAtPos(d->readCharAtPos(fromPos), toPos);
            d->writeCharAtPos(0x80, fromPos);
            ++toPos;
            ++fromPos;
            --bytesNeedToMove;
        }
    } else {
        bytesCount = abs(bytesCount) % d->buffer->length();
        int bytesNeedToMove = d->buffer->length() - bytesCount;
        if (bytesCount <= 0) return;
        int fromPos = d->currentPosition + bytesNeedToMove;
        int toPos = fromPos + bytesCount;
        while (bytesNeedToMove != 0) {
            d->writeCharAtPos(d->readCharAtPos(fromPos), toPos);
            d->writeCharAtPos(0x80, fromPos);
            --toPos;
            --fromPos;
            --bytesNeedToMove;
        }
    }
}

void RingIODevice::init()
{
    d->currentPosition = 0x00;
    d->buffer->fill(0x80);
}


qint64 RingIODevice::readData(char *data, qint64 maxSize)
{
    qDebug() << Q_FUNC_INFO << d->currentPosition << maxSize;
    auto tmp = maxSize;
    while (tmp != 0) {
        *(data++) = (char)d->getChar();
        --tmp;
    }
    return maxSize;
}

qint64 RingIODevice::writeData(const char *data, qint64 maxSize)
{
    Q_UNUSED(data)
    Q_UNUSED(maxSize)
    return -1;
}

int RingIoDevicePrivate::getChar()
{
    if (currentPosition >= buffer->length())
        currentPosition = 0;
    int ret = (quint8)buffer->at(currentPosition);
    buffer->operator [](currentPosition++) = 0x80;
    return ret;
}

int RingIoDevicePrivate::correctPosition(const int delta)
{
    auto newPos = ((currentPosition + delta) % buffer->length());
    if (newPos < 0) newPos = 0;
    return newPos;
}

void RingIoDevicePrivate::removeChar(int pos)
{
    for (; pos < (buffer->length() - 1); ++pos) {
        buffer->operator [](pos) = buffer->at(pos + 1);
    }
    buffer->operator [](pos - 1) = 0x80;
    currentPosition = correctPosition(-1);
}

void RingIoDevicePrivate::insertChar(int pos, char ch)
{
    for (int i = buffer->length() - 1; i >= pos; -- i) {
        buffer->operator [](i) = buffer->at(i - 1);
    }
    buffer->operator [](pos) = ch;
    currentPosition = correctPosition(1);
}

char RingIoDevicePrivate::readCharAtPos(const int pos)
{
    auto realPos = pos % buffer->length();
    return buffer->at(realPos);
}

void RingIoDevicePrivate::writeCharAtPos(const char ch, const int pos)
{
    auto realPos = pos % buffer->length();
    buffer->operator [](realPos) = ch;
}
