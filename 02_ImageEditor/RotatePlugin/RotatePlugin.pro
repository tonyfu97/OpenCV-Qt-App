######################################################################
# Automatically generated by qmake (3.1) Tue Aug 8 08:21:27 2023
######################################################################

TEMPLATE = lib
TARGET = RotatePlugin
COPNFIG += plugin
INCLUDEPATH += . ..

# OpenCV
unix: mac {
    INCLUDEPATH += /opt/homebrew/include/opencv4
    LIBS += -L/opt/homebrew/opt/opencv/lib -lopencv_core -lopencv_imgproc
}

# You can make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# Please consult the documentation of the deprecated API in order to know
# how to port your code away from it.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Input
HEADERS += rotate_plugin.h
SOURCES += rotate_plugin.cpp
