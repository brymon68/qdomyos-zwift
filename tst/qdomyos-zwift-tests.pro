GOOGLETEST_DIR = $$PWD/googletest

include(../defaults.pri)
include(gtest_dependency.pri)

TEMPLATE = app

CONFIG += console c++11
CONFIG -= app_bundle
CONFIG += thread
CONFIG += androidextras

SOURCES += \
        TrainingProgram/trainprogramtestsuite.cpp \
        ToolTests/qfittestsuite.cpp \
        ToolTests/treadmillrunnclosedlooptests.cpp \
        ToolTests/testsettingstestsuite.cpp \
        ToolTests/testtrainingloadtestsuite.cpp \
        ToolTests/zwiftworkouttestsuite.cpp \
        ToolTests/workoutimporttestsuite.cpp \
        Tools/testsettings.cpp \
        main.cpp

# Avoid the "File too big" error building in Windows. This has happened when a template class is used with Google Test / typed tests
# to produce a large number of classes.
win32:QMAKE_CXXFLAGS += -Wa,-mbig-obj

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../src/release/ -lqdomyos-zwift
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../src/debug/ -lqdomyos-zwift
else:unix: LIBS += -L$$OUT_PWD/../src/ -lqdomyos-zwift

INCLUDEPATH += $$PWD/../src $$PWD/../src/devices $$PWD/../src/fit-sdk
DEPENDPATH += $$PWD/../src $$PWD/../src/devices

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../src/release/libqdomyos-zwift.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../src/debug/libqdomyos-zwift.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../src/release/qdomyos-zwift.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../src/debug/qdomyos-zwift.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../src/libqdomyos-zwift.a

HEADERS += \
    TrainingProgram/trainprogramtestsuite.h \
    ToolTests/qfittestsuite.h \
    ToolTests/testsettingstestsuite.h \
    ToolTests/testtrainingloadtestsuite.h \
    ToolTests/zwiftworkouttestsuite.h \
    Tools/testsettings.h
