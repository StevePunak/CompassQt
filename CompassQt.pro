QT -= gui
QT += mqtt

CONFIG += c++11 console static
CONFIG -= app_bundle

DEFINES += QT_DEPRECATED_WARNINGS

TARGET=compass

INCLUDEPATH+= $$[QT_SYSROOT]/usr/include
# Point this at the KanoopCommonQt sources from git@github.com:StevePunak/KanoopCommonQt.git
INCLUDEPATH+=$$(HOME)/src/KanoopCommonQt

SOURCES += \
        calibration.cpp \
        compassdaemon.cpp \
        lsm9ds1.cpp \
        main.cpp \
        monitorthread.cpp \
        mqttpublish.cpp \
        pollthread.cpp

LIBS += -lKanoopCommon -lwiringPi

unix:
contains(CONFIG, cross_compile):{
        message("building for PI")
#  1. location of libKanoopCommonQt.a
        LIBS += -L${HOME}/lib/arm
#  2. location of the Qt libraries on target
        LIBS += -Wl,-rpath=/usr/local/qt5pi/$$[QT_VERSION]/lib
    }else{
        message("Not building for PI")
        LIBS += -L${HOME}/lib/x86
    }

# Default rules for deployment. Point it to your location on pi
unix:!android: target.path = /home/pi/opt/compass

!isEmpty(target.path): INSTALLS += target

DISTFILES +=

HEADERS += \
    calibration.h \
    compassdaemon.h \
    lsm9ds1.h \
    lsm9ds1_registers.h \
    lsm9ds1_types.h \
    monitorthread.h \
    mqttpublish.h \
    pollthread.h

