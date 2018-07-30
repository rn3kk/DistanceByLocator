#-------------------------------------------------
#
# Project created by QtCreator 2016-04-13T17:48:56
#
#-------------------------------------------------

QT       += core

QT       -= gui


TARGET = DistanceByLocator
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

INCLUDEPATH += "C:/Program Files (x86)/hamlib-w32-3.0.1/include"

LIBS += -L"C:/Program Files (x86)/hamlib-w32-3.0.1/lib/msvc"
LIBS += -llibhamlib-2

SOURCES += main.cpp
