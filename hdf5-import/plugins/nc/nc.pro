TEMPLATE = lib
CONFIG += plugin

TARGET = ncplugin
DESTDIR = ../../../bin/plugin

INCLUDEPATH += ../../ \
	       c:\drishtilib\netcdf\include \

LIBPATH += c:\drishtilib\netcdf\lib

LIBS +=	netcdf.lib

HEADERS = ncplugin.h

SOURCES = ncplugin.cpp

