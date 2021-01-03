#-------------------------------------------------
#
# Project created by QtCreator 2019-11-02T13:07:18
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = Simens
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
LIBS += -LDLL -lws2_32

SOURCES += main.cpp \
    libsiemenscnc/siemenscnc_tcp_private.c \
    libsiemenscnc/siemenscnc.c \
    libsiemenscnc/siemenscnc_private.c \
    libsiemenscnc/828d/siemens_828d_new.c \
    libsiemenscnc/siemenscnc_data.c

HEADERS += \
    libsiemenscnc/siemenscnc_tcp_private.h \
    libsiemenscnc/siemenscnc.h \
    libsiemenscnc/siemenscnc_private.h \
    libsiemenscnc/828d/siemens_828d_new.h
