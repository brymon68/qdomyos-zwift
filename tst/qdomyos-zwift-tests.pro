GOOGLETEST_DIR = $$PWD/googletest

include(../defaults.pri)
include(gtest_dependency.pri)

TEMPLATE = app

CONFIG += console c++11
CONFIG -= app_bundle
CONFIG += thread
CONFIG += androidextras

SOURCES += \
        ToolTests/testsettingstestsuite.cpp \
        Tools/testsettings.cpp \
        main.cpp

LIBS += -L$$OUT_PWD/../src/ -lqdomyos-zwift

INCLUDEPATH += $$PWD/../src $$PWD/../src/devices
DEPENDPATH += $$PWD/../src $$PWD/../src/devices

PRE_TARGETDEPS += $$OUT_PWD/../src/libqdomyos-zwift.a

HEADERS += \
    ToolTests/testsettingstestsuite.h \
    Tools/testsettings.h
