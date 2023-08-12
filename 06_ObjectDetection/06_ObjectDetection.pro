######################################################################
# Automatically generated by qmake (3.1) Sat Aug 12 09:41:55 2023
######################################################################

TEMPLATE = app
TARGET = 06_ObjectDetection

QT += core gui multimedia
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
INCLUDEPATH += .

# OpenCV
unix: mac {
    INCLUDEPATH += /opt/homebrew/include/opencv4
    LIBS += -L/opt/homebrew/opt/opencv/lib -lopencv_core -lopencv_imgproc -lopencv_imgcodecs -lopencv_video -lopencv_videoio
}

# You can make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# Please consult the documentation of the deprecated API in order to know
# how to port your code away from it.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Input
HEADERS += capture_thread.h mainwindow.h utilities.h
SOURCES += capture_thread.cpp main.cpp mainwindow.cpp utilities.cpp
