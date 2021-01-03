#-------------------------------------------------
#
# Project created by QtCreator 2019-11-05T10:50:05
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = MitTest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
LIBS += -LDLL -lws2_32

SOURCES += main.cpp \
    libezsocket/ezsocket.c \
    libezsocket/ezsocket_new.c \
    libezsocket/ezsocket_private.c \
    libezsocket/ezsocket_tcp_private.c

HEADERS += \
    libezsocket/ezsocket.h \
    libezsocket/ezsocket_new.h \
    libezsocket/ezsocket_private.h \
    libezsocket/ezsocket_tcp_private.h
