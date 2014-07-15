#include "settingdialog.h"
#include "ui_settingdialog.h"
#include <QSettings>
#include <QDebug>
#include <QFileDialog>
#include <QAudioDeviceInfo>

const static char *audioFilePathSection = "main/audiofilePath";
const static char *audioFileLengthSection = "main/audiofileLenght";
const static char *audioInputSampleSizeSection = "main/audioinputsamplesize";
const static char *audioInputSampleRateSection = "main/audioinputsamplerate";
const static char *audioInputDeviceSection = "main/audioinputdevice";
const static char *mp3BitRateSection = "main/mp3samplerate";
const static char *mp3LameSampleRatePrefixSection = "main/mp3lamesamplerateprefix";

const static char *acceptableFormatStr = "<html><head/><body><p><span style=\" font-size:9pt; font-weight:600; color:#005500;\">Формат поддерживается устройством</span></p></body></html>";
const static char *unacceptableFormatStr = "<html><head/><body><p><span style=\" font-size:9pt; font-weight:600; color:#550000;\">Формат не поддерживается устройством</span></p></body></html>";


SettingDialog::SettingDialog(const QString &settingsFile, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingDialog),
    m_settingsFile(settingsFile)
{
    ui->setupUi(this);

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(onAccept()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    connect(ui->toolButtonSelectPath, SIGNAL(clicked()), this, SLOT(onSelectLogDirButtonPushed()));
    connect(ui->comboBoxAudioInputDevice, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(onSelectAudioDeviceComboboxValueChanged(QString)));
    connect(ui->comboBoxSampleRates, SIGNAL(currentIndexChanged(int)),
            this, SLOT(onNeedToUpdateInfoAboutFormat(int)));
    connect(ui->comboBoxSampleSize, SIGNAL(currentIndexChanged(int)),
            this, SLOT(onNeedToUpdateInfoAboutFormat(int)));

    ui->label->setText("");
    loadSettings();
}

SettingDialog::~SettingDialog()
{
    delete ui;
}


QString SettingDialog::getPath() const
{
    return ui->lineEditPath->text();
}

int SettingDialog::getSampleSize() const
{
    return ui->comboBoxSampleSize->currentText().toInt();
}

int SettingDialog::getSampleRate() const
{
    return ui->comboBoxSampleRates->currentText().toInt();
}

QString SettingDialog::getDeviceName() const
{
    return ui->comboBoxAudioInputDevice->currentText();
}

int SettingDialog::getLenghtAudioLog() const
{
    return ui->spinBoxTime->value();
}

int SettingDialog::getMp3Bitrate() const
{
    return ui->spinBoxMp3Bitrate->value();
}

QString SettingDialog::getLameBitratePrefix() const
{
    if (ui->comboBoxMp3BitrateType->currentIndex() == 0) {
        return QString("--abr");
    } else {
        return QString("-b");
    }
}

void SettingDialog::saveSettings()
{
    QSettings settings(m_settingsFile, QSettings::IniFormat);
    settings.setValue(audioFileLengthSection, getLenghtAudioLog());
    settings.setValue(audioFilePathSection, getPath());
    settings.setValue(audioInputSampleRateSection, getSampleRate());
    settings.setValue(audioInputSampleSizeSection, getSampleSize());
    settings.setValue(audioInputDeviceSection, getDeviceName());
    settings.setValue(mp3BitRateSection, getMp3Bitrate());
    settings.setValue(mp3LameSampleRatePrefixSection, getLameBitratePrefix());
}

void SettingDialog::loadSettings()
{
    auto devices = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
    for (const QAudioDeviceInfo &device : devices) {
        ui->comboBoxAudioInputDevice->addItem(device.deviceName());
    }

    QSettings settings(m_settingsFile, QSettings::IniFormat);
    QString audioLogPath = settings.value(audioFilePathSection, ".").toString();
    int audiofileLenght = settings.value(audioFileLengthSection, 10).toInt();
    QString deviceName = settings.value(audioInputDeviceSection, "").toString();
    QString lameBitratePrexif = settings.value(mp3LameSampleRatePrefixSection, "--abr").toString();
    int sampleSize = settings.value(audioInputSampleSizeSection, 0).toInt();
    int sampleRate = settings.value(audioInputSampleRateSection, 0).toInt();
    int mp3BitRate = settings.value(mp3BitRateSection, 32).toInt();

    ui->spinBoxMp3Bitrate->setValue(mp3BitRate);
    ui->lineEditPath->setText(audioLogPath);
    ui->spinBoxTime->setValue(audiofileLenght);
    findAndSetDefaultValueOnComboBox(ui->comboBoxAudioInputDevice, deviceName);
    findAndSetDefaultValueOnComboBox(ui->comboBoxSampleSize, QString::number(sampleSize));
    findAndSetDefaultValueOnComboBox(ui->comboBoxSampleRates, QString::number(sampleRate));
    ui->comboBoxMp3BitrateType->setCurrentIndex(lameBitratePrexif == "-b" ? 1 : 0);
}

QString SettingDialog::selectDirectory(const QString &title)
{
    return QFileDialog::getExistingDirectory(this, title);

}

void SettingDialog::checkCurrentFormat()
{
    auto currentDevice = getCurrentDeviceInfo();

    if (currentDevice.isNull()) {
        ui->label->setText(unacceptableFormatStr);
        return;
    }
    QAudioFormat af;
    af.setByteOrder(QAudioFormat::LittleEndian);
    int sampleSize = ui->comboBoxSampleSize->currentText().toInt();
    int sampleRate = ui->comboBoxSampleRates->currentText().toInt();
    af.setSampleRate(sampleRate);
    af.setSampleSize(sampleSize);
    af.setCodec("audio/pcm");
    af.setChannelCount(1);
    if (sampleSize == 8) {
        af.setSampleType(QAudioFormat::UnSignedInt);
    } else {
        af.setSampleType(QAudioFormat::SignedInt);
    }
    bool isFormatSupported = currentDevice.isFormatSupported(af);
    ui->label->setText(isFormatSupported ? acceptableFormatStr :
                                           unacceptableFormatStr);
}

QAudioDeviceInfo SettingDialog::getCurrentDeviceInfo()
{
    auto devices = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
    QString currentDeviceName = ui->comboBoxAudioInputDevice->currentText();
    QAudioDeviceInfo retVal;
    if (!currentDeviceName.isEmpty()){
        for (const QAudioDeviceInfo &dev : devices) {
            if (dev.deviceName() == currentDeviceName) {
                retVal = dev;
                break;
            }
        }
    }
    return retVal;
}

void SettingDialog::findAndSetDefaultValueOnComboBox(QComboBox * const comboBox, const QString &value)
{
    if (comboBox) {
        int idx = comboBox->findText(value);
        if (idx >=0) {
            comboBox->setCurrentIndex(idx);
        }
    }
}

void SettingDialog::onAccept()
{
    saveSettings();
    accept();
}

void SettingDialog::onSelectLogDirButtonPushed()
{
    QString path = selectDirectory(tr("Выбор директории для аудио истории"));
    if (!path.isEmpty()) {
        ui->lineEditPath->setText(path);
    }
}

void SettingDialog::onSelectAudioDeviceComboboxValueChanged(const QString &name)
{
    auto devices = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
    QString previousSampleRate = ui->comboBoxSampleRates->currentText();
    for (const QAudioDeviceInfo &dev : devices) {
        if (dev.deviceName() == name) {
            ui->comboBoxSampleRates->clear();
            auto sampleRates = dev.supportedSampleRates();
            for (const int sampleRate : sampleRates){
                ui->comboBoxSampleRates->addItem(QString::number(sampleRate));
            }
            int idx = ui->comboBoxSampleRates->findText(previousSampleRate);
            if (idx >= 0) {
                ui->comboBoxSampleRates->setCurrentIndex(idx);
            }
            break;
        }
    }
    checkCurrentFormat();
}

void SettingDialog::onNeedToUpdateInfoAboutFormat(const int idx)
{
    Q_UNUSED(idx)
    checkCurrentFormat();
}

