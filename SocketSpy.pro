QT += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

SOURCES += \
    aseman/SslServer.cpp \
    aseman/asemanabstracttcpserver.cpp \
    aseman/asemansslserver.cpp \
    aseman/asemantcpserver.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    aseman/SslServer.h \
    aseman/asemanabstracttcpserver.h \
    aseman/asemansslserver.h \
    aseman/asemantcpserver.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

RESOURCES += \
    resource.qrc
