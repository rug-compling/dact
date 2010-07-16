QT += svg
TARGET = dact
TEMPLATE = app
SOURCES += main.cpp \
    DactMainWindow.cpp
HEADERS += DactMainWindow.h
FORMS += DactMainWindow.ui
RESOURCES += application.qrc
LIBS += -lxerces-c -lxalan-c
