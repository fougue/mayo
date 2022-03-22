TARGET = mayo_tests
TEMPLATE = app

CONFIG += c++17 no_batch

QT += testlib
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x050F00

*msvc*:QMAKE_CXXFLAGS += /std:c++17
*g++*:QMAKE_CXXFLAGS += -std=c++17

INCLUDEPATH += \
    ../src/3rdparty

SOURCES += \
    main.cpp \
    ../src/3rdparty/fmt/src/format.cc \

# Base --
HEADERS += \
    test_base.h \
    $$files(../src/base/*.h) \
    $$files(../src/io_occ/*.h) \
    $$files(../src/io_ply/*.h) \

SOURCES += \
    test_base.cpp \
    $$files(../src/base/*.cpp) \
    $$files(../src/io_occ/*.cpp) \
    $$files(../src/io_ply/*.cpp) \

# App --
QT += widgets

HEADERS += \
    test_app.h \
    $$files(../src/graphics/*.h) \
    $$files(../src/gui/*.h) \
    $$files(../src/io_image/*.h) \

SOURCES += \
    test_app.cpp \
    ../src/app/qstring_utils.cpp \
    ../src/app/recent_files.cpp \
    ../src/app/theme.cpp \
    $$files(../src/graphics/*.cpp) \
    $$files(../src/gui/*.cpp) \
    $$files(../src/io_image/*.cpp) \

win32 {
    LIBS += -lOpengl32 -lUser32
}

# Copy input files
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

# -- For TestApp
LIBS += -lTKService -lTKV3d -lTKMeshVS -lTKOpenGl
