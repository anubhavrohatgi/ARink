#-------------------------------------------------
#
# Project created by QtCreator 2017-09-14T10:27:32
#
#-------------------------------------------------

QT       += core gui widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ARink
TEMPLATE = app


QMAKE_CXXFLAGS_RELEASE += -O3 -msse4.1 -mssse3 -msse3 -msse2 -msse

OBJECTS_DIR = .tmplib
MOC_DIR = .tmplib

INCLUDEPATH += /usr/local/include \

LIBS += -L/usr/local/lib \
        -lopencv_highgui \
        -lopencv_core \
        -lopencv_imgproc \
        -lopencv_imgcodecs\
        -lopencv_objdetect\
        -lopencv_calib3d\
        -lopencv_ximgproc



LIBS += -L/usr/lib \
        -ltbb

SOURCES += main.cpp\
        mainwindow.cpp \
    ocvhelpers.cpp \
    filters.cpp

HEADERS  += mainwindow.h \
    ocvhelpers.h \
    interpolation.h \
    filters.h

FORMS    += mainwindow.ui

RESOURCES += \
    resources.qrc
