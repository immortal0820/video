#-------------------------------------------------
#
# Project created by QtCreator 2016-12-08T15:21:32
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = video
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp \
    info.cpp \
    logininfor.cpp \
    videoplay.cpp \
    register.cpp
   

HEADERS  += widget.h \
    info.h \
    logininfor.h \
    videoplay.h \
    header.h \
    register.h
    

FORMS    += widget.ui \
    info.ui \
    logininfor.ui \
    videoplay.ui \
    register.ui
   

RESOURCES += \
    imag.qrc \
    imag.qrc

DISTFILES +=
