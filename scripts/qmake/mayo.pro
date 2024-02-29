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

MAYO_DIR = $$PWD/../..

# Check Qt version
message(Qt version $$QT_VERSION)
!versionAtLeast(QT_VERSION, 5.14) {
    error(Qt >= 5.14 is required but detected version is $$QT_VERSION)
}

# If variable 'CHECK_ARCH_64' is empty then host is not 64bit
CHECK_ARCH_64 = $$find(QT_ARCH, 64)

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
    QT_IMPLICIT_QFILEINFO_CONSTRUCTION \

release_with_debuginfo:msvc {
    # https://learn.microsoft.com/en-us/cpp/build/how-to-debug-a-release-build?view=msvc-170
    QMAKE_CXXFLAGS_RELEASE += /Zi
    QMAKE_LFLAGS_RELEASE += /DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF
}

msvc {
    DEFINES += NOMINMAX
    QMAKE_CXXFLAGS += /we4150 # Deletion of pointer to incomplete type 'XXXX'; no destructor called
    QMAKE_CXXFLAGS += /std:c++17

    greaterThan(QT_MAJOR_VERSION, 5) {
        DEFINES += _USE_MATH_DEFINES
    }
}

gcc|clang {
    QMAKE_CXXFLAGS += -std=c++17
}

clang {
    # Silent Clang warnings about instantiation of variable 'Mayo::GenericProperty<T>::TypeName'
    QMAKE_CXXFLAGS += -Wno-undefined-var-template
    # See https://libcxx.llvm.org/docs/UsingLibcxx.html
    # LIBS += -lstdc++fs
}

unix:isEmpty(CHECK_ARCH_64) {
    macx {
        DEFINES += _DARWIN_USE_64_BIT_INODE
    } else {
        DEFINES += _FILE_OFFSET_BITS=64  _LARGEFILE64_SOURCE=1
    }

    gcc|clang:contains(QT_ARCH, arm) {
        # See:
        #     https://stackoverflow.com/questions/48149323/what-does-the-gcc-warning-project-parameter-passing-for-x-changed-in-gcc-7-1-m
        #     https://stackoverflow.com/questions/52020305/what-exactly-does-gccs-wpsabi-option-do-what-are-the-implications-of-supressi
        QMAKE_CXXFLAGS += -Wno-psabi
    }
}

macx {
    DEFINES += GL_SILENCE_DEPRECATION
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.15
    LIBS += -liconv
#   QMAKE_CXXFLAGS += -mmacosx-version-min=10.15
    ICON = $$MAYO_DIR/images/appicon.icns
}

win32 {
    LIBS += -lOpengl32 -lUser32
}

INCLUDEPATH += \
    $$MAYO_DIR/src/app \
    $$MAYO_DIR/src/3rdparty

HEADERS += \
    $$files($$MAYO_DIR/src/base/*.h) \
    $$files($$MAYO_DIR/src/io_dxf/*.h) \
    $$files($$MAYO_DIR/src/io_image/*.h) \
    $$files($$MAYO_DIR/src/io_occ/*.h) \
    $$files($$MAYO_DIR/src/io_off/*.h) \
    $$files($$MAYO_DIR/src/io_ply/*.h) \
    $$files($$MAYO_DIR/src/graphics/*.h) \
    $$files($$MAYO_DIR/src/gui/*.h) \
    $$files($$MAYO_DIR/src/measure/*.h) \
    $$files($$MAYO_DIR/src/app/*.h) \

SOURCES += \
    $$files($$MAYO_DIR/src/base/*.cpp) \
    $$files($$MAYO_DIR/src/io_dxf/*.cpp) \
    $$files($$MAYO_DIR/src/io_image/*.cpp) \
    $$files($$MAYO_DIR/src/io_occ/*.cpp) \
    $$files($$MAYO_DIR/src/io_off/*.cpp) \
    $$files($$MAYO_DIR/src/io_ply/*.cpp) \
    $$files($$MAYO_DIR/src/graphics/*.cpp) \
    $$files($$MAYO_DIR/src/gui/*.cpp) \
    $$files($$MAYO_DIR/src/measure/*.cpp) \
    $$files($$MAYO_DIR/src/app/*.cpp) \
    \
    $$MAYO_DIR/src/3rdparty/fmt/src/format.cc \

win32:lessThan(QT_MAJOR_VERSION, 6) {
    QT += winextras
    HEADERS += $$files($$MAYO_DIR/src/app/windows/*.h)
    SOURCES += $$files($$MAYO_DIR/src/app/windows/*.cpp)
}

FORMS += $$files($$MAYO_DIR/src/app/*.ui)

RESOURCES += $$MAYO_DIR/mayo.qrc
RC_ICONS = $$MAYO_DIR/images/appicon.ico

OTHER_FILES += \
    $$MAYO_DIR/README.md \
    $$MAYO_DIR/.github/workflows/ci_linux.yml \
    $$MAYO_DIR/.github/workflows/ci_linux_arm.yml \
    $$MAYO_DIR/.github/workflows/ci_macos.yml \
    $$MAYO_DIR/.github/workflows/ci_windows.yml \
    $$MAYO_DIR/images/credits.txt \
    $$MAYO_DIR/scripts/bump-version.rb \

# Embed qtbase_*.qm files as a resource
qtBaseQmRes.files = \
    $$[QT_INSTALL_TRANSLATIONS]/qtbase_en.qm \
    $$[QT_INSTALL_TRANSLATIONS]/qtbase_fr.qm
qtBaseQmRes.base = $$[QT_INSTALL_TRANSLATIONS]
qtBaseQmRes.prefix = "/i18n"
RESOURCES += qtBaseQmRes

# Optional developer-specific QMake pri file for environment related settings
exists($$PWD/env.pri) {
    include($$PWD/env.pri)
}

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

versionAtLeast(OCC_VERSION_STR, 7.8.0) {
    # -- IGES support
    LIBS += -lTKDEIGES
    # -- STEP support
    LIBS += -lTKDESTEP
    # -- STL support
    LIBS += -lTKDESTL
    # -- VRML support
    LIBS += -lTKDEVRML
} else {
    # -- IGES support
    LIBS += -lTKIGES -lTKXDEIGES
    # -- STEP support
    LIBS += -lTKSTEP -lTKSTEP209 -lTKSTEPAttr -lTKSTEPBase -lTKXDESTEP
    # -- STL support
    LIBS += -lTKSTL
    # -- VRML support
    LIBS += -lTKVRML

    versionAtLeast(OCC_VERSION_STR, 7.7.0) {
        LIBS += -lTKXDE
    }
}

# -- OBJ/glTF support
versionAtLeast(OCC_VERSION_STR, 7.4.0) {
    LIBS += -lTKRWMesh
    versionAtLeast(OCC_VERSION_STR, 7.8.0) {
        LIBS += -lTKDEOBJ -lTKDEGLTF
    }
} else {
    SOURCES -= \
        $$MAYO_DIR/src/io_occ/io_occ_base_mesh.cpp \
        $$MAYO_DIR/src/io_occ/io_occ_gltf_reader.cpp \
        $$MAYO_DIR/src/io_occ/io_occ_obj_reader.cpp

    message(glTF reader disabled because OpenCascade < v7.4)
    message(OBJ reader disabled because OpenCascade < v7.4)
}

!versionAtLeast(OCC_VERSION_STR, 7.5.0) {
    SOURCES -= $$MAYO_DIR/src/io_occ/io_occ_gltf_writer.cpp
    message(glTF writer disabled because OpenCascade < v7.5)
}

!versionAtLeast(OCC_VERSION_STR, 7.6.0) {
    SOURCES -= $$MAYO_DIR/src/io_occ/io_occ_obj_writer.cpp
    message(OBJ writer disabled because OpenCascade < v7.6)
}

!versionAtLeast(OCC_VERSION_STR, 7.7.0) {
    SOURCES -= $$MAYO_DIR/src/io_occ/io_occ_vrml_reader.cpp
    message(VRML reader disabled because OpenCascade < v7.7)
}

# assimp
isEmpty(ASSIMP_INC_DIR) | isEmpty(ASSIMP_LIB_DIR) {
    message(assimp OFF)
} else {
    !versionAtLeast(OCC_VERSION_STR, 7.5.0) {
        message(assimp reader disabled because OpenCascade < v7.5)
    }
    else {
        message(assimp ON)
        ASSIMP_IS_ON = 1
    }
}

defined(ASSIMP_IS_ON, var) {
    HEADERS += $$files($$MAYO_DIR/src/io_assimp/*.h)
    SOURCES += $$files($$MAYO_DIR/src/io_assimp/*.cpp)

    ASSIMP_VERSION_FILE_CONTENTS = $$cat($$ASSIMP_INC_DIR/version.h, lines)
    ASSIMP_aiGetVersionPatch = $$find(ASSIMP_VERSION_FILE_CONTENTS, aiGetVersionPatch)
    !isEmpty(ASSIMP_aiGetVersionPatch) {
    } else {
        DEFINES += NO_ASSIMP_aiGetVersionPatch
        message(Assimp function aiGetVersionPatch() not available)
    }

    INCLUDEPATH += $$ASSIMP_INC_DIR/..
    LIBS += -L$$ASSIMP_LIB_DIR -lassimp$$ASSIMP_LIBNAME_SUFFIX
    DEFINES += HAVE_ASSIMP
}

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

    HEADERS += $$files($$MAYO_DIR/src/io_gmio/*.h)
    SOURCES += $$files($$MAYO_DIR/src/io_gmio/*.cpp)

    INCLUDEPATH += $$GMIO_ROOT/include
    LIBS += -L$$GMIO_ROOT/lib -lgmio_static -lzlibstatic
    SOURCES += $$GMIO_ROOT/src/gmio_support/stream_qt.cpp
    DEFINES += HAVE_GMIO
}

# Unit tests
CONFIG(withtests) {
    include($$MAYO_DIR/tests/tests.pri)
    DEFINES += MAYO_WITH_TESTS
}

# Optional developer-specific QMake pri file for custom processing
exists($$PWD/custom.pri) {
    include($$PWD/custom.pri)
}
