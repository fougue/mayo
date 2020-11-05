#****************************************************************************
#* Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
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

QT += core gui widgets
message(Qt version $$QT_VERSION)

CONFIG += c++17
CONFIG += file_copies
CONFIG(debug, debug|release) {
    CONFIG += console
} else {
    CONFIG -= console
    CONFIG += release_with_debuginfo
}

release_with_debuginfo:*msvc* {
    # https://docs.microsoft.com/en-us/cpp/build/reference/how-to-debug-a-release-build
    QMAKE_CXXFLAGS_RELEASE += /Zi
    QMAKE_LFLAGS_RELEASE += /DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF
}

*msvc* {
    QMAKE_CXXFLAGS += /we4150 # Deletion of pointer to incomplete type 'XXXX'; no destructor called
    QMAKE_CXXFLAGS += /std:c++17
}
*g++* {
    QMAKE_CXXFLAGS += -std=c++17
}

INCLUDEPATH += \
    src/app \
    src/3rdparty

HEADERS += \
    src/3rdparty/fougtools/qttools/gui/item_view_buttons.h \
    src/3rdparty/fougtools/qttools/gui/proxy_styled_item_delegate.h \
    \
    $$files(src/base/*.h) \
    $$files(src/graphics/*.h) \
    $$files(src/gui/*.h) \
    $$files(src/app/*.h) \

SOURCES += \
    src/3rdparty/fougtools/occtools/qt_utils.cpp \
    src/3rdparty/fougtools/qttools/gui/item_view_buttons.cpp \
    src/3rdparty/fougtools/qttools/gui/item_view_utils.cpp \
    src/3rdparty/fougtools/qttools/gui/proxy_styled_item_delegate.cpp \
    src/3rdparty/fougtools/qttools/gui/qwidget_utils.cpp \
    \
    $$files(src/base/*.cpp) \
    $$files(src/graphics/*.cpp) \
    $$files(src/gui/*.cpp) \
    $$files(src/app/*.cpp) \

win* {
    QT += winextras
    HEADERS += $$files(src/app/windows/*.h)
    SOURCES += $$files(src/app/windows/*.cpp)

    COPIES += WinInstallerFiles
    WinInstallerFiles.files  = $$files($$PWD/installer/*.iss)
    WinInstallerFiles.files += $$files($$PWD/installer/*.conf)
    WinInstallerFiles.path = $$OUT_PWD/installer
}

FORMS += $$files(src/app/*.ui)

TRANSLATIONS += \
    i18n/mayo_en.ts \
    i18n/mayo_fr.ts

RESOURCES += mayo.qrc
RC_ICONS = images/appicon.ico

OTHER_FILES += \
    README.md \
    appveyor.yml \
    images/credits.txt

# gmio
isEmpty(GMIO_ROOT) {
    message(gmio OFF)
} else {
    message(gmio ON)
    CONFIG(debug, debug|release) {
        GMIO_BIN_SUFFIX = d
    } else {
        GMIO_BIN_SUFFIX =
    }

    INCLUDEPATH += $$GMIO_ROOT/include
    LIBS += -L$$GMIO_ROOT/lib -lgmio_static$$GMIO_BIN_SUFFIX
    SOURCES += \
        $$GMIO_ROOT/src/gmio_support/stl_occ_brep.cpp \
        $$GMIO_ROOT/src/gmio_support/stl_occ_polytri.cpp \
        $$GMIO_ROOT/src/gmio_support/stream_qt.cpp
    DEFINES += HAVE_GMIO
}

# OpenCascade
include(opencascade.pri)
message(OpenCascade version $$OCC_VERSION_STR)
LIBS += -lTKernel -lTKMath -lTKTopAlgo -lTKService
LIBS += -lTKG2d -lTKG3d -lTKV3d -lTKOpenGl
LIBS += -lTKBRep -lTKXSBase -lTKGeomBase
LIBS += -lTKMeshVS -lTKXSDRAW
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
        src/base/io_occ_base_mesh.cpp \
        src/base/io_occ_gltf.cpp \
        src/base/io_occ_obj.cpp
}
# -- VRML support
LIBS += -lTKVRML
