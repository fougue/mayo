TARGET = mayo
TEMPLATE = app

QT += core gui widgets

CONFIG += c++17

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

INCLUDEPATH += \
    src/app \
    src/3rdparty

HEADERS += \
    src/3rdparty/fougtools/occtools/occtools.h \
    src/3rdparty/fougtools/occtools/qt_utils.h \
    src/3rdparty/fougtools/qttools/core/qstring_hfunc.h \
    src/3rdparty/fougtools/qttools/gui/gui.h \
    src/3rdparty/fougtools/qttools/gui/item_view_buttons.h \
    src/3rdparty/fougtools/qttools/gui/item_view_utils.h \
    src/3rdparty/fougtools/qttools/gui/proxy_styled_item_delegate.h \
    src/3rdparty/fougtools/qttools/gui/qwidget_utils.h \
    \
    src/app/button_flat.h \
    src/app/dialog_about.h \
    src/app/dialog_export_options.h \
    src/app/dialog_inspect_xde.h \
    src/app/dialog_options.h \
    src/app/dialog_save_image_view.h \
    src/app/dialog_task_manager.h \
    src/app/mainwindow.h \
    src/app/occt_window.h \
    src/app/settings.h \
    src/app/settings_keys.h \
    src/app/theme.h \
    src/app/widget_clip_planes.h \
    src/app/widget_file_system.h \
    src/app/widget_gui_document.h \
    src/app/widget_message_indicator.h \
    src/app/widget_model_tree.h \
    src/app/widget_model_tree_builder.h \
    src/app/widget_model_tree_builder_mesh.h \
    src/app/widget_model_tree_builder_xde.h \
    src/app/widget_occ_view.h \
    src/app/widget_occ_view_controller.h \
    src/app/widget_properties_editor.h \
    src/app/widget_shape_selector.h \
    \
    src/base/application.h \
    src/base/application_item.h \
    src/base/application_item_selection_model.h \
    src/base/bnd_utils.h \
    src/base/brep_utils.h \
    src/base/caf_utils.h \
    src/base/document.h \
    src/base/document_item.h \
    src/base/geom_utils.h \
    src/base/libtree.h \
    src/base/math_utils.h \
    src/base/mesh_item.h \
    src/base/mesh_utils.h \
    src/base/occt_enums.h \
    src/base/property.h \
    src/base/property_builtins.h \
    src/base/property_enumeration.h \
    src/base/qmeta_gp_pnt.h \
    src/base/qmeta_gp_trsf.h \
    src/base/qmeta_quantity.h \
    src/base/qmeta_quantity_color.h \
    src/base/qmeta_tdf_label.h \
    src/base/quantity.h \
    src/base/result.h \
    src/base/span.h \
    src/base/string_utils.h \
    src/base/unit.h \
    src/base/unit_system.h \
    src/base/xde_document_item.h \
    src/base/xde_shape_property_owner.h \
    \
    src/gpx/ais_text.h \
    src/gpx/gpx_document_item.h \
    src/gpx/gpx_document_item_factory.h \
    src/gpx/gpx_mesh_item.h \
    src/gpx/gpx_utils.h \
    src/gpx/gpx_xde_document_item.h \
    src/gpx/v3d_view_camera_animation.h \
    src/gpx/v3d_view_controller.h \
    \
    src/gui/gui_application.h \
    src/gui/gui_document.h \
    src/gui/gui_document_list_model.h \
    src/gui/qtgui_utils.h \

SOURCES += \
    src/3rdparty/fougtools/occtools/qt_utils.cpp \
    src/3rdparty/fougtools/qttools/gui/item_view_buttons.cpp \
    src/3rdparty/fougtools/qttools/gui/item_view_utils.cpp \
    src/3rdparty/fougtools/qttools/gui/proxy_styled_item_delegate.cpp \
    src/3rdparty/fougtools/qttools/gui/qwidget_utils.cpp \
    \
    src/app/button_flat.cpp \
    src/app/dialog_about.cpp \
    src/app/dialog_export_options.cpp \
    src/app/dialog_inspect_xde.cpp \
    src/app/dialog_options.cpp \
    src/app/dialog_save_image_view.cpp \
    src/app/dialog_task_manager.cpp \
    src/app/main.cpp \
    src/app/mainwindow.cpp \
    src/app/occt_window.cpp \
    src/app/settings.cpp \
    src/app/theme.cpp \
    src/app/widget_clip_planes.cpp \
    src/app/widget_file_system.cpp \
    src/app/widget_gui_document.cpp \
    src/app/widget_message_indicator.cpp \
    src/app/widget_model_tree.cpp \
    src/app/widget_model_tree_builder.cpp \
    src/app/widget_model_tree_builder_mesh.cpp \
    src/app/widget_model_tree_builder_xde.cpp \
    src/app/widget_occ_view.cpp \
    src/app/widget_occ_view_controller.cpp \
    src/app/widget_properties_editor.cpp \
    src/app/widget_shape_selector.cpp \
    \
    src/base/application.cpp \
    src/base/application_item.cpp \
    src/base/application_item_selection_model.cpp \
    src/base/bnd_utils.cpp \
    src/base/brep_utils.cpp \
    src/base/caf_utils.cpp \
    src/base/document.cpp \
    src/base/document_item.cpp \
    src/base/geom_utils.cpp \
    src/base/math_utils.cpp \
    src/base/mesh_item.cpp \
    src/base/mesh_utils.cpp \
    src/base/occt_enums.cpp \
    src/base/property.cpp \
    src/base/property_builtins.cpp \
    src/base/property_enumeration.cpp \
    src/base/quantity.cpp \
    src/base/result.cpp \
    src/base/string_utils.cpp \
    src/base/unit.cpp \
    src/base/unit_system.cpp \
    src/base/xde_document_item.cpp \
    src/base/xde_shape_property_owner.cpp \
    \
    src/gpx/ais_text.cpp \
    src/gpx/gpx_document_item.cpp \
    src/gpx/gpx_document_item_factory.cpp \
    src/gpx/gpx_mesh_item.cpp \
    src/gpx/gpx_utils.cpp \
    src/gpx/gpx_xde_document_item.cpp \
    src/gpx/v3d_view_camera_animation.cpp \
    src/gpx/v3d_view_controller.cpp \
    \
    src/gui/gui_application.cpp \
    src/gui/gui_document.cpp \
    src/gui/gui_document_list_model.cpp \
    src/gui/qtgui_utils.cpp \

win* {
    QT += winextras
    HEADERS += src/app/win_taskbar_global_progress.h
    SOURCES += src/app/win_taskbar_global_progress.cpp
}

include(src/3rdparty/fougtools/qttools/task/qttools_task.pri)

FORMS += \
    src/app/dialog_about.ui \
    src/app/dialog_export_options.ui \
    src/app/dialog_inspect_xde.ui \
    src/app/dialog_options.ui \
    src/app/dialog_save_image_view.ui \
    src/app/dialog_task_manager.ui \
    src/app/mainwindow.ui \
    src/app/widget_clip_planes.ui \
    src/app/widget_model_tree.ui \
    src/app/widget_properties_editor.ui \

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
