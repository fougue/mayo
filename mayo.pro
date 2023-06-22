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
!versionAtLeast(QT_VERSION, 5.14) {
    error(Qt >= 5.14 is required but detected version is $$QT_VERSION)
}

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
macx {
    DEFINES += GL_SILENCE_DEPRECATION
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.15
    LIBS += -liconv
#   QMAKE_CXXFLAGS += -mmacosx-version-min=10.15
    ICON = images/appicon.icns
}
win32 {
    LIBS += -lOpengl32 -lUser32
}

INCLUDEPATH += \
    src/app \
    src/3rdparty

HEADERS += \
    $$files(src/base/*.h) \
    $$files(src/io_dxf/*.h) \
    $$files(src/io_image/*.h) \
    $$files(src/io_occ/*.h) \
    $$files(src/io_off/*.h) \
    $$files(src/io_ply/*.h) \
    $$files(src/graphics/*.h) \
    $$files(src/gui/*.h) \
    $$files(src/measure/*.h) \
    $$files(src/app/*.h) \

SOURCES += \
    $$files(src/base/*.cpp) \
    $$files(src/io_dxf/*.cpp) \
    $$files(src/io_image/*.cpp) \
    $$files(src/io_occ/*.cpp) \
    $$files(src/io_off/*.cpp) \
    $$files(src/io_ply/*.cpp) \
    $$files(src/graphics/*.cpp) \
    $$files(src/gui/*.cpp) \
    $$files(src/measure/*.cpp) \
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
    .github/workflows/ci_linux.yml \
    .github/workflows/ci_macos.yml \
    .github/workflows/ci_windows.yml \
    images/credits.txt \
    scripts/bump-version.rb \

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

versionAtLeast(OCC_VERSION_STR, 7.7.0) {
    LIBS += -lTKXDE
}

# -- IGES support
LIBS += -lTKIGES -lTKXDEIGES
# -- STEP support
LIBS += -lTKSTEP -lTKSTEP209 -lTKSTEPAttr -lTKSTEPBase -lTKXDESTEP
# -- STL support
LIBS += -lTKSTL
# -- OBJ/glTF support
versionAtLeast(OCC_VERSION_STR, 7.4.0) {
    LIBS += -lTKRWMesh
} else {
    SOURCES -= \
        src/io_occ/io_occ_base_mesh.cpp \
        src/io_occ/io_occ_gltf_reader.cpp \
        src/io_occ/io_occ_obj_reader.cpp

    message(glTF reader disabled because OpenCascade < v7.4)
    message(OBJ reader disabled because OpenCascade < v7.4)
}

!versionAtLeast(OCC_VERSION_STR, 7.5.0) {
    SOURCES -= src/io_occ/io_occ_gltf_writer.cpp
    message(glTF writer disabled because OpenCascade < v7.5)
}

!versionAtLeast(OCC_VERSION_STR, 7.6.0) {
    SOURCES -= src/io_occ/io_occ_obj_writer.cpp
    message(OBJ writer disabled because OpenCascade < v7.6)
}
# -- VRML support
LIBS += -lTKVRML
!versionAtLeast(OCC_VERSION_STR, 7.7.0) {
    SOURCES -= src/io_occ/io_occ_vrml_reader.cpp
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
    HEADERS += $$files(src/io_assimp/*.h)
    SOURCES += $$files(src/io_assimp/*.cpp)

    ASSIMP_VERSION_FILE_CONTENTS = $$cat($$ASSIMP_INC_DIR/version.h, lines)
    ASSIMP_aiGetVersionPatch = $$find(ASSIMP_VERSION_FILE_CONTENTS, aiGetVersionPatch)
    !isEmpty(ASSIMP_aiGetVersionPatch) {
    } else {
        DEFINES += NO_ASSIMP_aiGetVersionPatch
        message(Assimp function aiGetVersionPatch() not available)
    }

    INCLUDEPATH += $$ASSIMP_INC_DIR/..
    LIBS += -L$$ASSIMP_LIB_DIR
    win32 {
        CONFIG(debug, debug|release) {
            LIBS += -lassimp-vc142-mtd -lzlibstaticd
        } else {
            LIBS +=-lassimp-vc142-mtd -lzlibstatic
        }
    }
    else {
        LIBS += -lassimp
    }

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

    HEADERS += $$files(src/io_gmio/*.h)
    SOURCES += $$files(src/io_gmio/*.cpp)

    INCLUDEPATH += $$GMIO_ROOT/include
    LIBS += -L$$GMIO_ROOT/lib -lgmio_static -lzlibstatic
    SOURCES += $$GMIO_ROOT/src/gmio_support/stream_qt.cpp
    DEFINES += HAVE_GMIO
}

# Unit tests
CONFIG(withtests) {
    include(tests/tests.pri)
    DEFINES += MAYO_WITH_TESTS
}

# Optional developer-specific QMake pri file for custom processing
exists($$PWD/custom.pri) {
    include($$PWD/custom.pri)
}
