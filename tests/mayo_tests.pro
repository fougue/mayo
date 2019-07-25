TARGET = mayo_tests
TEMPLATE = app

CONFIG += c++14 no_batch

QT += testlib

HEADERS += \
    test.h \

SOURCES += \
    test.cpp \
    main.cpp \
    ../src/brep_utils.cpp \
    ../src/mesh_utils.cpp \
    ../src/quantity.cpp \
    ../src/string_utils.cpp \
    ../src/unit.cpp \
    ../src/unit_system.cpp \

# OpenCascade
include(../opencascade.pri)
LIBS += -lTKernel -lTKMath -lTKBRep -lTKTopAlgo -lTKPrim -lTKMesh
