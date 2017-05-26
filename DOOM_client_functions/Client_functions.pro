TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.c \
    dconnect.c \
    client_functions.c
QMAKE_LFLAGS += -lncurses
QMAKE_LFLAGS += -lpthread
include(deployment.pri)
qtcAddDeployment()

HEADERS += \
    dconnect.h \
    dconnect_settings.h \
    client_functions.h \
    consoleColor.h

