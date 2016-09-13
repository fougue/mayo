TARGET = mayo
TEMPLATE = app

QT += core gui widgets

HEADERS += \
    src/brep_shape_item.h \
    src/document.h \
    src/document_item.h \
    src/fougtools/qttools/gui/qwidget_utils.h \
    src/mainwindow.h \
    src/occt_window.h \
    src/qt_occ_view.h \
    src/qt_occ_view_controller.h \
    src/stl_mesh_item.h \
    src/task_manager_dialog.h \
    src/fougtools/qttools/gui/gui.h \
    src/fougtools/qttools/gui/item_view_utils.h \
    src/fougtools/occtools/occtools.h \
    src/fougtools/occtools/qt_utils.h \
    src/message_indicator.h \
    src/about_dialog.h \
    src/options_dialog.h \
    src/options.h \
    src/save_image_view_dialog.h \
    src/application.h \
    src/property.h \
    src/property_enumeration.h \
    src/property_builtins.h \
    src/application_tree_widget.h \
    src/document_item_props_widget.h \
    src/gui_document.h \
    src/gui_document_view3d.h \
    src/gui_application.h \
    src/gpx_document_item.h \
    src/gpx_brep_shape_item.h \
    src/gpx_stl_mesh_item.h

SOURCES += \
    src/brep_shape_item.cpp \
    src/document.cpp \
    src/document_item.cpp \
    src/fougtools/qttools/gui/qwidget_utils.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/occt_window.cpp \
    src/qt_occ_view.cpp \
    src/qt_occ_view_controller.cpp \
    src/stl_mesh_item.cpp \
    src/task_manager_dialog.cpp \
    src/fougtools/qttools/gui/item_view_utils.cpp \
    src/fougtools/occtools/qt_utils.cpp \
    src/message_indicator.cpp \
    src/about_dialog.cpp \
    src/options_dialog.cpp \
    src/options.cpp \
    src/save_image_view_dialog.cpp \
    src/application.cpp \
    src/property.cpp \
    src/property_enumeration.cpp \
    src/application_tree_widget.cpp \
    src/document_item_props_widget.cpp \
    src/gui_document.cpp \
    src/gui_document_view3d.cpp \
    src/gui_application.cpp \
    src/gpx_document_item.cpp \
    src/gpx_brep_shape_item.cpp \
    src/gpx_stl_mesh_item.cpp

include(src/fougtools/qttools/task/qttools_task.pri)
include(src/qt-solutions/qtpropertybrowser/src/qtpropertybrowser.pri)
INCLUDEPATH += src/qt-solutions/qtpropertybrowser/src

FORMS += \
    src/mainwindow.ui \
    src/task_manager_dialog.ui \
    src/about_dialog.ui \
    src/options_dialog.ui \
    src/save_image_view_dialog.ui \
    src/application_tree_widget.ui \
    src/document_item_props_widget.ui

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
