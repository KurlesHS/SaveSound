#include "savesoundmainwindow.h"
#include <QApplication>
#include <VoiceOverIp/ringiodevice.h>
#include <QDebug>
#include <QAudioDeviceInfo>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SaveSoundMainWindow w;
    w.show();
    RingIODevice *r = new RingIODevice(1000);
    QByteArray c(1000, 0xff);
    //r->writeAtPosition(QByteArray("!ello!"), 0);
    //r->writeAtPosition(" world!h", 6);

    //qDebug() << r->read(30);
    r->writeAtPosition(c, 0);
    r->addOrRemoveBytesFromCurrentPosition(100);
    QAudioDeviceInfo info = QAudioDeviceInfo::defaultOutputDevice();
    qDebug() << info.supportedSampleRates();
    return a.exec();
}
