QT       += core gui widgets

TARGET = mayo
TEMPLATE = app

HEADERS += \
    mainwindow.h \
    qt_occ_view.h \
    qt_occ_view_controller.h \
    document.h \
    ui_document.h \
    fougtools/qttools/gui/qwidget_utils.h \
    occt_window.h \
    task_manager_dialog.h \
    document_item.h \
    brep_shape_item.h \
    stl_mesh_item.h

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    qt_occ_view.cpp \
    qt_occ_view_controller.cpp \
    document.cpp \
    ui_document.cpp \
    fougtools/qttools/gui/qwidget_utils.cpp \
    occt_window.cpp \
    task_manager_dialog.cpp \
    document_item.cpp \
    brep_shape_item.cpp \
    stl_mesh_item.cpp

include(fougtools/qttools/task/qttools_task.pri)

FORMS += \
    mainwindow.ui \
    task_manager_dialog.ui

# gmio
isEmpty(GMIO_ROOT):error(Variable GMIO_ROOT is empty)
CONFIG(debug, debug|release) {
    GMIO_BIN_SUFFIX = _d
} else {
    GMIO_BIN_SUFFIX =
}
INCLUDEPATH += $$GMIO_ROOT/include
LIBS += -L$$GMIO_ROOT/lib -lgmio$$GMIO_BIN_SUFFIX
SOURCES += \
    $$GMIO_ROOT/src/gmio_support/stl_occ_mesh.cpp \
    $$GMIO_ROOT/src/gmio_support/stream_qt.cpp

# OpenCascade
#CASCADE_ROOT=C:/dev/libs/OpenCASCADE6.9.1-vc12-64/opencascade-6.9.1
#CASCADE_ROOT=C:/dev/libs/OpenCASCADE7.0.0-vc12-64/opencascade-7.0.0
isEmpty(CASCADE_ROOT):error(Variable CASCADE_ROOT is empty)
include(occ.pri)
LIBS += -lTKernel -lTKMath -lTKTopAlgo -lTKV3d -lTKOpenGl -lTKService
LIBS += -lTKBRep -lTKSTL
LIBS += -lTKXSBase -lTKIGES -lTKSTEP
LIBS += -lTKMeshVS -lTKXSDRAW

OCCT_DEFINES = $$(CSF_DEFINES)
DEFINES += $$split(OCCT_DEFINES, ;)

RESOURCES += mayo.qrc
