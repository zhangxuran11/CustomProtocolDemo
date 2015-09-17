QT += core gui network
TARGET = CustomProtocolDemo
TEMPLATE = app

include(ZTPManager/ZTPManager.pri)


FORMS    += mainwindow.ui

HEADERS += \
    mainwindow.h

SOURCES += \
    mainwindow.cpp \
    main.cpp
