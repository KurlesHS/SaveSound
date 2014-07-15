#ifndef ADUIOVISUALIZERWIDGET_H
#define ADUIOVISUALIZERWIDGET_H

#include <QWidget>

namespace Ui {
class AduioVisualizerWidget;
}

class AduioVisualizerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AduioVisualizerWidget(QWidget *parent = 0);
    ~AduioVisualizerWidget();
    void setBytePerSample(const int &byteCount);

public Q_SLOTS:
    void updateData(const QByteArray &data, const int sender);

protected:
    void paintEvent(QPaintEvent *e);


private:
    Ui::AduioVisualizerWidget *ui;
    QByteArray m_visualizeData;
    int m_sender;
    int m_byteCount;
};

#endif // ADUIOVISUALIZERWIDGET_H
