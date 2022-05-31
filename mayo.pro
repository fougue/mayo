#****************************************************************************
#* Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
#* All rights reserved.
#* See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
#****************************************************************************

TEMPLATE = app
TARGET = mayo
include(version.pri)
CONFIG(debug, debug|release) {
    message(Mayo version $$MAYO_VERSION debug)
} else {
    message(Mayo version $$MAYO_VERSION release)
}

message(Qt version $$QT_VERSION)

QT += core gui widgets
greaterThan(QT_MAJOR_VERSION, 5) {
    QT += openglwidgets
}

CONFIG += c++17
CONFIG(debug, debug|release) {
    CONFIG += console
} else {
    CONFIG -= console
    CONFIG += release_with_debuginfo
}

DEFINES += \
    QT_DISABLE_DEPRECATED_BEFORE=0x050F00 \
    QT_IMPLICIT_QFILEINFO_CONSTRUCTION

release_with_debuginfo:msvc {
    # https://docs.microsoft.com/en-us/cpp/build/reference/how-to-debug-a-release-build
    QMAKE_CXXFLAGS_RELEASE += /Zi
    QMAKE_LFLAGS_RELEASE += /DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF
}

msvc {
    DEFINES += NOMINMAX
    QMAKE_CXXFLAGS += /we4150 # Deletion of pointer to incomplete type 'XXXX'; no destructor called
    QMAKE_CXXFLAGS += /std:c++17
}
gcc|clang {
    QMAKE_CXXFLAGS += -std=c++17
}
clang {
    # Silent Clang warnings about instantiation of variable 'Mayo::GenericProperty<T>::TypeName'
    QMAKE_CXXFLAGS += -Wno-undefined-var-template
}
*clang-libc++* {
    # See https://libcxx.llvm.org/docs/UsingLibcxx.html
    LIBS += -lc++fs
}
macx {
    DEFINES += GL_SILENCE_DEPRECATION
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.15
#   QMAKE_CXXFLAGS += -mmacosx-version-min=10.15
}
win32 {
    LIBS += -lOpengl32 -lUser32
}

INCLUDEPATH += \
    src/app \
    src/3rdparty

HEADERS += \
    $$files(src/base/*.h) \
    $$files(src/io_occ/*.h) \
    $$files(src/io_dxf/*.h) \
    $$files(src/io_image/*.h) \
    $$files(src/io_ply/*.h) \
    $$files(src/graphics/*.h) \
    $$files(src/gui/*.h) \
    $$files(src/app/*.h) \

SOURCES += \
    $$files(src/base/*.cpp) \
    $$files(src/io_occ/*.cpp) \
    $$files(src/io_dxf/*.cpp) \
    $$files(src/io_image/*.cpp) \
    $$files(src/io_ply/*.cpp) \
    $$files(src/graphics/*.cpp) \
    $$files(src/gui/*.cpp) \
    $$files(src/app/*.cpp) \
    \
    src/3rdparty/fmt/src/format.cc \

win32:lessThan(QT_MAJOR_VERSION, 6) {
    QT += winextras
    HEADERS += $$files(src/app/windows/*.h)
    SOURCES += $$files(src/app/windows/*.cpp)
}

FORMS += $$files(src/app/*.ui)

RESOURCES += mayo.qrc
RC_ICONS = images/appicon.ico

OTHER_FILES += \
    README.md \
    appveyor.yml \
    .travis.yml \
    .github/workflows/ci.yml \
    images/credits.txt \
    scripts/bump-version.rb \
    scripts/travis-build.sh \

# OpenCascade
include(opencascade.pri)
!isEmpty(OCC_VERSION_STR) {
    message(OpenCascade version $$OCC_VERSION_STR)
} else {
    warning(OpenCascade version <unknown>)
}

LIBS += \
    -lTKBin \
    -lTKBinL \
    -lTKBinXCAF \
    -lTKBO \
    -lTKBool \
    -lTKBRep \
    -lTKCAF \
    -lTKCDF \
    -lTKernel \
    -lTKG2d \
    -lTKG3d \
    -lTKGeomAlgo \
    -lTKGeomBase \
    -lTKHLR \
    -lTKLCAF \
    -lTKMath \
    -lTKMesh \
    -lTKMeshVS \
    -lTKOpenGl \
    -lTKPrim \
    -lTKService \
    -lTKShHealing \
    -lTKTopAlgo  \
    -lTKXSDRAW \  # Seems to be required on macOS(see https://github.com/fougue/mayo/issues/41#issuecomment-742732322)
    -lTKV3d  \
    -lTKVCAF \
    -lTKXCAF \
    -lTKXml \
    -lTKXmlL \
    -lTKXmlXCAF \
    -lTKXSBase \

# -- IGES support
LIBS += -lTKIGES -lTKXDEIGES
# -- STEP support
LIBS += -lTKSTEP -lTKSTEP209 -lTKSTEPAttr -lTKSTEPBase -lTKXDESTEP
# -- STL support
LIBS += -lTKSTL
# -- OBJ/glTF support
minOpenCascadeVersion(7, 4, 0) {
    LIBS += -lTKRWMesh
} else {
    SOURCES -= \
        src/io_occ/io_occ_base_mesh.cpp \
        src/io_occ/io_occ_gltf_reader.cpp \
        src/io_occ/io_occ_obj_reader.cpp
}

!minOpenCascadeVersion(7, 5, 0) {
    SOURCES -= src/io_occ/io_occ_gltf_writer.cpp
}

!minOpenCascadeVersion(7, 6, 0) {
    SOURCES -= src/io_occ/io_occ_obj_writer.cpp
}
# -- VRML support
LIBS += -lTKVRML

# gmio
isEmpty(GMIO_ROOT) {
    message(gmio OFF)
} else {
    message(gmio ON)
    CONFIG(debug, debug|release) {
        #GMIO_BIN_SUFFIX = d
        GMIO_BIN_SUFFIX =
    } else {
        GMIO_BIN_SUFFIX =
    }

    HEADERS += $$files(src/io_gmio/*.h)
    SOURCES += $$files(src/io_gmio/*.cpp)

    INCLUDEPATH += $$GMIO_ROOT/include
    LIBS += -L$$GMIO_ROOT/lib -lgmio_static -lzlibstatic
    SOURCES += \
#        $$GMIO_ROOT/src/gmio_support/stl_occ_brep.cpp \
#        $$GMIO_ROOT/src/gmio_support/stl_occ_polytri.cpp \
        $$GMIO_ROOT/src/gmio_support/stream_qt.cpp
    DEFINES += HAVE_GMIO
}

# Unit tests
CONFIG(withtests) {
    include(tests/tests.pri)
    DEFINES += MAYO_WITH_TESTS
}

# Developer custom processing
exists($$PWD/custom.pri) {
    include($$PWD/custom.pri)
}
