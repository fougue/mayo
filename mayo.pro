TARGET = mayo
TEMPLATE = app

QT += core gui widgets

*msvc* {
    QMAKE_CXXFLAGS += /we4150 # Deletion of pointer to incomplete type 'XXXX'; no destructor called
}

CONFIG += console

HEADERS += \
    src/document.h \
    src/document_item.h \
    src/mainwindow.h \
    src/occt_window.h \
    src/qt_occ_view_controller.h \
    src/fougtools/qttools/gui/gui.h \
    src/fougtools/qttools/gui/qwidget_utils.h \
    src/fougtools/qttools/gui/item_view_utils.h \
    src/fougtools/occtools/occtools.h \
    src/fougtools/occtools/qt_utils.h \
    src/options.h \
    src/application.h \
    src/property.h \
    src/property_enumeration.h \
    src/property_builtins.h \
    src/gui_document.h \
    src/gui_application.h \
    src/gpx_document_item.h \
    src/dialog_about.h \
    src/dialog_options.h \
    src/dialog_task_manager.h \
    src/dialog_save_image_view.h \
    src/widget_application_tree.h \
    src/widget_document_item_props.h \
    src/widget_message_indicator.h \
    src/widget_gui_document_view3d.h \
    src/widget_occ_view.h \
    src/dialog_export_options.h \
    src/mesh_utils.h \
    src/dialog_inspect_xde.h \
    src/caf_utils.h \
    src/xde_document_item.h \
    src/gpx_xde_document_item.h \
    src/fougtools/qttools/core/qstring_hfunc.h \
    src/xde_shape_explorer.h \
    src/mesh_item.h \
    src/gpx_mesh_item.h \
    src/button_view3d.h \
    src/widget_clip_planes.h \
    src/span.h \
    src/bnd_utils.h \
    src/string_utils.h

SOURCES += \
    src/document.cpp \
    src/document_item.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/occt_window.cpp \
    src/qt_occ_view_controller.cpp \
    src/fougtools/qttools/gui/item_view_utils.cpp \
    src/fougtools/qttools/gui/qwidget_utils.cpp \
    src/fougtools/occtools/qt_utils.cpp \
    src/options.cpp \
    src/application.cpp \
    src/property.cpp \
    src/property_enumeration.cpp \
    src/gui_document.cpp \
    src/gui_application.cpp \
    src/gpx_document_item.cpp \
    src/dialog_about.cpp \
    src/dialog_options.cpp \
    src/dialog_task_manager.cpp \
    src/dialog_save_image_view.cpp \
    src/widget_application_tree.cpp \
    src/widget_document_item_props.cpp \
    src/widget_message_indicator.cpp \
    src/widget_gui_document_view3d.cpp \
    src/widget_occ_view.cpp \
    src/dialog_export_options.cpp \
    src/mesh_utils.cpp \
    src/dialog_inspect_xde.cpp \
    src/caf_utils.cpp \
    src/xde_document_item.cpp \
    src/gpx_xde_document_item.cpp \
    src/xde_shape_explorer.cpp \
    src/mesh_item.cpp \
    src/gpx_mesh_item.cpp \
    src/button_view3d.cpp \
    src/widget_clip_planes.cpp \
    src/bnd_utils.cpp \
    src/string_utils.cpp

include(src/fougtools/qttools/task/qttools_task.pri)
include(src/qt-solutions/qtpropertybrowser/src/qtpropertybrowser.pri)
INCLUDEPATH += src/qt-solutions/qtpropertybrowser/src

FORMS += \
    src/mainwindow.ui \
    src/dialog_about.ui \
    src/dialog_options.ui \
    src/dialog_task_manager.ui \
    src/dialog_save_image_view.ui \
    src/widget_application_tree.ui \
    src/widget_document_item_props.ui \
    src/dialog_export_options.ui \
    src/dialog_inspect_xde.ui \
    src/widget_clip_planes.ui

# gmio
#isEmpty(GMIO_ROOT):error(Variable GMIO_ROOT is empty)
isEmpty(GMIO_ROOT) {
    warning(gmio is disabled)
} else {
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
isEmpty(CASCADE_ROOT):error(Variable CASCADE_ROOT is empty)
include(occ.pri)
LIBS += -lTKernel -lTKMath -lTKTopAlgo -lTKV3d -lTKOpenGl -lTKService
LIBS += -lTKG2d
LIBS += -lTKBRep -lTKSTL
LIBS += -lTKXSBase -lTKIGES -lTKSTEP -lTKXDESTEP -lTKXDEIGES
LIBS += -lTKMeshVS -lTKXSDRAW
LIBS += -lTKLCAF -lTKXCAF
LIBS += -lTKG3d
LIBS += -lTKGeomBase

OCCT_DEFINES = $$(CSF_DEFINES)
DEFINES += $$split(OCCT_DEFINES, ;)
DEFINES += OCCT_HANDLE_NOCAST
RESOURCES += mayo.qrc
