#ifndef ITEMMODELFORHISTORY_H
#define ITEMMODELFORHISTORY_H

#include <QStandardItemModel>
#include <QDate>
#include <QTime>
#include "MediaInfo/MediaInfoDLL.h"

class ItemModelForHistory : public QStandardItemModel
{
    Q_OBJECT
    enum Roles {
        FileNameRole = Qt::UserRole + 1
    };

public:
    explicit ItemModelForHistory(const QString &dirForHistory, QObject *parent = 0);
    void setDate(const QDate &date);
    void setDirForHistory(const QString &dir);
    QString getFilepathFromIndex(const QModelIndex &idx);

private:
    void setupHeaders();
    void updateModel();
    void addFileInModel(const QTime &startTime, const QTime &duration, const QString &filename);
    void addFileInLog(const QString &filepath);

public Q_SLOTS:
    void onLogFileUpdated(const QString &filepath);

signals:

public slots:

private:
    QString m_dirForHistory;
    QDate m_date;
    QRegExp m_regexpForFile;
    MediaInfoDLL::MediaInfo m_mediaInfo;
};

#endif // ITEMMODELFORHISTORY_H
