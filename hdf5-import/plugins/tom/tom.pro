TEMPLATE = lib
CONFIG += plugin

TARGET = tomplugin
DESTDIR = ../../../bin/plugin

INCLUDEPATH += ../../

HEADERS = tomhead.h \
	  tomplugin.h

SOURCES = tomplugin.cpp

