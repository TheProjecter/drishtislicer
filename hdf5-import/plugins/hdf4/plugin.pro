TEMPLATE = lib
CONFIG += plugin

TARGET = hdf4plugin
DESTDIR = ../../../bin/plugin

INCLUDEPATH += ../../ \
	       c:\drishtilib\netcdf\include \
	       c:\drishtilib\hdf4\include

LIBPATH += c:\drishtilib\netcdf\lib \
	   c:\drishtilib\hdf4\dll

LIBS +=	netcdf.lib \
	hd423m.lib \
	mfhdf_fcstubdll.lib \
	hdf_fcstubdll.lib \
	hm423m.lib

HEADERS = common.h \
	  hdf4plugin.h

SOURCES = hdf4plugin.cpp

