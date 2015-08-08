QT       += core gui network
CONFIG   += debug_and_release

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ckb
TEMPLATE = app
# GL isn't needed
QMAKE_LIBS_OPENGL =

QMAKE_CFLAGS += -Wno-unused-parameter
QMAKE_CXXFLAGS += -Wno-unused-parameter

# Output path
macx {
    DESTDIR = $$PWD/../..
} else {
    DESTDIR = $$PWD/../../bin
}

# OSX settings
QMAKE_MAC_SDK = macosx10.10
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9
ICON = ckb-logo.icns
QMAKE_INFO_PLIST = ckb-info.plist
macx {
    LIBS += -framework Foundation -framework AudioToolbox
}

# Load version number from VERSION file
CKB_VERSION_STR = `cat $$PWD/../../VERSION`
DEFINES += CKB_VERSION_STR="\\\"$$CKB_VERSION_STR\\\""

# Zip library for decompressing firmwares
LIBS += -lz
DEFINES += QUAZIP_STATIC

# Conditionally use libappindicator to support Unity indicators
linux {
system(pkg-config --exists appindicator-0.1) {
    CONFIG += link_pkgconfig
    PKGCONFIG += appindicator-0.1
    DEFINES += USE_LIBAPPINDICATOR
}
}

SOURCES += main.cpp\
        mainwindow.cpp \
    kbwidget.cpp \
    colorbutton.cpp \
    settingswidget.cpp \
    kblightwidget.cpp \
    keymap.cpp \
    media_linux.cpp \
    kblight.cpp \
    kbprofile.cpp \
    kbanimwidget.cpp \
    animscript.cpp \
    kbanim.cpp \
    animadddialog.cpp \
    animsettingdialog.cpp \
    gradientbutton.cpp \
    gradientdialog.cpp \
    gradientdialogwidget.cpp \
    kbmode.cpp \
    kb.cpp \
    rlistwidget.cpp \
    kbprofiledialog.cpp \
    keywidget.cpp \
    kbbindwidget.cpp \
    kbbind.cpp \
    rebindwidget.cpp \
    modeselectdialog.cpp \
    quazip/JlCompress.cpp \
    quazip/qioapi.cpp \
    quazip/quaadler32.cpp \
    quazip/quacrc32.cpp \
    quazip/quagzipfile.cpp \
    quazip/quaziodevice.cpp \
    quazip/quazip.cpp \
    quazip/quazipdir.cpp \
    quazip/quazipfile.cpp \
    quazip/quazipfileinfo.cpp \
    quazip/quazipnewinfo.cpp \
    quazip/unzip.c \
    quazip/zip.c \
    kbfirmware.cpp \
    fwupgradedialog.cpp \
    autorun.cpp \
    ckbsettings.cpp \
    kbperf.cpp \
    ckbsettingswriter.cpp \
    keyaction.cpp \
    mperfwidget.cpp

HEADERS  += mainwindow.h \
    kbwidget.h \
    colorbutton.h \
    settingswidget.h \
    kblightwidget.h \
    keymap.h \
    media.h \
    kblight.h \
    kbprofile.h \
    kbanimwidget.h \
    animscript.h \
    ckb-anim.h \
    kbanim.h \
    animadddialog.h \
    animsettingdialog.h \
    gradientbutton.h \
    gradientdialog.h \
    gradientdialogwidget.h \
    kbmode.h \
    kb.h \
    rlistwidget.h \
    kbprofiledialog.h \
    keywidget.h \
    kbbindwidget.h \
    kbbind.h \
    rebindwidget.h \
    modeselectdialog.h \
    quazip/crypt.h \
    quazip/ioapi.h \
    quazip/JlCompress.h \
    quazip/quaadler32.h \
    quazip/quachecksum32.h \
    quazip/quacrc32.h \
    quazip/quagzipfile.h \
    quazip/quaziodevice.h \
    quazip/quazip.h \
    quazip/quazip_global.h \
    quazip/quazipdir.h \
    quazip/quazipfile.h \
    quazip/quazipfileinfo.h \
    quazip/quazipnewinfo.h \
    quazip/unzip.h \
    quazip/zip.h \
    kbfirmware.h \
    fwupgradedialog.h \
    autorun.h \
    ckbsettings.h \
    kbperf.h \
    ckbsettingswriter.h \
    keyaction.h \
    mperfwidget.h

FORMS    += mainwindow.ui \
    kbwidget.ui \
    settingswidget.ui \
    kblightwidget.ui \
    kbanimwidget.ui \
    animadddialog.ui \
    animsettingdialog.ui \
    gradientdialog.ui \
    kbprofiledialog.ui \
    kbbindwidget.ui \
    rebindwidget.ui \
    modeselectdialog.ui \
    fwupgradedialog.ui \
    mperfwidget.ui

RESOURCES += \
    image.qrc \
    text.qrc \
    binary.qrc

OBJECTIVE_SOURCES += \
    media_mac.m

DISTFILES += \
    ckb-info.plist
