TEMPLATE = lib
CONFIG += plugin

TARGET = rawslicesplugin
DESTDIR = ../../../bin/plugin

INCLUDEPATH += ../../

FORMS += loadrawdialog.ui

HEADERS = rawslicesplugin.h \
	  loadrawdialog.h

SOURCES = rawslicesplugin.cpp \
	  loadrawdialog.cpp

