#ifndef RINGIODEVICE_H
#define RINGIODEVICE_H

#include <QIODevice>

class RingIoDevicePrivate;

class RingIODevice : public QIODevice
{
    Q_OBJECT
public:
    explicit RingIODevice(const int &bufferSize, QObject *parent = 0);
    virtual ~RingIODevice();
    qint64 size() const;
    QByteArray readByteArray(int size);
    void writeAtPosition(const QByteArray &data, const int position);
    void addOrRemoveBytesFromEnd(int bytesCount, const int bytesInWindows);
    void addOrRemoveBytesFromCurrentPosition(int bytesCount);
    void init();

protected:
    qint64 readData(char *data, qint64 maxSize) Q_DECL_OVERRIDE;
    qint64 writeData(const char *data, qint64 maxSize) Q_DECL_OVERRIDE;

signals:

public slots:

private:
    RingIoDevicePrivate *d;

};

#endif // RINGIODEVICE_H
