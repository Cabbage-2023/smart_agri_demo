#-------------------------------------------------
#  智慧农业环境监测与控制系统
#  ELF1 (i.MX6ULL) / Qt 5.6.2 / 交叉编译
#-------------------------------------------------
QT       += core gui widgets

CONFIG   += c++11

TARGET   = smart_agri
TEMPLATE = app

SOURCES += \
    main.cpp \
    aht20device.cpp \
    bh1726device.cpp \
    sensorworker.cpp \
    fancontroller.cpp \
    ledcontroller.cpp \
    trendchart.cpp \
    mainwindow.cpp

HEADERS += \
    hwconfig.h \
    aht20device.h \
    bh1726device.h \
    sensorworker.h \
    fancontroller.h \
    ledcontroller.h \
    trendchart.h \
    mainwindow.h
