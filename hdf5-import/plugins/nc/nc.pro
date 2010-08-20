TEMPLATE = lib
CONFIG += plugin

TARGET = ncplugin
DESTDIR = ../../../bin/plugin

win32 {
INCLUDEPATH += ../../ \
	       c:\drishtilib\netcdf\include \

LIBPATH += c:\drishtilib\netcdf\lib

LIBS +=	netcdf.lib
}

unix {
!macx {

INCLUDEPATH += ../..

LIBS += -lnetcdf

}
}


HEADERS = ncplugin.h

SOURCES = ncplugin.cpp

