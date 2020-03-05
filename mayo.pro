TARGET = mayo
TEMPLATE = app

QT += core gui widgets

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

include(version.pri)
CONFIG(debug, debug|release) {
    message(Mayo version $$MAYO_VERSION debug)
} else {
    message(Mayo version $$MAYO_VERSION release)
}

win* {
    COPIES += WinInstallerFiles
    WinInstallerFiles.files  = $$files($$PWD/installer/*.iss)
    WinInstallerFiles.files += $$files($$PWD/installer/*.conf)
    WinInstallerFiles.path = $$OUT_PWD/installer
}

INCLUDEPATH += \
    src/app \
    src/3rdparty

HEADERS += \
    src/3rdparty/fougtools/qttools/gui/item_view_buttons.h \
    src/3rdparty/fougtools/qttools/gui/proxy_styled_item_delegate.h \
    \
    $$files(src/base/*.h) \
    $$files(src/gpx/*.h) \
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
    $$files(src/gpx/*.cpp) \
    $$files(src/gui/*.cpp) \
    $$files(src/app/*.cpp) \

win* {
    QT += winextras
    HEADERS += $$files(src/app/windows/*.h)
    SOURCES += $$files(src/app/windows/*.cpp)
}

include(src/3rdparty/fougtools/qttools/task/qttools_task.pri)

FORMS += $$files(src/app/*.ui)

RESOURCES += mayo.qrc
RC_ICONS = images/appicon.ico

OTHER_FILES += \
    README.md \
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
LIBS += -lTKernel -lTKMath -lTKTopAlgo -lTKV3d -lTKOpenGl -lTKService
LIBS += -lTKG2d
LIBS += -lTKBRep -lTKSTL
LIBS += -lTKXSBase -lTKIGES -lTKSTEP -lTKXDESTEP -lTKXDEIGES
LIBS += -lTKMeshVS -lTKXSDRAW
LIBS += -lTKLCAF -lTKXCAF -lTKCAF
LIBS += -lTKG3d
LIBS += -lTKGeomBase
