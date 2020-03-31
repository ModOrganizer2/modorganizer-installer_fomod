#-------------------------------------------------
#
# Project created by QtCreator 2013-01-01T19:24:15
#
#-------------------------------------------------

TARGET = installerFomod
TEMPLATE = lib

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += plugins
CONFIG += dll

CONFIG(release, debug|release) {
  QMAKE_CXXFLAGS += /Zi /wd4503
  QMAKE_LFLAGS += /DEBUG
}

DEFINES += INSTALLERFOMOD_LIBRARY

SOURCES += installerfomod.cpp \
    fomodinstallerdialog.cpp \
    fomodscreenshotdialog.cpp \
    scalelabel.cpp \
    xmlreader.cpp

HEADERS += installerfomod.h \
    fomodinstallerdialog.h \
    fomodscreenshotdialog.h \
    scalelabel.h \
    xmlreader.h

FORMS += \
    fomodinstallerdialog.ui \
    fomodscreenshotdialog.ui

RESOURCES += \
    resources.qrc

OTHER_FILES += \
    installerfomod.json\
    SConscript\
    CMakeLists.txt

include(../plugin_template.pri)

INCLUDEPATH += "$${BOOSTPATH}"
INCLUDEPATH += ../gamefeatures

