QT += core
QT -= gui

CONFIG += console c++11
CONFIG -= app_bundle

INCLUDEPATH += $$PWD

ZMQ_ROOT = $$(ZMQ_ROOT)
isEmpty(ZMQ_ROOT) {
    ZMQ_ROOT = $$PWD/../third_party/libzmq
}

ZMQ_LIB_NAME = $$(ZMQ_LIB_NAME)
isEmpty(ZMQ_LIB_NAME) {
    ZMQ_LIB_NAME = libzmq
}

INCLUDEPATH += $$ZMQ_ROOT/include

win32 {
    LIBS += -L$$ZMQ_ROOT/lib -l$$ZMQ_LIB_NAME
}

unix {
    LIBS += -lzmq
}

SOURCES += \
    $$PWD/zmq_helpers.cpp

HEADERS += \
    $$PWD/zmq_helpers.h
