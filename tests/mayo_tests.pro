TARGET = mayo_tests
TEMPLATE = app

CONFIG += c++17 no_batch

QT += testlib

INCLUDEPATH += \
    ../src/3rdparty

HEADERS += \
    test.h \

SOURCES += \
    test.cpp \
    main.cpp \
    ../src/base/brep_utils.cpp \
    ../src/base/mesh_utils.cpp \
    ../src/base/quantity.cpp \
    ../src/base/string_utils.cpp \
    ../src/base/unit.cpp \
    ../src/base/unit_system.cpp \

# OpenCascade
include(../opencascade.pri)
LIBS += -lTKernel -lTKMath -lTKBRep -lTKTopAlgo -lTKPrim -lTKMesh
