TEMPLATE = app
TARGET   = cmdlib
DESTDIR  = ../../final
QT       = core gui
QT      += sql

greaterThan(QT_MAJOR_VERSION,4): QT+= widgets
greaterThan(QT_MAJOR_VERSION,5): QT+= core5compat

TRANSLATIONS = ../final/loc/cmdlib_de.ts

HEADERS = \
    adddialog.h \
    cfgaccess.h \
    dbaccess.h \
    dbconnect.h \
    dbsqlite.h \
    dbtext.h \
    introwindow.h \
    main.h \
    mainwindow.h \
    settingsdialog.h \
    terminalwindow.h

SOURCES = \
    adddialog.cpp \
    cfgaccess.cpp \
    dbaccess.cpp \
    dbconnect.cpp \
    dbsqlite.cpp \
    dbtext.cpp \
    introwindow.cpp \
    main.cpp \
    mainwindow.cpp \
    settingsdialog.cpp \
    terminalwindow.cpp
