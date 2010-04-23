TEMPLATE = lib
CONFIG += plugin

TARGET = tomplugin
DESTDIR = ../../../bin/plugin

INCLUDEPATH += ../../

HEADERS = common.h \
	  tomhead.h \
	  tomplugin.h

SOURCES = tomplugin.cpp

