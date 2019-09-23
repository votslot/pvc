#-------------------------------------------------
#
# Project created by QtCreator 2019-07-10T23:45:06
#
#-------------------------------------------------

QT       += opengl

QT       -= gui

TARGET = pcr
TEMPLATE = lib
CONFIG += staticlib

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_BUILD

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ../src/partition.cpp \
    ../src/pcrlib.cpp \
    ../src/OpenGL/blit-gl.cpp \
    ../src/OpenGL/icompute-gl.cpp \
    ../src/storage.cpp \
    ../src/matrix-utils.cpp \
    ../src/font.cpp

HEADERS += \
    ../src/compute.h \
    ../src/icompute.h \
    ../src/partition.h \
    ../src/OpenGL/wrapper-gl.h \
    ../src/storage.h \
    ../src/matrix-utils.h
unix {
#    target.path = /usr/lib
    target.path = ./
    INSTALLS += target
}

DESTDIR = $$PWD/LibOutput
QMAKE_CXXFLAGS += -Wno-missing-braces


DISTFILES += \
    ../src/OpenGL/wave-test.cs.glsl \
    ../src/OpenGL/shaders/render-points.cs.glsl \
    ../src/OpenGL/shaders/post-proc.cs.glsl \
    ../src/OpenGL/shaders/render-font.cs.glsl
