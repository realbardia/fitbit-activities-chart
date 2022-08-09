QT += core gui charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

SOURCES += \
    fitbitchart.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    fitbitchart.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

RESOURCES += \
    resource.qrc
