#ifndef SETTINGDIALOG_H
#define SETTINGDIALOG_H

#include <QDialog>
#include <QAudioDeviceInfo>

namespace Ui {
class SettingDialog;
}

class QComboBox;
class SettingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingDialog(const QString &settingsFile, QWidget *parent = 0);
    ~SettingDialog();

    int getLocalPortNumber() const;
    int getRemotePortNumber() const;
    QString getPath() const;
    int getSampleSize() const;
    int getSampleRate() const;
    QString getDeviceName() const;
    int getLenghtAudioLog() const;
    int getMp3Bitrate() const;
    QString getLameBitratePrefix() const;

private:
    void saveSettings();
    void loadSettings();
    QString selectDirectory(const QString &title);
    void checkCurrentFormat();
    QAudioDeviceInfo getCurrentDeviceInfo();
    void findAndSetDefaultValueOnComboBox(QComboBox * const comboBox, const QString &value);

private Q_SLOTS:
    void onAccept();
    void onSelectLogDirButtonPushed();
    void onSelectAudioDeviceComboboxValueChanged(const QString &name);
    void onNeedToUpdateInfoAboutFormat(const int idx);

private:
    Ui::SettingDialog *ui;
    QString m_settingsFile;
};

#endif // SETTINGDIALOG_H
