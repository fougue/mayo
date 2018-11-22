TARGET = mayo_tests
TEMPLATE = app

CONFIG += c++14 no_batch

QT += testlib

HEADERS += \
    test.h \
    ../src/brep_utils.h \
    ../src/quantity.h \
    ../src/result.h \
    ../src/unit.h \
    ../src/unit_system.h \

SOURCES += \
    test.cpp \
    main.cpp \
    ../src/brep_utils.cpp \
    ../src/quantity.cpp \
    ../src/unit.cpp \
    ../src/unit_system.cpp \

# OpenCascade
include(../occ.pri)
LIBS += -lTKernel -lTKMath -lTKBRep -lTKTopAlgo -lTKPrim
