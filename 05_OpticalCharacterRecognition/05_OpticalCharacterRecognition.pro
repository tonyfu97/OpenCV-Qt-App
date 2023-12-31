######################################################################
# Automatically generated by qmake (3.1) Thu Aug 10 20:37:39 2023
######################################################################

TEMPLATE = app
TARGET = 05_OpticalCharacterRecognition

QT += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
INCLUDEPATH += .

# OpenCV
unix: mac {
    INCLUDEPATH += /opt/homebrew/include/opencv4
    LIBS += -L/opt/homebrew/opt/opencv/lib -lopencv_core -lopencv_imgproc -lopencv_dnn
}

# Tesseract
unix: mac {
    INCLUDEPATH += /opt/homebrew/include/tesseract_nested
    LIBS += -L/opt/homebrew/lib -ltesseract
}

# You can make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# Please consult the documentation of the deprecated API in order to know
# how to port your code away from it.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
DEFINES += TESSDATA_PREFIX=\\\"/opt/homebrew/share/tessdata/\\\"

# Input
HEADERS += mainwindow.h screencapturer.h
SOURCES += main.cpp mainwindow.cpp screencapturer.cpp
