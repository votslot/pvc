#-------------------------------------------------
#
# Project created by QtCreator 2019-06-22T23:15:59
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = pcv-qt
TEMPLATE = app

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
        main.cpp \
        mainwindow.cpp \
     openglwindow.cpp \
    quad.cpp \
    ../TexturedQuad/texturedquad.cpp \
    ../PcrLib/src/pcrlib.cpp \
    ../PcrLib/src/OpenGL/icompute-gl.cpp \
    ../PcrLib/src/OpenGL/blit-gl.cpp \
    ../PcrLib/src/matrix-utils.cpp \
    ../PcrLib/src/storage.cpp \
    ../PcrLib/src/partition.cpp

HEADERS += \
        mainwindow.h \
    openglwindow.h \
    mainwindow.h \
    openglwindow.h \
    ../PcrLib/pcrlib.h \
    ../PcrLib/src/icompute.h \
    ../PcrLib/src/OpenGL/wrapper-gl.h \
    ../PcrLib/src/storage.h \
    ../PcrLib/src/partition.h \
    ../PcrLib/src/matrix-utils.h \
    ../PcrLib/src/OpenGL/shaders/ginclude.h

FORMS += \
        mainwindow.ui
