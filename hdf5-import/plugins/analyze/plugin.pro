TEMPLATE = lib
CONFIG += plugin

TARGET = analyzeplugin
DESTDIR = ../../../bin/plugin

INCLUDEPATH += ../../

HEADERS = common.h \
	  analyzeplugin.h

SOURCES = analyzeplugin.cpp

