#****************************************************************************
#* Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
#* All rights reserved.
#* See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
#****************************************************************************

cmake_minimum_required(VERSION 3.10)

set(Mayo_VersionMajor 0)
set(Mayo_VersionMinor 10)
set(Mayo_VersionPatch 0)
set(Mayo_Version ${Mayo_VersionMajor}.${Mayo_VersionMinor}.${Mayo_VersionPatch})

project(
    mayo
    VERSION ${Mayo_Version}
    DESCRIPTION "Mayo - Opensource 3D CAD viewer and converter"
    LANGUAGES CXX
)

set(Mayo_Copyright "Copyright @ 2016-present, Fougue Ltd., All Rights Reserved")
mark_as_advanced(FORCE Mayo_Copyright)

set(CMAKE_EXECUTE_PROCESS_COMMAND_ECHO STDOUT)

# Find git commit hash and revision number
set(Mayo_VersionCommit 0)
set(Mayo_VersionRevNum 0)

find_package(Git)
if(Git_FOUND)
    # Maybe use preferrably add_custom_command() with PRE_BUILD event?
    message(STATUS "GIT_EXECUTABLE=${GIT_EXECUTABLE}")
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        OUTPUT_VARIABLE Mayo_VersionCommit
    )
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-list --count HEAD
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        OUTPUT_VARIABLE Mayo_VersionRevNum
    )
    # Fix newline character at the end of Mayo_VersionCommit and Mayo_VersionRevNum variables
    string(REGEX REPLACE "\n$" "" Mayo_VersionCommit "${Mayo_VersionCommit}")
    string(REGEX REPLACE "\n$" "" Mayo_VersionRevNum "${Mayo_VersionRevNum}")
    # execute_process() may have failed, so make sure Mayo_Version[Commit/RevNum] are not empty strings
    # Note: execute_process() could fail in the typical case where Mayo source code was extracted
    #       from ZIP archive file(ie not via "git clone")
    if(Mayo_VersionCommit STREQUAL "")
        set(Mayo_VersionCommit 0)
    endif()
    if(Mayo_VersionRevNum STREQUAL "")
        set(Mayo_VersionRevNum 0)
    endif()
endif()

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

# Find bit size of the target machine
math(EXPR Mayo_TargetArchBitSize "8 * ${CMAKE_SIZEOF_VOID_P}")

##########
# Options
##########

option(Mayo_BuildTests "Build Mayo testing suite" OFF)
option(Mayo_BuildPluginAssimp "Build plugin to import/export mesh files supported by Assimp" OFF)
if(WIN32)
    set(Mayo_PostBuildCopyRuntimeDLLs_DefaultValue OFF)
    if(CMAKE_GENERATOR MATCHES "Visual Studio")
        set(Mayo_PostBuildCopyRuntimeDLLs_DefaultValue ON)
    endif()

    message(STATUS "Mayo_PostBuildCopyRuntimeDLLs_DefaultValue=${Mayo_PostBuildCopyRuntimeDLLs_DefaultValue}")
    option(
        Mayo_PostBuildCopyRuntimeDLLs
        "\
Add post build step to copy all runtime DLLs alongside Mayo executable.\n\
This simplifies running Mayo from eg Visual Studio IDE\
"
        ${Mayo_PostBuildCopyRuntimeDLLs_DefaultValue}
    )
endif()

option(Mayo_BuildApp "Build Mayo GUI application" ON)
option(Mayo_BuildConvCli "Build Mayo CLI converter" ON)

# TODO
# option(Mayo_BuildPluginGmio "Build plugin to import/export mesh files supported by gmio" OFF)

##########
# Qt
##########

find_package(QT NAMES Qt6 Qt5 REQUIRED)
if(QT_FOUND)
    message(STATUS "Qt version ${QT_VERSION}")
    if(QT_VERSION VERSION_LESS 5.12)
        message(FATAL_ERROR "Qt >= 5.12 is required but detected version is ${QT_VERSION}")
    endif()

    set("Qt${QT_VERSION_MAJOR}_DIR" "${QT_DIR}")

    if(Mayo_BuildApp OR Mayo_BuildConvCli)
        find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)
    endif()

    if(Mayo_BuildApp)
        find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui Widgets Test)
        if(WIN32 AND QT_VERSION_MAJOR EQUAL 5)
            find_package(Qt5 REQUIRED COMPONENTS WinExtras)
        elseif(QT_VERSION_MAJOR EQUAL 6)
            find_package(Qt6 REQUIRED COMPONENTS OpenGLWidgets)
        endif()

        if(Mayo_PostBuildCopyRuntimeDLLs)
            find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Svg)
        endif()
    endif()
endif()

#qt_standard_project_setup()
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

##########
# Compiler config
##########

set(CMAKE_CXX_STANDARD 17)

set(
    Mayo_CompileDefinitions
    QT_DISABLE_DEPRECATED_BEFORE=0x050F00
    QT_IMPLICIT_QFILEINFO_CONSTRUCTION
)
set(Mayo_CompileOptions)

if(MSVC)
    list(APPEND Mayo_CompileDefinitions NOMINMAX)
    # Deletion of pointer to incomplete type 'XXXX'; no destructor called
    list(APPEND Mayo_CompileOptions /we4150)
    # Specify both the source character set and the execution character set as UTF-8
    list(APPEND Mayo_CompileOptions /utf-8)
    if(QT_VERSION_MAJOR EQUAL 6)
        list(APPEND Mayo_CompileDefinitions _USE_MATH_DEFINES)
    endif()
elseif(APPLE)
    list(APPEND Mayo_CompileDefinitions GL_SILENCE_DEPRECATION)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    list(
        APPEND Mayo_CompileOptions
        -Wno-undefined-var-template
        -Wno-deprecated-declarations
        -Wno-macro-redefined
    )
endif()

if(Mayo_TargetArchBitSize STREQUAL "32")
    if(APPLE)
        list(APPEND Mayo_CompileDefinitions _DARWIN_USE_64_BIT_INODE)
    elseif(UNIX)
        list(APPEND Mayo_CompileDefinitions _FILE_OFFSET_BITS=64  _LARGEFILE64_SOURCE=1)
    endif()

    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm")
        # See:
        #     https://stackoverflow.com/questions/48149323/what-does-the-gcc-warning-project-parameter-passing-for-x-changed-in-gcc-7-1-m
        #     https://stackoverflow.com/questions/52020305/what-exactly-does-gccs-wpsabi-option-do-what-are-the-implications-of-supressi
        list(APPEND Mayo_CompileOptions -Wno-psabi)
    endif()
endif()

##########
# Linker config
##########

set(MayoCore_LinkDirectories)
set(MayoCore_LinkLibraries)
set(MayoIO_LinkLibraries)

if(MSVC)
    list(APPEND MayoCore_LinkLibraries Opengl32 User32)
elseif(APPLE)
    list(APPEND MayoCore_LinkLibraries iconv)
elseif(HAIKU)
    list(APPEND MayoCore_LinkLibraries iconv)
endif()

##########
# Set "include" directories
##########

set(
    Mayo_IncludeDirectories
    src/3rdparty
    ${CMAKE_BINARY_DIR}
)

##########
# MayoCore+MayoIO source files
##########

file(
    GLOB MayoCore_SourceFiles
    ${PROJECT_SOURCE_DIR}/src/base/*.cpp
    ${PROJECT_SOURCE_DIR}/src/graphics/*.cpp
    ${PROJECT_SOURCE_DIR}/src/gui/*.cpp
    ${PROJECT_SOURCE_DIR}/src/3rdparty/fmt/src/format.cc
)

file(
    GLOB MayoIO_SourceFiles
    ${PROJECT_SOURCE_DIR}/src/io_dxf/*.cpp
    ${PROJECT_SOURCE_DIR}/src/io_image/*.cpp
    ${PROJECT_SOURCE_DIR}/src/io_occ/*.cpp
    ${PROJECT_SOURCE_DIR}/src/io_off/*.cpp
    ${PROJECT_SOURCE_DIR}/src/io_ply/*.cpp
)

##########
# MayoCore+MayoIO header files
##########

file(
    GLOB MayoCore_HeaderFiles
    ${PROJECT_SOURCE_DIR}/src/base/*.h
    ${PROJECT_SOURCE_DIR}/src/graphics/*.h
    ${PROJECT_SOURCE_DIR}/src/gui/*.h
)

file(
    GLOB MayoIO_HeaderFiles
    ${PROJECT_SOURCE_DIR}/src/io_dxf/*.h
    ${PROJECT_SOURCE_DIR}/src/io_image/*.h
    ${PROJECT_SOURCE_DIR}/src/io_occ/*.h
    ${PROJECT_SOURCE_DIR}/src/io_off/*.h
    ${PROJECT_SOURCE_DIR}/src/io_ply/*.h
)


##########
# OpenCascade
##########

if(WIN32)
    # Can't use find_package(OpenCASCADE) as OpenCASCADEConfig.cmake is broken for Windows platform
    # See https://dev.opencascade.org/content/3rdparty-hard-coded-paths-when-building-against-opencascade
    include(${PROJECT_SOURCE_DIR}/cmake/OpenCascadeWin.cmake)
    if (OpenCASCADE_FOUND)
        # Add OpenCASCADE include directory
        list(APPEND Mayo_IncludeDirectories ${OpenCASCADE_INCLUDE_DIR})

        # Add OpenCASCADE library directories
        # Note: QtCreator option "Run->Add build library search to PATH" will add to PATH env variable
        #       the contents of MayoCore_LinkDirectories variable. For convenience, let's add also the
        #       directories containing DLLs
        list(
            APPEND MayoCore_LinkDirectories
            ${OpenCASCADE_BINARY_DIR}
            ${OpenCASCADE_LIBRARY_DIR}
            ${OpenCASCADE_3RDPARTY_BINARY_DIRS}
        )    
     endif()
else()
    find_package(OpenCASCADE REQUIRED)
    if(OpenCASCADE_FOUND)
        # It seems that OpenCascade cmake package files specify INTERFACE_INCLUDE_DIRECTORIES
        # starting from 7.7.0 version only
        # Older versions require to explicitly add OpenCascade include dir
        get_target_property(OpenCASCADE_TKernel_INCLUDE_DIR TKernel INTERFACE_INCLUDE_DIRECTORIES)
        list(FIND OpenCASCADE_TKernel_INCLUDE_DIR ${OpenCASCADE_INCLUDE_DIR} OpenCASCADE_TKernel_INCLUDE_DIR_IsSet)
        if(OpenCASCADE_TKernel_INCLUDE_DIR_IsSet EQUAL -1)
            list(APPEND Mayo_IncludeDirectories ${OpenCASCADE_INCLUDE_DIR})
        endif()
    endif()
endif()

list(APPEND Mayo_CompileDefinitions OCCT_HANDLE_NOCAST)
if(UNIX AND NOT APPLE)
    list(APPEND Mayo_CompileDefinitions OCC_CONVERT_SIGNALS)
endif()

if(OpenCASCADE_FOUND)
    message(STATUS "OpenCascade version ${OpenCASCADE_VERSION}")

    # std::iterator class template(used as a base class to provide typedefs) is deprecated in C++17
    # OpenCascade < 7.7.0 uses std::iterator for NCollection_StlIterator and this is causing many
    # deprecation warnings
    if(OpenCASCADE_VERSION VERSION_LESS 7.7.0)
        if(MSVC)
            # Silent warnings C4996 and STL4015
            list(APPEND Mayo_CompileDefinitions _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)
        elseif((CMAKE_CXX_COMPILER_ID MATCHES "GNU") AND (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 12))
            list(APPEND Mayo_CompileOptions -Wno-deprecated-declarations)
        endif()
    endif()

    # Add OpenCASCADE libraries
    list(
        APPEND MayoCore_LinkLibraries
        # FoundationClasses
        TKernel TKMath
        # ModelingData
        TKG2d TKG3d TKGeomBase TKBRep
        # ModelingAlgorithms
        TKBO TKBool TKGeomAlgo TKHLR TKMesh TKPrim TKShHealing TKTopAlgo
        # Visualization
        TKMeshVS TKOpenGl TKService TKV3d
        # ApplicationFramework
        TKBin TKBinL TKBinXCAF TKCAF TKCDF TKLCAF TKVCAF TKXml TKXmlL
        # DataExchange
        TKXCAF TKXmlXCAF TKXSBase
    )

    if(OpenCASCADE_VERSION VERSION_GREATER_EQUAL 7.8.0)
        list(APPEND MayoIO_LinkLibraries TKDE)
        list(APPEND MayoIO_LinkLibraries TKDEIGES)
        list(APPEND MayoIO_LinkLibraries TKDESTEP)
        list(APPEND MayoIO_LinkLibraries TKDESTL)
        list(APPEND MayoIO_LinkLibraries TKDEVRML)
    else()
        list(APPEND MayoIO_LinkLibraries TKIGES TKXDEIGES)
        list(APPEND MayoIO_LinkLibraries TKSTEP TKSTEP209 TKSTEPAttr TKSTEPBase TKXDESTEP)
        list(APPEND MayoIO_LinkLibraries TKSTL)
        list(APPEND MayoIO_LinkLibraries TKVRML)
        if(OpenCASCADE_VERSION VERSION_GREATER_EQUAL 7.7.0)
            list(APPEND MayoIO_LinkLibraries TKXDE)
        endif()
    endif()

    # OBJ/glTF support
    if(OpenCASCADE_VERSION VERSION_GREATER_EQUAL 7.4.0)
        list(APPEND MayoIO_LinkLibraries TKRWMesh)
        if(OpenCASCADE_VERSION VERSION_GREATER_EQUAL 7.8.0)
            list(APPEND MayoIO_LinkLibraries TKDEOBJ TKDEGLTF)
        endif()
    else()
        list(
            REMOVE_ITEM MayoIO_SourceFiles
            ${PROJECT_SOURCE_DIR}/src/io_occ/io_occ_base_mesh.cpp
            ${PROJECT_SOURCE_DIR}/src/io_occ/io_occ_gltf_reader.cpp
            ${PROJECT_SOURCE_DIR}/src/io_occ/io_occ_obj_reader.cpp
        )
        message(STATUS "glTF reader disabled because OpenCascade < v7.4")
        message(STATUS "OBJ reader disabled because OpenCascade < v7.4")
    endif()

    if(OpenCASCADE_VERSION VERSION_LESS 7.5.0)
        list(REMOVE_ITEM MayoIO_SourceFiles ${PROJECT_SOURCE_DIR}/src/io_occ/io_occ_gltf_writer.cpp)
        message(STATUS "glTF writer disabled because OpenCascade < v7.5")
    endif()

    if(OpenCASCADE_VERSION VERSION_LESS 7.6.0)
        list(REMOVE_ITEM MayoIO_SourceFiles ${PROJECT_SOURCE_DIR}/src/io_occ/io_occ_obj_writer.cpp)
        message(STATUS "OBJ writer disabled because OpenCascade < v7.6")
    endif()

    # VRML support
    if(OpenCASCADE_VERSION VERSION_LESS 7.7.0)
        list(REMOVE_ITEM MayoIO_SourceFiles ${PROJECT_SOURCE_DIR}/src/io_occ/io_occ_vrml_reader.cpp)
        message(STATUS "VRML reader disabled because OpenCascade < v7.7")
    endif()
endif()

##########
# Assimp plugin
##########

if(Mayo_BuildPluginAssimp)
    if(OpenCASCADE_VERSION VERSION_GREATER_EQUAL 7.5.0)
        find_package(assimp)
        if(NOT assimp_FOUND)
            message(WARNING "Assimp library not found")
        endif()
    else()
        message(STATUS "Assimp reader disabled because OpenCascade < v7.5")
    endif()
endif()

if(assimp_FOUND)
    message(STATUS "assimp version ${assimp_VERSION}")
    set(MAYO_HAVE_ASSIMP 1)

    file(GLOB MayoPluginAssimp_HeaderFiles ${PROJECT_SOURCE_DIR}/src/io_assimp/*.h)
    file(GLOB MayoPluginAssimp_SourceFiles ${PROJECT_SOURCE_DIR}/src/io_assimp/*.cpp)
    list(APPEND MayoIO_HeaderFiles ${MayoPluginAssimp_HeaderFiles})
    list(APPEND MayoIO_SourceFiles ${MayoPluginAssimp_SourceFiles})

    list(APPEND MayoIO_LinkLibraries ${ASSIMP_LIBRARIES})

    list(GET ASSIMP_INCLUDE_DIRS 0 assimp_IncludeDir)
    file(READ "${assimp_IncludeDir}/assimp/version.h" assimp_FileVersionH)
    if(assimp_FileVersionH MATCHES "aiGetVersionPatch")
        set(MAYO_HAVE_ASSIMP_aiGetVersionPatch 1)
    else()
        message(STATUS "Assimp function aiGetVersionPatch() not available")
    endif()
endif()

##########
# gmio
##########

# TODO Finalize
if(gmio_FOUND)
    message(STATUS "gmio version ${gmio_VERSION}")
    set(MAYO_HAVE_GMIO 1)

    file(GLOB MayoPluginGmio_HeaderFiles ${PROJECT_SOURCE_DIR}/src/io_gmio/*.h)
    file(GLOB MayoPluginGmio_SourceFiles ${PROJECT_SOURCE_DIR}/src/io_gmio/*.cpp)
    list(APPEND MayoIO_HeaderFiles ${MayoPluginGmio_HeaderFiles})
    list(APPEND MayoIO_SourceFiles ${MayoPluginGmio_SourceFiles})

    # Needs -L$$GMIO_ROOT/lib -lgmio_static -lzlibstatic
    list(APPEND MayoIO_LinkLibraries ${GMIO_LIBRARIES})
endif()

##########
# Configure files
##########

configure_file(${PROJECT_SOURCE_DIR}/src/mayo_config.h.cmake  common/mayo_config.h  @ONLY)
configure_file(${PROJECT_SOURCE_DIR}/src/mayo_version.h.cmake common/mayo_version.h @ONLY)

##########
# Icons for Mayo exe targets
##########

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(Mayo_AppIconMacOS images/appicon.icns)
    set_source_files_properties(images/appicon.icns PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
endif()

if(MSVC)
    set(Mayo_RcIconsWin ${PROJECT_SOURCE_DIR}/images/appicon.rc)
endif()

##########
# Target: MayoCore(static lib)
##########

add_library(MayoCore STATIC ${MayoCore_HeaderFiles} ${MayoCore_SourceFiles})

target_include_directories(MayoCore PRIVATE ${Mayo_IncludeDirectories})
target_compile_definitions(MayoCore PRIVATE ${Mayo_CompileDefinitions})
target_compile_options(MayoCore PRIVATE ${Mayo_CompileOptions})
target_link_libraries(MayoCore PRIVATE ${MayoCore_LinkLibraries})

add_library(MayoCoreLib INTERFACE)
target_link_libraries(MayoCoreLib INTERFACE MayoCore ${MayoCore_LinkLibraries})
target_link_directories(MayoCoreLib INTERFACE ${MayoCore_LinkDirectories})
target_include_directories(MayoCoreLib INTERFACE ${Mayo_IncludeDirectories})

##########
# Target: MayoIO(static lib)
##########

add_library(MayoIO STATIC ${MayoIO_HeaderFiles} ${MayoIO_SourceFiles})

target_include_directories(MayoIO PRIVATE ${Mayo_IncludeDirectories})
target_compile_definitions(MayoIO PRIVATE ${Mayo_CompileDefinitions})
target_compile_options(MayoIO PRIVATE ${Mayo_CompileOptions})
target_link_libraries(MayoIO PRIVATE MayoCore ${MayoIO_LinkLibraries})

add_library(MayoIOLib INTERFACE)
target_link_libraries(MayoIOLib INTERFACE MayoIO MayoCoreLib ${MayoIO_LinkLibraries})

##########
# Target: MayoApp
##########

set(MayoApp_HeaderFiles)
set(MayoApp_SourceFiles)
set(MayoApp_LinkLibraries)

# MayoApp headers
file(
    GLOB MayoApp_HeaderFiles
    ${PROJECT_SOURCE_DIR}/src/app/*.h
    ${PROJECT_SOURCE_DIR}/src/measure/*.h
    ${PROJECT_SOURCE_DIR}/src/qtbackend/*.h
    ${PROJECT_SOURCE_DIR}/src/qtcommon/*.h
)

if(WIN32 AND QT_VERSION_MAJOR EQUAL 5)
    list(APPEND MayoApp_HeaderFiles ${PROJECT_SOURCE_DIR}/src/app/windows/win_taskbar_global_progress.h)
endif()

# MayoApp sources
file(
    GLOB MayoApp_SourceFiles
    ${PROJECT_SOURCE_DIR}/src/app/*.cpp
    ${PROJECT_SOURCE_DIR}/src/measure/*.cpp
    ${PROJECT_SOURCE_DIR}/src/qtbackend/*.cpp
    ${PROJECT_SOURCE_DIR}/src/qtcommon/*.cpp
)

if(WIN32 AND QT_VERSION_MAJOR EQUAL 5)
    list(APPEND MayoApp_SourceFiles ${PROJECT_SOURCE_DIR}/src/app/windows/win_taskbar_global_progress.cpp)
endif()

# MayoApp UI files
file(GLOB MayoApp_QtUiFiles ${PROJECT_SOURCE_DIR}/src/app/*.ui)

# MayoApp libs
list(
    APPEND MayoApp_LinkLibraries
    MayoCoreLib
    MayoIOLib
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
)

if(WIN32 AND QT_VERSION_MAJOR EQUAL 5)
    list(APPEND MayoApp_LinkLibraries Qt5::WinExtras)
elseif(QT_VERSION_MAJOR EQUAL 6)
    list(APPEND MayoApp_LinkLibraries Qt6::OpenGLWidgets)
endif()

if(Mayo_PostBuildCopyRuntimeDLLs)
    list(APPEND MayoApp_LinkLibraries Qt${QT_VERSION_MAJOR}::Svg)
endif()

if(Mayo_BuildApp)
    add_executable(
        mayo
        ${MayoApp_SourceFiles}
        ${MayoApp_HeaderFiles}
        ${MayoApp_QtUiFiles}
        i18n/mayo.qrc
        i18n/qt${QT_VERSION_MAJOR}base.qrc
        images/mayo.qrc
        ${Mayo_RcIconsWin}
        ${Mayo_AppIconMacOS}
    )

    target_compile_definitions(mayo PRIVATE ${Mayo_CompileDefinitions})
    target_compile_options(mayo PRIVATE ${Mayo_CompileOptions})
    target_link_libraries(mayo PRIVATE ${MayoApp_LinkLibraries})
    # Required by generated Qt UI files
    target_include_directories(mayo PRIVATE ${PROJECT_SOURCE_DIR}/src/app)

    set_target_properties(
        mayo
        PROPERTIES
            MACOSX_BUNDLE_BUNDLE_NAME "mayo"
            MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
            MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
            MACOSX_BUNDLE_COPYRIGHT ${Mayo_Copyright}
            MACOSX_BUNDLE_ICON_FILE appicon.icns
            WIN32_EXECUTABLE TRUE
            MACOSX_BUNDLE TRUE
    )

    if(Mayo_PostBuildCopyRuntimeDLLs)
        # Copy required Qt plugins
        set(QtPluginsDir "${QT_DIR}/../../../plugins")
        file(GLOB QtPluginIconEnginesDLLs  "${QtPluginsDir}/iconengines/qsvgicon*.dll")
        file(GLOB QtPluginImageFormatsDLLs "${QtPluginsDir}/imageformats/qsvg*.dll")
        file(GLOB QtPluginPlatformsDLLs    "${QtPluginsDir}/platforms/qwindows*.dll")
        set(QtPluginsDLLs ${QtPluginIconEnginesDLLs} ${QtPluginImageFormatsDLLs} ${QtPluginPlatformsDLLs})
        foreach(QtPluginDLL ${QtPluginsDLLs})
            cmake_path(GET QtPluginDLL PARENT_PATH QtPluginDLL_Path)
            cmake_path(GET QtPluginDLL_Path FILENAME QtPluginDLL_PathName)
            add_custom_command(
                TARGET mayo POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:mayo>/plugins/${QtPluginDLL_PathName}"
            )
            add_custom_command(
                TARGET mayo POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${QtPluginDLL}" "$<TARGET_FILE_DIR:mayo>/plugins/${QtPluginDLL_PathName}"
            )
        endforeach()

        # Copy OpenCascade 3rd-party DLLs
        foreach(Occ3rdDLL ${OpenCASCADE_3RDPARTY_DLLS})
            add_custom_command(
                TARGET mayo POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "${Occ3rdDLL}" $<TARGET_FILE_DIR:mayo>
            )
        endforeach()

        # Copy runtime DLLs specified with library IMPORTED_LOCATION property
        add_custom_command(
            TARGET mayo POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    $<TARGET_RUNTIME_DLLS:mayo> $<TARGET_FILE_DIR:mayo>
            COMMAND_EXPAND_LISTS
        )
    endif() # Mayo_PostBuildCopyRuntimeDLLs
endif() # Mayo_BuildApp

##########
# Target: mayo-conv
##########

file(
    GLOB MayoConv_HeaderFiles
    ${PROJECT_SOURCE_DIR}/src/cli/*.h
    ${PROJECT_SOURCE_DIR}/src/qtbackend/*.h
    ${PROJECT_SOURCE_DIR}/src/qtcommon/*.h
)
list(
    APPEND MayoConv_HeaderFiles
    ${PROJECT_SOURCE_DIR}/src/app/app_module_properties.h
    ${PROJECT_SOURCE_DIR}/src/app/app_module.h
    ${PROJECT_SOURCE_DIR}/src/app/app_ui_state.h
    ${PROJECT_SOURCE_DIR}/src/app/library_info.h
    ${PROJECT_SOURCE_DIR}/src/app/recent_files.h
)

file(
    GLOB MayoConv_SourceFiles
    ${PROJECT_SOURCE_DIR}/src/cli/*.cpp
    ${PROJECT_SOURCE_DIR}/src/qtbackend/*.cpp
    ${PROJECT_SOURCE_DIR}/src/qtcommon/*.cpp
)
list(
    APPEND MayoConv_SourceFiles
    ${PROJECT_SOURCE_DIR}/src/app/app_module_properties.cpp
    ${PROJECT_SOURCE_DIR}/src/app/app_module.cpp
    ${PROJECT_SOURCE_DIR}/src/app/app_ui_state.cpp
    ${PROJECT_SOURCE_DIR}/src/app/recent_files.cpp
)

if(Mayo_BuildConvCli)
    add_executable(
        mayo-conv
        ${MayoConv_HeaderFiles}
        ${MayoConv_SourceFiles}
        i18n/mayo.qrc
        i18n/qt${QT_VERSION_MAJOR}base.qrc
        ${Mayo_RcIconsWin}
        ${Mayo_AppIconMacOS}
    )

    target_compile_definitions(mayo-conv PRIVATE ${Mayo_CompileDefinitions})
    target_compile_options(mayo-conv PRIVATE ${Mayo_CompileOptions})
    target_link_libraries(
        mayo-conv PRIVATE
        MayoCoreLib
        MayoIOLib
        Qt${QT_VERSION_MAJOR}::Core
    )

    set_target_properties(
        mayo-conv PROPERTIES WIN32_EXECUTABLE FALSE
    )
endif() # Mayo_BuildConvCli

##########
# Target: test-mayo
##########

if(Mayo_BuildTests)
    enable_testing()

    file(GLOB MayoTests_HeaderFiles ${PROJECT_SOURCE_DIR}/tests/*.h)
    file(GLOB MayoTests_SourceFiles ${PROJECT_SOURCE_DIR}/tests/*.cpp)
    add_executable(
        test-mayo
        ${MayoTests_HeaderFiles}
        ${MayoTests_SourceFiles}
        # src/measure
        ${PROJECT_SOURCE_DIR}/src/measure/measure_display.cpp
        ${PROJECT_SOURCE_DIR}/src/measure/measure_tool.cpp
        ${PROJECT_SOURCE_DIR}/src/measure/measure_tool_brep.cpp
        # src/app
        ${PROJECT_SOURCE_DIR}/src/app/app_module.cpp
        ${PROJECT_SOURCE_DIR}/src/app/app_module_properties.cpp
        ${PROJECT_SOURCE_DIR}/src/app/app_ui_state.cpp
        ${PROJECT_SOURCE_DIR}/src/app/document_files_watcher.cpp
        ${PROJECT_SOURCE_DIR}/src/app/qstring_utils.cpp
        ${PROJECT_SOURCE_DIR}/src/app/qtgui_utils.cpp
        ${PROJECT_SOURCE_DIR}/src/app/recent_files.cpp
        # src/qtcommon
        ${PROJECT_SOURCE_DIR}/src/qtcommon/qtcore_utils.cpp
    )

    target_compile_definitions(test-mayo PRIVATE ${Mayo_CompileDefinitions})
    target_compile_options(test-mayo PRIVATE ${Mayo_CompileOptions})
    target_link_libraries(
        test-mayo PRIVATE
        MayoCoreLib
        MayoIOLib
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Test
    )

    set_target_properties(
        test-mayo PROPERTIES WIN32_EXECUTABLE FALSE
    )

    file(GLOB MayoTests_InputFiles ${PROJECT_SOURCE_DIR}/tests/inputs/*.*)
    file(COPY ${MayoTests_InputFiles} DESTINATION ${CMAKE_BINARY_DIR}/tests/inputs)
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/tests/outputs)

    add_test(NAME test-mayo COMMAND test-mayo)
endif()

##########
# Target: OtherFiles
##########

add_custom_target(
    OtherFiles SOURCES
    .github/workflows/ci_linux_arch.yml
    .github/workflows/ci_linux_fedora.yml
    .github/workflows/ci_linux_ubuntu.yml
    .github/workflows/ci_linux_ubuntu_armv7.yml
    .github/workflows/ci_macos.yml
    .github/workflows/ci_macos_conda.yml
    .github/workflows/ci_windows.yml
    images/credits.txt
    scripts/bump-version.rb
    scripts/gen-cmake-opencascade-vars.bat
    src/mayo_config.h.cmake
    src/mayo_version.h.cmake
    README.md
)

##########
# Custom steps to be processed, if file ".../cmake/CustomSteps.cmake" exists
##########

if (EXISTS "${PROJECT_SOURCE_DIR}/cmake/CustomSteps.cmake")
    message(STATUS "File '${PROJECT_SOURCE_DIR}/cmake/CustomSteps.cmake' found...")
    include(${PROJECT_SOURCE_DIR}/cmake/CustomSteps.cmake)
endif()
