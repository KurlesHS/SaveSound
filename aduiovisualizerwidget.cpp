#include "aduiovisualizerwidget.h"
#include "ui_aduiovisualizerwidget.h"

#include <QPainter>
#include <QDebug>

AduioVisualizerWidget::AduioVisualizerWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AduioVisualizerWidget),
    m_sender(0),
    m_byteCount(2)
{
    ui->setupUi(this);
}

AduioVisualizerWidget::~AduioVisualizerWidget()
{
    delete ui;
}

void AduioVisualizerWidget::setBytePerSample(const int &byteCount)
{
    m_byteCount = byteCount;
}

void AduioVisualizerWidget::updateData(const QByteArray &data, const int sender)
{
    m_visualizeData = data;
    m_sender = sender;
    update();
}

void AduioVisualizerWidget::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e)
    static const QColor green("#00ff21");
    static const QColor red("#ff0000");
    static const QColor yellow("#ffd800");
    QPainter painter(this);
    painter.fillRect(rect(), m_sender == 0 ? green : red);
    QPen p;
    p.setBrush(yellow);
    p.setWidth(3);
    painter.setPen(p);

    double dy = pow(256.0, m_byteCount);
    double mediumDy = dy / 2.0;
    double stepX = (double)width() / (double)m_visualizeData.length() * m_byteCount;
    double stepY = (double)height() / dy;
    QPointF lastPoint;
    for (double x = 0; x < m_visualizeData.length() / m_byteCount; ++x) {
        int value = mediumDy;
        if (m_byteCount == 1) {
            value = (quint8)m_visualizeData.at((int)x);
        } else if (m_byteCount == 2) {
            value = (qint16)((quint8)m_visualizeData.at((int)x * m_byteCount) | ((quint8)m_visualizeData.at((int)x * m_byteCount + 1) << 0x08));
            value += mediumDy;
        }
        QPointF currentPoint = QPointF(stepX * x, stepY * value);
        if (int(x) == 0) {
            lastPoint = currentPoint;
            continue;
        }
        painter.drawLine(lastPoint, currentPoint);
        //qDebug() << lastPoint << currentPoint;
        lastPoint = currentPoint;
    }
}
