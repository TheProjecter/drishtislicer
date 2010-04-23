TEMPLATE = lib
CONFIG += plugin

TARGET = imagestackplugin
DESTDIR = ../../../bin/plugin

INCLUDEPATH += ../../

HEADERS = common.h \
	  imagestackplugin.h

SOURCES = imagestackplugin.cpp

