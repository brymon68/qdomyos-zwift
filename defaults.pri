QT += gui bluetooth widgets quick networkauth websockets
android: QT += androidextras gui-private
android: include(android_openssl/openssl.pri)
INCLUDEPATH += $$PWD/src/qmdnsengine/src/include
ANDROID_PACKAGE_SOURCE_DIR = $$PWD/src/android
ANDROID_ABIS = arm64-v8a
