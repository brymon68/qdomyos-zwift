include(../defaults.pri)
QT += httpserver core-private
TARGET = x22i
CONFIG += c++17 console optimize_full ltcg
DEFINES += QT_DEPRECATED_WARNINGS IO_UNDER_QT Q_HTTPSERVER
QMAKE_LFLAGS_RELEASE += -s
QMAKE_CXXFLAGS += -fno-sized-deallocation
CONFIG -= optimize_size
QMAKE_CFLAGS_OPTIMIZE_FULL -= -Oz
QMAKE_CFLAGS_OPTIMIZE_FULL += -O3
INCLUDEPATH += devices/ qmdnsengine/src/include

SOURCES += \
    androidqlog.cpp \
    logwriter.cpp \
    virtualdevices/virtualdevice.cpp \
    localipaddress.cpp \
    characteristics/characteristicnotifier2a53.cpp \
    characteristics/characteristicnotifier2acc.cpp \
    characteristics/characteristicnotifier2acd.cpp \
    characteristics/characteristicnotifier2ad9.cpp \
    characteristics/characteristicwriteprocessor.cpp \
    devices/nordictrackifitadbtreadmill/nordictrackifitadbtreadmill.cpp \
    qmdnsengine/src/src/abstractserver.cpp \
    qmdnsengine/src/src/bitmap.cpp \
    qmdnsengine/src/src/browser.cpp \
    qmdnsengine/src/src/cache.cpp \
    qmdnsengine/src/src/dns.cpp \
    qmdnsengine/src/src/hostname.cpp \
    qmdnsengine/src/src/mdns.cpp \
    qmdnsengine/src/src/message.cpp \
    qmdnsengine/src/src/prober.cpp \
    qmdnsengine/src/src/provider.cpp \
    qmdnsengine/src/src/query.cpp \
    qmdnsengine/src/src/record.cpp \
    qmdnsengine/src/src/resolver.cpp \
    qmdnsengine/src/src/server.cpp \
    qmdnsengine/src/src/service.cpp \
    devices/bluetooth.cpp \
    devices/bluetoothdevice.cpp \
    characteristics/characteristicnotifier2a37.cpp \
    characteristics/characteristicnotifier2ad2.cpp \
    characteristics/characteristicwriteprocessor2ad9.cpp \
    devices/dircon/dirconmanager.cpp \
    devices/dircon/dirconpacket.cpp \
    devices/dircon/dirconprocessor.cpp \
    homeform.cpp \
    keepawakehelper.cpp \
    main.cpp \
    metric.cpp \
    qzsettings.cpp \
    signalhandler.cpp \
    templateinfosenderbuilder.cpp \
    devices/treadmill.cpp \
    virtualdevices/virtualtreadmill.cpp \
    androidstatusbar.cpp \
    zwiftincline.cpp \
    webserverinfosender.cpp

HEADERS += \
    logwriter.h \
    zwift-api/PlayerStateWrapper.h \
    zwift-api/zwift_client_auth.h \
    virtualdevices/virtualdevice.h \
    localipaddress.h \
    characteristics/characteristicnotifier2a53.h \
    characteristics/characteristicnotifier2acc.h \
    characteristics/characteristicnotifier2acd.h \
    characteristics/characteristicnotifier2ad9.h \
    definitions.h \
    devices/nordictrackifitadbtreadmill/nordictrackifitadbtreadmill.h \
    qmdnsengine/src/include/qmdnsengine/abstractserver.h \
    qmdnsengine/src/include/qmdnsengine/bitmap.h \
    qmdnsengine/src/include/qmdnsengine/browser.h \
    qmdnsengine/src/include/qmdnsengine/cache.h \
    qmdnsengine/src/include/qmdnsengine/dns.h \
    qmdnsengine/src/include/qmdnsengine/hostname.h \
    qmdnsengine/src/include/qmdnsengine/mdns.h \
    qmdnsengine/src/include/qmdnsengine/message.h \
    qmdnsengine/src/include/qmdnsengine/prober.h \
    qmdnsengine/src/include/qmdnsengine/provider.h \
    qmdnsengine/src/include/qmdnsengine/query.h \
    qmdnsengine/src/include/qmdnsengine/record.h \
    qmdnsengine/src/include/qmdnsengine/resolver.h \
    qmdnsengine/src/include/qmdnsengine/server.h \
    qmdnsengine/src/include/qmdnsengine/service.h \
    qmdnsengine/src/src/bitmap_p.h \
    qmdnsengine/src/src/browser_p.h \
    qmdnsengine/src/src/cache_p.h \
    qmdnsengine/src/src/hostname_p.h \
    qmdnsengine/src/src/message_p.h \
    qmdnsengine/src/src/prober_p.h \
    qmdnsengine/src/src/provider_p.h \
    qmdnsengine/src/src/query_p.h \
    qmdnsengine/src/src/record_p.h \
    qmdnsengine/src/src/resolver_p.h \
    qmdnsengine/src/src/server_p.h \
    qmdnsengine/src/src/service_p.h \
    devices/bluetooth.h \
    devices/bluetoothdevice.h \
    characteristics/characteristicnotifier.h \
    characteristics/characteristicnotifier2a37.h \
    characteristics/characteristicnotifier2ad2.h \
    characteristics/characteristicwriteprocessor.h \
    characteristics/characteristicwriteprocessor2ad9.h \
    devices/dircon/dirconmanager.h \
    devices/dircon/dirconpacket.h \
    devices/dircon/dirconprocessor.h \
    homeform.h \
    keepawakehelper.h \
    metric.h \
    qdebugfixup.h \
    qmdnsengine_export.h \
    qzsettings.h \
    signalhandler.h \
    templateinfosenderbuilder.h \
    devices/treadmill.h \
    virtualdevices/virtualtreadmill.h \
    wobjectdefs.h \
    wobjectimpl.h \
    androidstatusbar.h \
    zwiftincline.h \
    zwiftinclinegrade.h \
    webserverinfosender.h

RESOURCES += icons.qrc qml.qrc

VERSION = 2.21.5
