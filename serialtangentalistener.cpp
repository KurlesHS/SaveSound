#include "serialtangentalistener.h"
#include <QSerialPort>
#include <QTimer>
#include <QDebug>

SerialTangentaListener::SerialTangentaListener(QObject *parent) :
    QObject(parent),
    m_serialPort(new QSerialPort(this)),
    m_tangentState(false),
    m_timerForSendData(new QTimer(this)),
    m_timeoutTimer(new QTimer(this))
{
    connect(m_serialPort, SIGNAL(readyRead()), this,
            SLOT(onReadyRead()));
    connect(m_timerForSendData, SIGNAL(timeout()),
            this, SLOT(onTimerForSendData()));
    connect(m_timeoutTimer, SIGNAL(timeout()),
            this, SLOT(onTimeoutTimer()));
    m_timeoutTimer->setInterval(20);
    m_timeoutTimer->setSingleShot(true);
    m_timerForSendData->setInterval(100);
}

SerialTangentaListener::~SerialTangentaListener()
{
    stop();
    delete m_serialPort;
}

bool SerialTangentaListener::start(const QString &portName)
{
    stop();
    m_serialPort->setPortName(portName);
    if (!m_serialPort->open(QIODevice::ReadWrite)) return false;
    if (!m_serialPort->setDataBits(QSerialPort::Data8)) return false;
    if (!m_serialPort->setFlowControl(QSerialPort::NoFlowControl)) return false;
    if (!m_serialPort->setStopBits(QSerialPort::OneStop)) return false;
    if (!m_serialPort->setParity(QSerialPort::NoParity)) return false;
    onTimerForSendData();
    m_timerForSendData->start();
    qDebug() << Q_FUNC_INFO;

    return true;
}

void SerialTangentaListener::stop()
{
    m_timeoutTimer->stop();
    m_timerForSendData->stop();
    emit tangentReleased();
    if (m_serialPort->isOpen())
        m_serialPort->close();
}

void SerialTangentaListener::onReadyRead()
{
    m_timeoutTimer->stop();
    if (!m_tangentState){
        m_tangentState = true;
        emit tangentPressed();
    }
}

void SerialTangentaListener::onTimeoutTimer()
{
    if (m_tangentState) {
        m_tangentState = false;
        emit tangentReleased();
    }
}

void SerialTangentaListener::onTimerForSendData()
{
    m_serialPort->write("X");
    m_timeoutTimer->start();
}
