#ifndef VOICEOVERIPHANDLER_H
#define VOICEOVERIPHANDLER_H

#include <QObject>
#include <QHostAddress>

class VoiceHandlerPrivate;

class VoiceOverIpHandler : public QObject
{
    Q_OBJECT
public:
    enum States {
        waitState = 0x00,
        sendState = 0x01,
        receiveState = 0x02,
        disconnectState = 0x03
    };
    explicit VoiceOverIpHandler(QObject *parent = 0);
    virtual ~VoiceOverIpHandler();
   void setLenghtAudioLogInMinutes(const int minutes);
    void setAudioLogPath(const QString &path);
    void setDeviceName(const QString &deviceName);
    void setSampleRate(const int sampleRate);
    void setSampleSize(const int sampleSize);
    void setMp3LameBitratePrefix(const QString &lamePrefix);
    void setMp3Bitrate(const int mp3Bitrate);
    void start();
    void stop();

signals:
    void log(QString text);
    void outgoingData(QByteArray data, int senderId);
    void logFileUpdated(const QString &filename);

public slots:

private:
    VoiceHandlerPrivate *d;

};

#endif // VOICEOVERIPHANDLER_H
