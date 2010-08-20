TEMPLATE = lib
CONFIG += plugin

TARGET = magickplugin
DESTDIR = ../../../bin/plugin

INCLUDEPATH += ../../ \
	       %MAGICK_HOME%\include

LIBPATH += %MAGICK_HOME%\lib

LIBS +=	core_rl_magick++_.lib \
	core_rl_magick_.lib \
	core_rl_wand_.lib

HEADERS = magickplugin.h

SOURCES = magickplugin.cpp

