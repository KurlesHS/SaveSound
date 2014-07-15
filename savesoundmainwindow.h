#ifndef NETRADIOSOUNDMAINWINDOW_H
#define NETRADIOSOUNDMAINWINDOW_H

#include <QMainWindow>
#include <QDate>
#include <QModelIndex>
namespace Ui {
class SaveSoundMainWindow;
}

class ItemModelForHistory;
class VoiceOverIpHandler;
class QMovie;
class QThread;

class SaveSoundMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SaveSoundMainWindow(QWidget *parent = 0);
    ~SaveSoundMainWindow();

protected:
    void closeEvent(QCloseEvent * event);
    bool eventFilter(QObject *obj, QEvent *e);

private:
    void setup(const QString &logPath, const int audioLogLenght, const QString &deviceName, const int sampleRate, const int sampleSize, const int mp3Bitrate, const QString mp3LameBitratePrefix);
    void updateLogDayInUi();

private Q_SLOTS:
    void onUpdateTimeTimeout();
    void log(const QString &text);
    void onSettingActionTriggered();
    void onHistoryItemDoubleClicked(const QModelIndex &idx);
    void onPrevDayButtonPushed();
    void onNextDayButtonPushed();

private:
    Ui::SaveSoundMainWindow *ui;
    VoiceOverIpHandler *voice;
    ItemModelForHistory *m_modelForHistory;
    QDate m_dateForLog;
    QMovie *m_errorStationStatusMovie;
    QMovie *m_waitStationStatusMovie;
    QMovie *m_sendStationStatusMovie;
    QMovie *m_receiveStationStatusMovie;
    QTimer *m_timerForShortcuts;
};

#endif // NETRADIOSOUNDMAINWINDOW_H
