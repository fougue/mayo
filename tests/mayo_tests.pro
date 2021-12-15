TARGET = mayo_tests
TEMPLATE = app

CONFIG += c++17 no_batch

QT += testlib
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x050F00

*msvc*:QMAKE_CXXFLAGS += /std:c++17
*g++*:QMAKE_CXXFLAGS += -std=c++17

INCLUDEPATH += \
    ../src/3rdparty

HEADERS += \
    test.h \
    $$files(../src/base/*.h) \
    $$files(../src/io_occ/*.h) \
    ../src/gui/qtgui_utils.h \

SOURCES += \
    test.cpp \
    main.cpp \
    \
    $$files(../src/base/*.cpp) \
    $$files(../src/io_occ/*.cpp) \
    ../src/app/qstring_utils.cpp \
    ../src/gui/qtgui_utils.cpp \
    \
    ../src/3rdparty/fmt/src/format.cc \

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
        ../src/io_occ/io_occ_base_mesh.cpp \
        ../src/io_occ/io_occ_gltf_reader.cpp \
        ../src/io_occ/io_occ_obj_reader.cpp
}

!minOpenCascadeVersion(7, 5, 0) {
    SOURCES -= ../src/io_occ/io_occ_gltf_writer.cpp
}
!minOpenCascadeVersion(7, 6, 0) {
    SOURCES -= ../src/io_occ/io_occ_obj_writer.cpp
}
# -- VRML support
LIBS += -lTKVRML
