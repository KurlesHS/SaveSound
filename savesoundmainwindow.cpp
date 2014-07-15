#include "savesoundmainwindow.h"
#include "ui_savesoundmainwindow.h"

#include "settingdialog.h"
#include "VoiceOverIp/voiceoveriphandler.h"
#include "History/itemmodelforhistory.h"
#include <QDateTime>
#include <QTimer>
#include <QMessageBox>
#include <QMovie>
#include <QDesktopServices>
#include <QCalendarWidget>
#include <QDialogButtonBox>
#include <QCloseEvent>
#include <QShortcut>
#include <QThread>


static const char settingsFileName[] = "settings.ini";

SaveSoundMainWindow::SaveSoundMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SaveSoundMainWindow),
    voice(nullptr)
{
    ui->setupUi(this);

    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->actionSettings, SIGNAL(triggered()), this, SLOT(onSettingActionTriggered()));
    m_waitStationStatusMovie = new QMovie("://resources/gifs/wait.gif");
    m_waitStationStatusMovie->start();
    m_errorStationStatusMovie = new QMovie("://resources/gifs/error.gif");
    m_errorStationStatusMovie->start();
    m_sendStationStatusMovie = new QMovie("://resources/gifs/send.gif");
    m_sendStationStatusMovie->start();
    m_receiveStationStatusMovie = new QMovie("://resources/gifs/recive.gif");
    m_receiveStationStatusMovie->start();

    SettingDialog dlg(settingsFileName, this);
    m_modelForHistory = new ItemModelForHistory(dlg.getPath());
    ui->treeView->setModel(m_modelForHistory);
    ui->treeView->setIndentation(0);
    ui->treeView->header()->setSectionResizeMode(QHeaderView::Stretch);
    connect(ui->treeView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onHistoryItemDoubleClicked(QModelIndex)));
    setup(dlg.getPath(), dlg.getLenghtAudioLog(),
          dlg.getDeviceName(), dlg.getSampleRate(),
          dlg.getSampleSize(), dlg.getMp3Bitrate(),
          dlg.getLameBitratePrefix());
    ui->widget->setBytePerSample(dlg.getSampleSize() / 8);
    m_dateForLog = QDate::currentDate();
    m_modelForHistory->setDate(m_dateForLog);
    updateLogDayInUi();
    connect(ui->toolButtonPrevDay, SIGNAL(clicked()), this, SLOT(onPrevDayButtonPushed()));
    connect(ui->toolButtonNextDay, SIGNAL(clicked()), this, SLOT(onNextDayButtonPushed()));
    ui->labelCalendarButton->installEventFilter(this);
    ui->labelTodayButton->installEventFilter(this);
    auto timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &SaveSoundMainWindow::onUpdateTimeTimeout);
    timer->start(1000);
    m_timerForShortcuts = new QTimer(this);
}

SaveSoundMainWindow::~SaveSoundMainWindow()
{
    delete ui;
    if (voice) {
        delete voice;
    }
}

void SaveSoundMainWindow::closeEvent(QCloseEvent *event)
{
    int button = QMessageBox::warning(this, tr("Внимание!"), tr("Вы действительно хотите выйти из программы?"),
                                      tr("Да"), tr("Нет"), QString(), 1);
    if (button == 1) {
        event->ignore();
    } else {
        event->accept();
    }
}

bool SaveSoundMainWindow::eventFilter(QObject *obj, QEvent *e)
{
    if (obj == ui->labelCalendarButton || obj == ui->labelTodayButton) {
        if (e->type() == QEvent::MouseButtonPress) {
            if (obj == ui->labelTodayButton) {
                auto currentDate = QDate::currentDate();
                if (m_dateForLog != currentDate) {
                    m_dateForLog = currentDate;
                    updateLogDayInUi();
                    m_modelForHistory->setDate(m_dateForLog);
                }
            } else if (obj == ui->labelCalendarButton) {
                auto calendar = new QCalendarWidget();
                calendar->setMaximumDate(QDate::currentDate());
                calendar->setSelectedDate(m_dateForLog);
                QDialog dlg(this);
                dlg.setWindowTitle(tr("Выбор дня для истории переговоров"));
                dlg.setWindowFlags(Qt::Dialog
                                   | Qt::WindowTitleHint
                                   | Qt::WindowCloseButtonHint
                                   | Qt::CustomizeWindowHint);
                auto lay = new QVBoxLayout(&dlg);
                dlg.setLayout(lay);
                auto btnBox = new QDialogButtonBox();
                btnBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
                lay->addWidget(calendar);
                lay->addWidget(btnBox);
                connect(btnBox, SIGNAL(accepted()), &dlg, SLOT(accept()));
                connect(btnBox, SIGNAL(rejected()), &dlg, SLOT(reject()));
                if (dlg.exec() == QDialog::Accepted) {
                    m_dateForLog = calendar->selectedDate();
                    updateLogDayInUi();
                    m_modelForHistory->setDate(m_dateForLog);

                }
            }
            return true;
        }
    }
        return QObject::eventFilter(obj, e);

}

void SaveSoundMainWindow::setup(const QString &logPath, const int audioLogLenght,
                                const QString &deviceName, const int sampleRate,
                                const int sampleSize, const int mp3Bitrate,
                                const QString mp3LameBitratePrefix)
{
    qDebug() << logPath << audioLogLenght;
    if (voice) {
        delete voice;
    }
    voice = new VoiceOverIpHandler();
    connect(voice, &VoiceOverIpHandler::log, this, &SaveSoundMainWindow::log);
    connect(voice, &VoiceOverIpHandler::outgoingData, ui->widget, &AduioVisualizerWidget::updateData);
    connect(voice, &VoiceOverIpHandler::logFileUpdated, m_modelForHistory, &ItemModelForHistory::onLogFileUpdated);
    voice->setAudioLogPath(logPath);
    voice->setLenghtAudioLogInMinutes(audioLogLenght);
    voice->setDeviceName(deviceName);
    voice->setSampleRate(sampleRate);
    voice->setSampleSize(sampleSize);
    voice->setMp3Bitrate(mp3Bitrate);
    voice->setMp3LameBitratePrefix(mp3LameBitratePrefix);
    voice->start();


}

void SaveSoundMainWindow::updateLogDayInUi()
{
    static const QStringList dates = {
        tr("Понедельник"),
        tr("Вторник"),
        tr("Среда"),
        tr("Четверг"),
        tr("Пятница"),
        tr("Суббота"),
        tr("Воскресение")
    };

    auto dataInText = m_dateForLog.toString("dd.MM.yyyy");
    auto text = tr("%0, %1")
            .arg(dataInText)
            .arg(dates.at(m_dateForLog.dayOfWeek() - 1));
    ui->labelDateForHistory->setText(text);
}

void SaveSoundMainWindow::onUpdateTimeTimeout()
{
    auto time = QDateTime::currentDateTime();
    ui->labelForCurrnetTime->setText(time.time().toString());
    ui->labelForCurrnetDate->setText(time.date().toString());
}

void SaveSoundMainWindow::log(const QString &text)
{
    Q_UNUSED(text)
    /*
    ui->plainTextEdit->appendPlainText(tr("%1: %2")
                                       .arg(QDateTime::currentDateTime().toString())
                                       .arg(text));
                                       */
}

void SaveSoundMainWindow::onSettingActionTriggered()
{
    qDebug() << Q_FUNC_INFO;
    SettingDialog dlg(settingsFileName, this);

    if (dlg.exec() == QDialog::Accepted) {
        setup(dlg.getPath(), dlg.getLenghtAudioLog(),
              dlg.getDeviceName(), dlg.getSampleRate(),
              dlg.getSampleSize(), dlg.getMp3Bitrate(),
              dlg.getLameBitratePrefix());
        ui->widget->setBytePerSample(dlg.getSampleSize() / 8);
    }
}

void SaveSoundMainWindow::onHistoryItemDoubleClicked(const QModelIndex &idx)
{
    QString path = m_modelForHistory->getFilepathFromIndex(idx);
    if (path.isEmpty())
        return;
    QDesktopServices::openUrl(QUrl::fromUserInput(path));
}

void SaveSoundMainWindow::onPrevDayButtonPushed()
{
    m_dateForLog = m_dateForLog.addDays(-1);
    updateLogDayInUi();
    m_modelForHistory->setDate(m_dateForLog);
}

void SaveSoundMainWindow::onNextDayButtonPushed()
{
    if (m_dateForLog < QDate::currentDate()) {
        m_dateForLog = m_dateForLog.addDays(1);
        updateLogDayInUi();
        m_modelForHistory->setDate(m_dateForLog);
    }
}
