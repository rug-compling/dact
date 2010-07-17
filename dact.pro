QT += svg
TARGET = dact
TEMPLATE = app
SOURCES += main.cpp \
    DactMainWindow.cpp
HEADERS += DactMainWindow.h
FORMS += DactMainWindow.ui
RESOURCES += application.qrc
LIBS += -lxerces-c -lxalan-c -L/home/daniel/sw/Alpino/TreebankTools/IndexedCorpus -lcorpus
INCLUDEPATH += /home/daniel/sw/Alpino/TreebankTools/IndexedCorpus /home/daniel/sw/Alpino/util
