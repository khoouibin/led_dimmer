QT = core
QT += serialport

CONFIG += c++17 cmdline
CONFIG -= app_bundle

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp \
        source/cli.cpp \
        source/logger_wrapper.cpp \
        source/interface_to_host.cpp \
        source/notification_pkg.cpp \
        source/config_settings.cpp \
        source/access_ini.cpp \
        source/HandleCmd.cpp

HEADERS += \
        header/json.hpp \
        header/cli.h \
        header/logger_wrapper.h \
        header/interface_to_host.h \
        header/notification_pkg.h \
        header/config_settings.h \
        header/access_ini.h \
        header/return_code.h \
        header/HandleCmd.h

INCLUDEPATH += $$PWD/header

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

LIBS += -lPocoUtil -lPocoData -lPocoFoundation -lPocoNet -lPocoJSON -lstdc++fs
