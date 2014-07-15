#ifndef VOICEHANDLER_P_H
#define VOICEHANDLER_P_H

#include <QObject>
#include <QHostAddress>
#include <QByteArray>
#include <QTime>
#include <QMutex>


class QFile;
class QAudioInput;
class QThread;
class QTemporaryFile;

class VoiceHandlerPrivate : public QObject
{
    Q_OBJECT

public:
    VoiceHandlerPrivate();
    virtual ~VoiceHandlerPrivate();

    void setLenghtAudioLogInMinutes(const int minutes);
    int getLenghtAudioLogInMinutes();

    void setAudioLogPath(const QString &path);
    QString getAudioLogPath();

    void setDeviceName(const QString &deviceName);
    void setSampleRate(const int sampleRate);
    void setSampleSize(const int sampleSize);
    void setMp3LameBitratePrefix(const QString &lamePrefix);
    void setMp3Bitrate(const int mp3Bitrate);


public Q_SLOTS:
    void start();
    void stop();

signals:
    void log(QString text);
    void outgoingData(QByteArray data, int senderId);
    void stateChanged(int state);
    void logFileUpdated(const QString &filename);

private:
    bool startInputAudio();
    void stopInputAudio();
    void createNewAudioFile();
    QByteArray getBytesFromInt(int number, const int numBytes);
    quint32 getIntFromBytes(const QByteArray &bytes);
    void saveAudioLogFileHelper(bool wait);

private Q_SLOTS:
    void onAudioInputNotify();

public Q_SLOTS:
    void saveAudioLogFile();
    void saveAudioLogFile(QFile *input, QFile *output = nullptr);


private:
    QIODevice *m_audioInputIODevice;
    QAudioInput *m_audioInput;
    QTime time;
    double m_sampleDurationInMs;
    int m_lenghtAudioLogInMinutes;
    QString m_pathToAudioLog;
    QTemporaryFile *m_incomingAudioDataFile;
    QDateTime m_createAudiofileTime;
    int m_currentState;
    QTime m_timeForWaitFile;
    QString m_deviceName;
    QString m_lamePrefix;
    int m_mp3Bitrate;
    int m_sampleRate;
    int m_sampleSize;


public:
    QThread *m_thread;
    QMutex m_mutex;

};

#endif // VOICEHANDLER_P_H
