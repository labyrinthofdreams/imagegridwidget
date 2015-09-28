#-------------------------------------------------
#
# Project created by QtCreator 2015-09-10T02:07:49
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = imagegridwidget
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    ..\imagegridwidget.cpp

HEADERS  += mainwindow.hpp \
    ..\imagegridwidget.hpp

FORMS    += mainwindow.ui

QMAKE_CXXFLAGS += -std=c++11
