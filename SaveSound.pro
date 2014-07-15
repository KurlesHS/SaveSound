#-------------------------------------------------
#
# Project created by QtCreator 2014-03-18T13:54:48
#
#-------------------------------------------------

CONFIG   += c++11
QT       += core gui network multimedia

INCLUDEPATH += $$PWD

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG(debug, debug|release):{OBJECTS_DIR = $$PWD/build/debug/obj}
else:{OBJECTS_DIR = $$PWD/build/release/obj}
RCC_DIR = $$PWD/build/rcc
UI_DIR = $$PWD/build/ui
MOC_DIR = $$PWD/build/moc

DESTDIR = $$PWD/app
TARGET   = SaveSound
TEMPLATE = app

DEFINES += _UNICODE

SOURCES  += \
        main.cpp\
        savesoundmainwindow.cpp \
        VoiceOverIp/voiceoveriphandler.cpp \
    VoiceOverIp/ringiodevice.cpp \
    settingdialog.cpp \
    aduiovisualizerwidget.cpp \
    History/itemmodelforhistory.cpp

HEADERS  += \
        savesoundmainwindow.h \
        VoiceOverIp/voiceoveriphandler.h \
    VoiceOverIp/ringiodevice.h \
    VoiceOverIp/voicehandler_p.h \
    settingdialog.h \
    aduiovisualizerwidget.h \
    History/itemmodelforhistory.h

FORMS    += \
        savesoundmainwindow.ui \
    settingdialog.ui \
    aduiovisualizerwidget.ui

RESOURCES += \
    resources.qrc
