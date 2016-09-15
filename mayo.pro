TARGET = mayo
TEMPLATE = app

QT += core gui widgets

*msvc* {
    QMAKE_CXXFLAGS += /we4150 # Deletion of pointer to incomplete type 'XXXX'; no destructor called
}

HEADERS += \
    src/brep_shape_item.h \
    src/document.h \
    src/document_item.h \
    src/fougtools/qttools/gui/qwidget_utils.h \
    src/mainwindow.h \
    src/occt_window.h \
    src/qt_occ_view_controller.h \
    src/stl_mesh_item.h \
    src/fougtools/qttools/gui/gui.h \
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
    src/gpx_brep_shape_item.h \
    src/gpx_stl_mesh_item.h \
    src/dialog_about.h \
    src/dialog_options.h \
    src/dialog_task_manager.h \
    src/dialog_save_image_view.h \
    src/widget_application_tree.h \
    src/widget_document_item_props.h \
    src/widget_message_indicator.h \
    src/widget_gui_document_view3d.h \
    src/widget_occ_view.h

SOURCES += \
    src/brep_shape_item.cpp \
    src/document.cpp \
    src/document_item.cpp \
    src/fougtools/qttools/gui/qwidget_utils.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/occt_window.cpp \
    src/qt_occ_view_controller.cpp \
    src/stl_mesh_item.cpp \
    src/fougtools/qttools/gui/item_view_utils.cpp \
    src/fougtools/occtools/qt_utils.cpp \
    src/options.cpp \
    src/application.cpp \
    src/property.cpp \
    src/property_enumeration.cpp \
    src/gui_document.cpp \
    src/gui_application.cpp \
    src/gpx_document_item.cpp \
    src/gpx_brep_shape_item.cpp \
    src/gpx_stl_mesh_item.cpp \
    src/dialog_about.cpp \
    src/dialog_options.cpp \
    src/dialog_task_manager.cpp \
    src/dialog_save_image_view.cpp \
    src/widget_application_tree.cpp \
    src/widget_document_item_props.cpp \
    src/widget_message_indicator.cpp \
    src/widget_gui_document_view3d.cpp \
    src/widget_occ_view.cpp

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
    src/widget_document_item_props.ui

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
