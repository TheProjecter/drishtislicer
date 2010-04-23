TEMPLATE = lib
CONFIG += plugin

TARGET = rawslabsplugin
DESTDIR = ../../../bin/plugin

INCLUDEPATH += ../../

FORMS += loadrawdialog.ui

HEADERS = common.h \
	  rawslabsplugin.h \
	  loadrawdialog.h

SOURCES = rawslabsplugin.cpp \
	  loadrawdialog.cpp

