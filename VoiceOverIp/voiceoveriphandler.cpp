#include "voiceoveriphandler.h"
#include "VoiceOverIp/voicehandler_p.h"
#include "ringiodevice.h"
#include <QUdpSocket>
#include <QAudioInput>
#include <QAudioOutput>
#include <QDebug>
#include <QDateTime>
#include <QThread>
#include <QMutexLocker>
#include <QTimer>
#include <QTemporaryFile>
#include <QtConcurrent/QtConcurrent>
#include <QDir>
#include <QFileSystemWatcher>
#include <QProcess>
#include <QFileInfo>


VoiceOverIpHandler::VoiceOverIpHandler(QObject *parent) :
    QObject(parent)
{
    d = new VoiceHandlerPrivate();
    d->m_thread = new QThread();
    d->m_thread->start();
    d->moveToThread(d->m_thread);
    connect(d, SIGNAL(log(QString)), this, SIGNAL(log(QString)));
    connect(d, SIGNAL(outgoingData(QByteArray,int)), this, SIGNAL(outgoingData(QByteArray,int)));
    connect(d, SIGNAL(logFileUpdated(QString)), this, SIGNAL(logFileUpdated(QString)));
}

/**********************************************************************************/

VoiceOverIpHandler::~VoiceOverIpHandler()
{
    d->m_thread->quit();
    if (!d->m_thread->wait(0x2000)) {
        d->m_thread->terminate();
    }
    delete d->m_thread;
    delete d;
}

void VoiceOverIpHandler::setLenghtAudioLogInMinutes(const int minutes)
{
    d->setLenghtAudioLogInMinutes(minutes);
}

void VoiceOverIpHandler::setAudioLogPath(const QString &path)
{
    d->setAudioLogPath(path);
}

void VoiceOverIpHandler::setDeviceName(const QString &deviceName)
{
    d->setDeviceName(deviceName);
}

void VoiceOverIpHandler::setSampleRate(const int sampleRate)
{
    d->setSampleRate(sampleRate);
}

void VoiceOverIpHandler::setSampleSize(const int sampleSize)
{
    d->setSampleSize(sampleSize);
}

void VoiceOverIpHandler::setMp3LameBitratePrefix(const QString &lamePrefix)
{
    d->setMp3LameBitratePrefix(lamePrefix);
}

void VoiceOverIpHandler::setMp3Bitrate(const int mp3Bitrate)
{
    d->setMp3Bitrate(mp3Bitrate);
}

void VoiceOverIpHandler::start()
{
    QTimer::singleShot(1000, d, SLOT(start()));
    //QMetaObject::invokeMethod(d, "start", Qt::QueuedConnection);
    //d->start();
}

void VoiceOverIpHandler::stop()
{
    QMetaObject::invokeMethod(d, "stop", Qt::QueuedConnection);
    //d->stop();
}

/***************************************************************************************/

VoiceHandlerPrivate::VoiceHandlerPrivate() :
    QObject(nullptr),
    m_audioInputIODevice(nullptr),
    m_audioInput(nullptr),
    m_sampleDurationInMs(0.0453514739229025),
    m_incomingAudioDataFile(nullptr)
{
    time.start();
    m_timeForWaitFile.start();

}

VoiceHandlerPrivate::~VoiceHandlerPrivate()
{
    qDebug() << Q_FUNC_INFO;
    stop();
    qDebug() << Q_FUNC_INFO << "inter saveAudioLogFileHelper";
    saveAudioLogFileHelper(true);
    qDebug() << Q_FUNC_INFO << "exit saveAudioLogFileHelper";
}

void VoiceHandlerPrivate::start()
{
    if (m_incomingAudioDataFile) {
        auto x = m_incomingAudioDataFile;
        m_incomingAudioDataFile = nullptr;
        saveAudioLogFile(x);
    }
    createNewAudioFile();
    startInputAudio();
    time.restart();
    QTimer *t = new QTimer();
    t->start(m_lenghtAudioLogInMinutes * 60000);
    qDebug() << Q_FUNC_INFO << m_lenghtAudioLogInMinutes;
    connect(t, SIGNAL(timeout()), this, SLOT(saveAudioLogFile()));
    emit log(tr("Запуск udp сервера."));
}

void VoiceHandlerPrivate::stop()
{
    stopInputAudio();
}

void VoiceHandlerPrivate::setLenghtAudioLogInMinutes(const int minutes)
{
    QMutexLocker locker(&m_mutex);
    m_lenghtAudioLogInMinutes = minutes;
}

int VoiceHandlerPrivate::getLenghtAudioLogInMinutes()
{
    QMutexLocker locker(&m_mutex);
    return m_lenghtAudioLogInMinutes;
}

void VoiceHandlerPrivate::setAudioLogPath(const QString &path)
{
    QMutexLocker locker(&m_mutex);
    m_pathToAudioLog = path;
}

QString VoiceHandlerPrivate::getAudioLogPath()
{
    QMutexLocker locker(&m_mutex);
    return m_pathToAudioLog;
}

void VoiceHandlerPrivate::setDeviceName(const QString &deviceName)
{
    m_deviceName = deviceName;
}

void VoiceHandlerPrivate::setSampleRate(const int sampleRate)
{
    m_sampleRate = sampleRate;
}

void VoiceHandlerPrivate::setSampleSize(const int sampleSize)
{
    m_sampleSize = sampleSize;
}

void VoiceHandlerPrivate::setMp3LameBitratePrefix(const QString &lamePrefix)
{
    m_lamePrefix = lamePrefix;
}

void VoiceHandlerPrivate::setMp3Bitrate(const int mp3Bitrate)
{
    m_mp3Bitrate = mp3Bitrate;
}


bool VoiceHandlerPrivate::startInputAudio()
{
    stopInputAudio();
    QAudioFormat format;
    format.setSampleRate(m_sampleRate);
    format.setChannelCount(1);
    format.setSampleSize(m_sampleSize);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(m_sampleSize == 8 ?
                             QAudioFormat::UnSignedInt : QAudioFormat::SignedInt);
    QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();
    auto devices = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
    for (const QAudioDeviceInfo &dev : devices) {
        if (dev.deviceName() == m_deviceName) {
            info = dev;
            break;
        }
    }

    if (!info.isFormatSupported(format)) {
        return false;
    }
    m_audioInput = new QAudioInput(info, format);
    m_audioInput->setNotifyInterval(40);
    connect(m_audioInput, &QAudioInput::notify, this, &VoiceHandlerPrivate::onAudioInputNotify);
    m_audioInputIODevice = m_audioInput->start();
    return true;
}

void VoiceHandlerPrivate::stopInputAudio()
{
    if (!m_audioInput) return;
    m_audioInput->stop();
    delete m_audioInput;
    m_audioInput = nullptr;
    m_audioInputIODevice = nullptr;
}

void VoiceHandlerPrivate::saveAudioLogFile(QFile *input, QFile *output)
{
    qDebug() << Q_FUNC_INFO << QThread::currentThreadId();
    auto filename = m_createAudiofileTime.time().toString("hh_mm_ss_%1").arg(m_lenghtAudioLogInMinutes * 60);
    filename.append(".wav");
    auto dirname = m_createAudiofileTime.date().toString("dd_MM_yy");

    QDir dir(m_pathToAudioLog);
    dir = QDir(dir.absolutePath() + QDir::separator() + dirname);
    emit log(tr("save log file: %1").arg(filename));
    if (!dir.exists()) {
        dir.mkpath(dir.absolutePath());
    }

    int numOfChannel = output ? 2: 1;
    if (dir.exists()) {
        QFile f(dir.absolutePath() + QDir::separator() + filename);
        if (f.open(QIODevice::WriteOnly)) {

            int outputSize = output ? output->size() : 0;
            int inputSize = input ? input->size() : 0;

            int lenght = qMax(outputSize, inputSize);
            lenght *= numOfChannel;

            f.write("RIFF"); //chunkId - 4b
            f.write(getBytesFromInt(lenght + 36, 4));   //chunkSize - Это оставшийся размер цепочки,
                                                        //начиная с этой позиции. Иначе говоря,
                                                        //это размер файла - 8, то есть, исключены поля chunkId и chunkSize.
            f.write("WAVE"); //format 4b
            f.write("fmt "); //subchunk1Id 4b
            f.write(getBytesFromInt(16, 4));    //subchunk1Size 4b
                                                //16 для формата PCM. Это оставшийся размер подцепочки,
                                                //начиная с этой позиции.
            f.write(getBytesFromInt(1, 2));     //audioFormat 2b Аудио формат, полный список можно получить здесь. 2b
                                                //Для PCM = 1 (то есть, Линейное квантование).
                                                //Значения, отличающиеся от 1, обозначают некоторый формат сжатия.
            f.write(getBytesFromInt(numOfChannel, 2));     //numChannels 2b Количество каналов. 2b Моно = 1, Стерео = 2 и т.д.
            f.write(getBytesFromInt(m_sampleRate, 4)); //sampleRate 4b Частота дискретизации. 4b 8000 Гц, 44100 Гц и т.д.
            f.write(getBytesFromInt(m_sampleRate * (m_sampleSize / 8) * numOfChannel, 4)); //byteRate 4b Количество байт, переданных за секунду воспроизведения 4b
            f.write(getBytesFromInt(numOfChannel * (m_sampleSize / 8), 2));     //blockAlign 2b Количество байт для одного сэмпла, включая все каналы. 2b
            f.write(getBytesFromInt(m_sampleSize, 2));     //bitsPerSample 2b Количество бит в сэмпле. Так называемая "глубина" или
                                                //точность звучания. 8 бит, 16 бит и т.д.
            f.write("data");                    //subchunk2Id 4b
            f.write(getBytesFromInt(lenght, 4));//subchunk2Size 4b Количество байт в области данных.

            auto inputFileName = input ? input->fileName() : QString();
            auto outputFileName = output ? output->fileName() : QString();

            QFile inputFile(inputFileName);
            QFile outputFile(outputFileName);

            inputFile.open(QIODevice::ReadOnly);
            outputFile.open(QIODevice::ReadOnly);
            for (int i = 0; i < lenght / numOfChannel; ++i) {
                if (numOfChannel > 1){
                    char c1 = 0x80;
                    char c2 = 0x80;
                    auto a1 = inputFile.read(1);
                    auto a2 = outputFile.read(1);
                    if (a1.length() != 0) c1 = a1.at(0);
                    if (a2.length() != 0) c2 = a2.at(0);
                    f.write(&c1, 1);
                    f.write(&c2, 1);
                } else {
                    f.write(inputFile.readAll());
                }
            }
            f.close();
            QProcess p;
            QStringList args;
            auto fileToSave = f.fileName().replace(".wav", ".mp3");
            args << m_lamePrefix << QString::number(m_mp3Bitrate) << "-a" << f.fileName() << fileToSave;
            p.start("lame.exe", args);
            auto started = p.waitForStarted();
            p.waitForFinished();
            //qDebug() << p.readAllStandardError();
            if (p.exitCode() == 0 && started) {
                QFile::remove(f.fileName());
                emit logFileUpdated(fileToSave);
            }
        }
    }
    delete input;
    delete output;
}


void VoiceHandlerPrivate::saveAudioLogFile()
{
    saveAudioLogFileHelper(false);
}

void VoiceHandlerPrivate::saveAudioLogFileHelper(bool wait)
{
    qDebug() << Q_FUNC_INFO << QThread::currentThreadId();
    if (m_incomingAudioDataFile) {
        auto x = m_incomingAudioDataFile;
        m_incomingAudioDataFile = nullptr;
        x->close();
        QFuture<void> f = QtConcurrent::run<void>(this, &VoiceHandlerPrivate::saveAudioLogFile, x, nullptr);
        if (wait) {
            f.waitForFinished();
            return;
        }
        //saveAudioLogFile(x, y);
    }
    createNewAudioFile();
}

void VoiceHandlerPrivate::createNewAudioFile()
{
    m_incomingAudioDataFile = new QTemporaryFile();
    m_incomingAudioDataFile->open();
    m_createAudiofileTime = QDateTime::currentDateTime();
}

QByteArray VoiceHandlerPrivate::getBytesFromInt(int number, const int numBytes)
{
    QByteArray retArray;
    for (int i = 0; i < numBytes; ++i) {
        retArray.append((char)(number >> (i * 8)));
    }
    return retArray;
}

quint32 VoiceHandlerPrivate::getIntFromBytes(const QByteArray &bytes)
{
    quint32 retVal = 0;
    for (int x = 0; x < bytes.length(); ++x) {
        retVal |= (quint32)bytes.at(x) << (x * 8);
    }
    return retVal;
}

void VoiceHandlerPrivate::onAudioInputNotify()
{
    static int x = 0;
    ++x;
    x %= 2;
    if (m_incomingAudioDataFile && m_audioInputIODevice) {
        QByteArray audioData = m_audioInputIODevice->readAll();;
        m_incomingAudioDataFile->write(audioData);
        if (x == 0){
            emit outgoingData(audioData, 0);
            //qDebug() << audioData.length();
        }
    }
}
