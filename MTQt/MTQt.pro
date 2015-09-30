#macx {
#    QMAKE_MAC_SDK = macosx10.10
#}

QT += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MTQt
TEMPLATE = app


SOURCES += main.cpp \
           mainwidget.cpp \
           glwidget.cpp

HEADERS  += mainwidget.h \
            glwidget.h \
            libav.h

QMAKE_CXXFLAGS += -std=c++11

LIBS+= -lavcodec -lavformat -lavutil
#INCLUDEPATH += /usr/local/include/libavcodec
#INCLUDEPATH += /usr/local/include/libavformat
#INCLUDEPATH += /usr/local/include/libavutil
