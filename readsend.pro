TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    readsend.cpp \
    rssem.cpp

LIBS += -lpthread -lrt

HEADERS += \
    readsend.h \
    rssem.h

