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

CONFIG += file_copies
COPIES += MayoInputs
MayoInputs.files = $$files(inputs/*.*)
MayoInputs.path = $$OUT_PWD/inputs

# OpenCascade
include(../opencascade.pri)
LIBS += -lTKernel -lTKMath -lTKBRep -lTKGeomBase -lTKTopAlgo -lTKPrim -lTKMesh -lTKG3d
LIBS += -lTKXSBase
LIBS += -lTKLCAF -lTKXCAF -lTKCAF
LIBS += -lTKCDF -lTKBin -lTKBinL -lTKBinXCAF -lTKXml -lTKXmlL -lTKXmlXCAF
# -- IGES support
LIBS += -lTKIGES -lTKXDEIGES
# -- STEP support
LIBS += -lTKSTEP -lTKXDESTEP
# -- STL support
LIBS += -lTKSTL
# -- OBJ/glTF support
minOpenCascadeVersion(7, 4, 0) {
    LIBS += -lTKRWMesh
} else {
    SOURCES -= \
        ../src/base/io_occ_base_mesh.cpp \
        ../src/base/io_occ_gltf_reader.cpp \
        ../src/base/io_occ_obj.cpp
}

!minOpenCascadeVersion(7, 5, 0) {
    SOURCES -= ../src/base/io_occ_gltf_writer.cpp
}
# -- VRML support
LIBS += -lTKVRML
