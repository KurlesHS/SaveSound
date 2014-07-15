#include "itemmodelforhistory.h"
#include <QDir>
#include <QRegExp>
#include <QDebug>

ItemModelForHistory::ItemModelForHistory(const QString &dirForHistory, QObject *parent) :
    QStandardItemModel(parent),
    m_dirForHistory(dirForHistory),
    m_regexpForFile("(\\d{2})_(\\d{2})_(\\d{2})_(\\d+).mp3")
{
    setupHeaders();
}

void ItemModelForHistory::setDate(const QDate &date)
{
    m_date = date;
    updateModel();
}

void ItemModelForHistory::setDirForHistory(const QString &dir)
{
    m_dirForHistory = dir;
}

QString ItemModelForHistory::getFilepathFromIndex(const QModelIndex &idx)
{
    QString retVal;
    if (idx.model() == this) {
        retVal = idx.data(FileNameRole).toString();
    }
    return retVal;
}

void ItemModelForHistory::setupHeaders()
{
    setHorizontalHeaderLabels({tr("Время начала записи"), tr("Длительность")});
}

void ItemModelForHistory::updateModel()
{
    clear();
    setupHeaders();
    if (!m_date.isValid())
        return;

    QString folderName = QString("%1_%2_%3")
            .arg(m_date.day(), 2, 10, QChar('0'))
            .arg(m_date.month(), 2, 10, QChar('0'))
            .arg(m_date.year() % 100, 2, 10, QChar('0'));
    QDir dir(QDir(m_dirForHistory).absolutePath() + QDir::separator() + folderName);
    if (dir.exists()) {
        for (const QFileInfo &fileInfo : dir.entryInfoList({"*.mp3"}, QDir::Files)) {
            onLogFileUpdated(fileInfo.filePath());
        }
    }
}

void ItemModelForHistory::addFileInModel(const QTime &startTime, const QTime &duration, const QString &filename)
{
    auto startTimeItem = new QStandardItem(startTime.toString());
    startTimeItem->setData(filename, FileNameRole);
    startTimeItem->setEditable(false);
    auto durationItem = new QStandardItem(duration.toString());
    durationItem->setEditable(false);
    appendRow({startTimeItem, durationItem});
}

void ItemModelForHistory::addFileInLog(const QString &filepath)
{
    if (m_regexpForFile.indexIn(filepath) >= 0){
        int hour = m_regexpForFile.cap(1).toInt();
        int minute = m_regexpForFile.cap(2).toInt();
        int seconds = m_regexpForFile.cap(3).toInt();
        int dur= m_regexpForFile.cap(4).toInt();
        if (m_mediaInfo.IsReady()) {
            if(m_mediaInfo.Open(filepath.toStdWString())) {
                dur = QString::fromStdWString(m_mediaInfo.Get(MediaInfoDLL::Stream_Audio, 0, __T("Duration"), MediaInfoDLL::Info_Text, MediaInfoDLL::Info_Name)).toInt() / 1000;
                m_mediaInfo.Close();
            }
        }
        QTime startTime(hour, minute, seconds, 1);
        QTime duration = QTime(0, 0, 0, 1).addSecs(dur);
        QFileInfo fileInfo(filepath);
        if (fileInfo.isFile() && fileInfo.exists()) {
            addFileInModel(startTime, duration, filepath);
        }
    }
}

void ItemModelForHistory::onLogFileUpdated(const QString &filepath)
{
    if (QDate::currentDate() == m_date)
        addFileInLog(filepath);
}
