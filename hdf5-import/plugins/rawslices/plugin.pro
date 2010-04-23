TEMPLATE = lib
CONFIG += plugin

TARGET = rawslicesplugin
DESTDIR = ../../../bin/plugin

INCLUDEPATH += ../../

FORMS += loadrawdialog.ui

HEADERS = common.h \
	  rawslicesplugin.h \
	  loadrawdialog.h

SOURCES = rawslicesplugin.cpp \
	  loadrawdialog.cpp

