######################################################################
# Automatically generated by qmake (2.01a) Fri Jun 27 12:07:18 2008
######################################################################

#include( ../../version.pri )

RESOURCES = hdf5-import.qrc

TEMPLATE = app

DEPENDPATH += .

QT += xml

CONFIG += assistant
CONFIG += debug_and_release

DESTDIR = ../bin

TARGET = hdf5import

win32 {

INCLUDEPATH += . \
	       c:\drishtilib\hdf5\include

LIBPATH += c:\drishtilib\hdf5\lib \
	   c:\drishtilib\hdf5\szip\lib \
	   c:\drishtilib\hdf5\zlib\lib

LIBS += hdf5_cpp.lib \
	hdf5.lib \
	szlib.lib \
	zlib.lib

QMAKE_LFLAGS += /NODEFAULTLIB:msvcrt.lib  \
	        /NODEFAULTLIB:msvcprt.lib
}

unix {
!macx {

INCLUDEPATH += . \
	../../3rdParty/include \
        ../../3rdParty/include/ImageMagick

LIBPATH += ../../3rdParty/lib

LIBS += -lnetcdf_c++ \
        -lnetcdf \
        -lmfhdf \
	-ldf \
        -lMagick++ \
        -lMagickWand \
        -lMagickCore \
        -ljpeg \
        -lz \
	-lhdf5_cpp

DEFINES += DRISHTIIMPORT_VERSION="\\\"$${VERSION}\\\""
DEFINES += DRISHTIIMPORT_DOCDIR="\\\"$${INSTALLDOCDIR}\\\""
DEFINES += DRISHTIIMPORT_ASSISTANT="\\\"$${ASSISTANTBIN}\\\""

}
}

macx {
INCLUDEPATH += . \
	../../3rdParty/include \
        ../../3rdParty/include/ImageMagick

LIBPATH += ../../3rdParty/lib

LIBS +=	-lnetcdf \
	-lnetcdf_c++ \
	-lMagick++ \
	-lMagickCore \
	-lMagickWand \
	-lmfhdf \
	-ldf \
	-lz \
	-ljpeg
}

FORMS += remapwidget.ui \
	 savepvldialog.ui \
	 drishtiimport.ui \
	 fileslistdialog.ui

# Input
HEADERS += global.h \
	   common.h \
	   staticfunctions.h \
	   fileslistdialog.h \
	   remapwidget.h \
           remaphistogramline.h \
           remaphistogramwidget.h \
	   remapimage.h \
	   gradienteditor.h \
	   gradienteditorwidget.h \
	   dcolordialog.h \
	   dcolorwheel.h \
	   drishtiimport.h \
	   myslider.h \
	   raw2pvl.h \
	   savepvldialog.h \
	   blockfilewriter.h \
	   volumedata.h \
	   volinterface.h

SOURCES += global.cpp \
	   staticfunctions.cpp \
	   fileslistdialog.cpp \
	   main.cpp \
           remapwidget.cpp \
           remaphistogramline.cpp \
           remaphistogramwidget.cpp \
	   remapimage.cpp \
	   gradienteditor.cpp \
	   gradienteditorwidget.cpp \
	   dcolordialog.cpp \
	   dcolorwheel.cpp \
	   drishtiimport.cpp \
	   myslider.cpp \
	   raw2pvl.cpp \
	   savepvldialog.cpp \
	   volumedata.cpp \
	   blockfilewriter.cpp

