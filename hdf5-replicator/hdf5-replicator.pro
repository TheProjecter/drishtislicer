######################################################################
# Automatically generated by qmake (2.01a) Fri Jun 27 12:07:18 2008
######################################################################

#include( ../../version.pri )

RESOURCES = hdf5-replicator.qrc

TEMPLATE = app

DEPENDPATH += .

QT += xml

#CONFIG += assistant
CONFIG += debug_and_release

DESTDIR = ../bin

TARGET = hdf5replicator

win32 {

INCLUDEPATH += . \
	       c:\drishtilib\netcdf\include \
	       c:\drishtilib\hdf4\include \
	       %MAGICK_HOME%\include \
	       c:\drishtilib\hdf5\include

LIBPATH += c:\drishtilib\netcdf\lib \
	   c:\drishtilib\hdf4\dll \
	   %MAGICK_HOME%\lib \
	   c:\drishtilib\hdf5\lib \
	   c:\drishtilib\hdf5\szip\lib \
	   c:\drishtilib\hdf5\zlib\lib

LIBS += hdf5_cpp.lib \
	hdf5.lib \
#	hd423m.lib \
#	mfhdf_fcstubdll.lib \
#	hdf_fcstubdll.lib \
#	hm423m.lib \
#	netcdf.lib \
	core_rl_magick++_.lib \
	core_rl_magick_.lib \
	core_rl_wand_.lib \
	szlib.lib 


QMAKE_LFLAGS += /NODEFAULTLIB:msvcrt.lib  \
	        /NODEFAULTLIB:msvcprt.lib

}

unix {
!macx {

INCLUDEPATH += . \
  /usr/include/ImageMagick

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

FORMS += loadrawdialog.ui \
	 remapwidget.ui \
	 savepvldialog.ui \
	 drishtiimport.ui \
	 fileslistdialog.ui

# Input
HEADERS += global.h \
	   staticfunctions.h \
	   fileslistdialog.h \
	   remapwidget.h \
           remaphistogramline.h \
           remaphistogramwidget.h \
	   remapimage.h \
	   abstractremapvolume.h \
           remapncvolume.h \
           remaprawvolume.h \
	   remapimagevolume.h \
	   remapdicomvolume.h \
	   remaprawslices.h \
	   remaprawslabs.h \
	   gradienteditor.h \
	   gradienteditorwidget.h \
	   loadrawdialog.h \
	   dcolordialog.h \
	   dcolorwheel.h \
	   drishtiimport.h \
	   myslider.h \
	   raw2pvl.h \
	   savepvldialog.h \
	   tomhead.h \
	   remaptomvolume.h \
	   remapanalyze.h \
	   remaphdf4.h \
	   volumefilemanager.h \
	   blockfilewriter.h

SOURCES += global.cpp \
	   staticfunctions.cpp \
	   fileslistdialog.cpp \
	   main.cpp \
           remapwidget.cpp \
           remaphistogramline.cpp \
           remaphistogramwidget.cpp \
	   remapimage.cpp \
           remapncvolume.cpp \
           remaprawvolume.cpp \
	   remapimagevolume.cpp \
	   remapdicomvolume.cpp \
	   remaprawslices.cpp \
	   remaprawslabs.cpp \
	   gradienteditor.cpp \
	   gradienteditorwidget.cpp \
	   loadrawdialog.cpp \
	   dcolordialog.cpp \
	   dcolorwheel.cpp \
	   drishtiimport.cpp \
	   myslider.cpp \
	   raw2pvl.cpp \
	   savepvldialog.cpp \
	   remaptomvolume.cpp \
	   remapanalyze.cpp \
	   remaphdf4.cpp \
	   volumefilemanager.cpp \
	   blockfilewriter.cpp

