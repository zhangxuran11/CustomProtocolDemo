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
mytarget.target = all
mytarget.commands = scp ./${TARGET} root@192.168.1.30:/${TARGET}_only_recv
QMAKE_EXTRA_TARGETS += mytarget
