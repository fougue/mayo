TARGET = mayo_tests
TEMPLATE = app

CONFIG += c++17 no_batch

QT += testlib

*msvc*:QMAKE_CXXFLAGS += /std:c++17
*g++*:QMAKE_CXXFLAGS += -std=c++17

INCLUDEPATH += \
    ../src/3rdparty

HEADERS += \
    test.h \
    $$files(../src/base/*.h) \

SOURCES += \
    test.cpp \
    main.cpp \
    \
    ../src/3rdparty/fougtools/occtools/qt_utils.cpp \
    $$files(../src/base/*.cpp) \

include(../src/3rdparty/fougtools/qttools/task/qttools_task.pri)

CONFIG += file_copies
COPIES += MayoInputs
MayoInputs.files = $$files(inputs/*.*)
MayoInputs.path = $$OUT_PWD/inputs

# OpenCascade
include(../opencascade.pri)

LIBS += -lTKernel -lTKMath -lTKBRep -lTKGeomBase -lTKTopAlgo -lTKPrim -lTKMesh -lTKG3d
LIBS += -lTKXSBase -lTKIGES -lTKSTEP -lTKXDESTEP -lTKXDEIGES
LIBS += -lTKLCAF -lTKXCAF -lTKCAF
LIBS += -lTKSTL
LIBS += -lTKCDF -lTKBin -lTKBinL -lTKBinXCAF -lTKXml -lTKXmlL -lTKXmlXCAF
