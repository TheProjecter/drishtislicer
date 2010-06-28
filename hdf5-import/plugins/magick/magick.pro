TEMPLATE = lib
CONFIG += plugin

TARGET = magickplugin
DESTDIR = ../../../bin/plugin

win32 {
INCLUDEPATH += ../../ \
	       %MAGICK_HOME%\include

LIBPATH += %MAGICK_HOME%\lib

LIBS +=	core_rl_magick++_.lib \
	core_rl_magick_.lib \
	core_rl_wand_.lib
}

unix {
!macx {

INCLUDEPATH += ../../ \
  /usr/include/ImageMagick

LIBS += -lMagick++ \
  -lMagickWand \
  -lMagickCore

}
}

HEADERS = magickplugin.h

SOURCES = magickplugin.cpp

