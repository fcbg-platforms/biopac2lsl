QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


win32: LIBS += -L$$PWD/./ -lmpdev

INCLUDEPATH += $$PWD/mpdev
DEPENDPATH += $$PWD/mpdev

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/./mpdev.lib
else:win32-g++: PRE_TARGETDEPS += $$PWD/./libmpdev.a

win32: LIBS += -L$$PWD/./ -llsl

INCLUDEPATH += $$PWD/lsl
DEPENDPATH += $$PWD/lsl

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/./lsl.lib
else:win32-g++: PRE_TARGETDEPS += $$PWD/./liblsl.a
