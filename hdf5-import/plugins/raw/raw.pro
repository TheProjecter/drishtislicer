TEMPLATE = lib
CONFIG += plugin

TARGET = rawplugin
DESTDIR = ../../../bin/plugin

INCLUDEPATH += ../../

FORMS += loadrawdialog.ui

HEADERS = rawplugin.h \
	  loadrawdialog.h

SOURCES = rawplugin.cpp \
	  loadrawdialog.cpp

