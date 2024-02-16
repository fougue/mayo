#****************************************************************************
#* Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
#* All rights reserved.
#* See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
#****************************************************************************

cmake_minimum_required(VERSION 3.10)

set(Mayo_VersionMajor 0)
set(Mayo_VersionMinor 9)
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

# Find git commit hash and revision number
set(Mayo_VersionCommit 0)
set(Mayo_VersionRevNum 0)

find_package(Git)
if (Git_FOUND)
    # Maybe use preferrably add_custom_command() with PRE_BUILD event?
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
endif()

configure_file(${PROJECT_SOURCE_DIR}/src/app/version.h.cmake app/version.h @ONLY)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

# Find bit size of the target machine
math(EXPR Mayo_TargetArchBitSize "8 * ${CMAKE_SIZEOF_VOID_P}")

##########
# Options
##########

option(Mayo_BuildTests "Build Mayo testing suite" OFF)
option(Mayo_BuildPluginAssimp "Build plugin to import/export mesh files supported by Assimp" OFF)
option(Mayo_BuildPluginGmio "Build plugin to import/export mesh files supported by gmio" OFF)

##########
# Qt
##########

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
if(QT_VERSION_MAJOR EQUAL 5)
    find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets Test)
    if(WIN32)
        find_package(Qt5 REQUIRED COMPONENTS WinExtras)
    endif()
elseif(QT_VERSION_MAJOR EQUAL 6)
    find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Test OpenGLWidgets)
endif()

message(STATUS "Qt version ${QT_VERSION}")
if(QT_VERSION VERSION_LESS 5.14)
    message(FATAL_ERROR "Qt >= 5.14 is required but detected version is ${QT_VERSION}")
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
    if(QT_VERSION_MAJOR EQUAL 6)
        list(APPEND Mayo_CompileDefinitions _USE_MATH_DEFINES)
    endif()
elseif(APPLE)
    list(APPEND Mayo_CompileDefinitions GL_SILENCE_DEPRECATION)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    list(
        APPEND Mayo_CompileOptions
        -Wno-undefined-var-template
        -Wno-deprecated-declarations
        -Wno-macro-redefined
    )
endif()

if (Mayo_TargetArchBitSize STREQUAL "32")
    if(APPLE)
        list(APPEND Mayo_CompileDefinitions _DARWIN_USE_64_BIT_INODE)
    elseif(UNIX)
        list(APPEND Mayo_CompileDefinitions _FILE_OFFSET_BITS=64  _LARGEFILE64_SOURCE=1)
    endif()

    if (CMAKE_SYSTEM_PROCESSOR MATCHES "arm")
        # See:
        #     https://stackoverflow.com/questions/48149323/what-does-the-gcc-warning-project-parameter-passing-for-x-changed-in-gcc-7-1-m
        #     https://stackoverflow.com/questions/52020305/what-exactly-does-gccs-wpsabi-option-do-what-are-the-implications-of-supressi
        list(APPEND Mayo_CompileOptions -Wno-psabi)
    endif()
endif()

##########
# Linker config
##########

set(Mayo_LinkLibraries)
set(Mayo_LinkDirectories)

if(QT_VERSION_MAJOR EQUAL 5)
    list(APPEND Mayo_LinkLibraries Qt5::Core Qt5::Gui Qt5::Widgets)
    if(WIN32)
        list(APPEND Mayo_LinkLibraries Qt5::WinExtras)
    endif()
elseif(QT_VERSION_MAJOR EQUAL 6)
    list(APPEND Mayo_LinkLibraries Qt6::Core Qt6::Gui Qt6::Widgets Qt6::OpenGLWidgets)
endif()

if(MSVC)
    list(APPEND Mayo_LinkLibraries Opengl32 User32)
elseif(APPLE)
    list(APPEND Mayo_LinkLibraries iconv)
endif()

##########
# Set "include" directories
##########

set(
    Mayo_IncludeDirectories
    src/app
    src/3rdparty
    ${CMAKE_BINARY_DIR}/app
)

##########
# Source files
##########

file(
    GLOB Mayo_SourceFiles
    ${PROJECT_SOURCE_DIR}/src/app/*.cpp
    ${PROJECT_SOURCE_DIR}/src/base/*.cpp
    ${PROJECT_SOURCE_DIR}/src/graphics/*.cpp
    ${PROJECT_SOURCE_DIR}/src/gui/*.cpp
    ${PROJECT_SOURCE_DIR}/src/io_dxf/*.cpp
    ${PROJECT_SOURCE_DIR}/src/io_image/*.cpp
    ${PROJECT_SOURCE_DIR}/src/io_occ/*.cpp
    ${PROJECT_SOURCE_DIR}/src/io_off/*.cpp
    ${PROJECT_SOURCE_DIR}/src/io_ply/*.cpp
    ${PROJECT_SOURCE_DIR}/src/measure/*.cpp
    ${PROJECT_SOURCE_DIR}/src/3rdparty/fmt/src/format.cc
)

if(WIN32 AND ${QT_VERSION_MAJOR} EQUAL 5)
    list(APPEND Mayo_SourceFiles ${PROJECT_SOURCE_DIR}/src/app/windows/win_taskbar_global_progress.cpp)
endif()

if(MSVC)
    set(Mayo_RcIconsWin ${PROJECT_SOURCE_DIR}/images/appicon.ico)
endif()

##########
# Header files
##########

file(
    GLOB Mayo_HeaderFiles
    ${PROJECT_SOURCE_DIR}/src/app/*.h
    ${PROJECT_SOURCE_DIR}/src/base/*.h
    ${PROJECT_SOURCE_DIR}/src/graphics/*.h
    ${PROJECT_SOURCE_DIR}/src/gui/*.h
    ${PROJECT_SOURCE_DIR}/src/io_dxf/*.h
    ${PROJECT_SOURCE_DIR}/src/io_gmio/*.h
    ${PROJECT_SOURCE_DIR}/src/io_image/*.h
    ${PROJECT_SOURCE_DIR}/src/io_occ/*.h
    ${PROJECT_SOURCE_DIR}/src/io_off/*.h
    ${PROJECT_SOURCE_DIR}/src/io_ply/*.h
    ${PROJECT_SOURCE_DIR}/src/io_measure/*.h
)

if(WIN32 AND ${QT_VERSION_MAJOR} EQUAL 5)
    list(APPEND Mayo_HeaderFiles ${PROJECT_SOURCE_DIR}/src/app/windows/win_taskbar_global_progress.h)
endif()

##########
# UI files
##########

file(
    GLOB Mayo_QtUiFiles
    ${PROJECT_SOURCE_DIR}/src/app/*.ui
)

##########
# Unit tests
##########

if(Mayo_BuildTests)
    list(
        APPEND Mayo_HeaderFiles
        ${PROJECT_SOURCE_DIR}/tests/test_app.h
        ${PROJECT_SOURCE_DIR}/tests/test_base.h
        ${PROJECT_SOURCE_DIR}/tests/test_measure.h
    )

    list(
        APPEND Mayo_SourceFiles
        ${PROJECT_SOURCE_DIR}/tests/runtests.cpp
        ${PROJECT_SOURCE_DIR}/tests/test_app.cpp
        ${PROJECT_SOURCE_DIR}/tests/test_base.cpp
        ${PROJECT_SOURCE_DIR}/tests/test_measure.cpp
    )

    list(APPEND Mayo_CompileDefinitions MAYO_WITH_TESTS)
    list(APPEND Mayo_LinkLibraries Qt${QT_VERSION_MAJOR}::Test)

    file(
        GLOB Mayo_InputTestFiles
        ${PROJECT_SOURCE_DIR}/tests/inputs/*.*
    )
    file(
        COPY ${Mayo_InputTestFiles}
        DESTINATION ${CMAKE_BINARY_DIR}/tests/inputs
    )

    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/tests/outputs)
endif()

##########
# OpenCascade
##########

if(WIN32)
    # Can't use find_package(OpenCASCADE) as OpenCASCADEConfig.cmake is broken for Windows platform
    # See https://dev.opencascade.org/content/3rdparty-hard-coded-paths-when-building-against-opencascade

    set(OpenCASCADE_DIR CACHE PATH "Path where is located OpenCASCADE env.bat file")
    execute_process(
        COMMAND cmd /c ${PROJECT_SOURCE_DIR}/scripts/gen-cmake-opencascade-vars.bat "${OpenCASCADE_DIR}"
        OUTPUT_FILE Mayo_OpenCascadeEnv.cmake
    )

    include(${CMAKE_BINARY_DIR}/Mayo_OpenCascadeEnv.cmake)
    if(OpenCASCADE_FOUND)
        # Set variables for OpenCASCADE version
        file(READ ${OpenCASCADE_INCLUDE_DIR}/Standard_Version.hxx OpenCascade_FileVersionHxx)
        string(REGEX MATCH "define[ \t]+OCC_VERSION_MAJOR[ \t]+([0-9]+)" OpenCASCADE_MAJOR_VERSION "${OpenCascade_FileVersionHxx}")
        set(OpenCASCADE_MAJOR_VERSION ${CMAKE_MATCH_1})
        string(REGEX MATCH "define[ \t]+OCC_VERSION_MINOR[ \t]+([0-9]+)" OpenCASCADE_MINOR_VERSION "${OpenCascade_FileVersionHxx}")
        set(OpenCASCADE_MINOR_VERSION ${CMAKE_MATCH_1})
        string(REGEX MATCH "define[ \t]+OCC_VERSION_MAINTENANCE[ \t]+([0-9]+)" OpenCASCADE_MAINTENANCE_VERSION "${OpenCascade_FileVersionHxx}")
        set(OpenCASCADE_MAINTENANCE_VERSION ${CMAKE_MATCH_1})
        set(OpenCASCADE_VERSION "${OpenCASCADE_MAJOR_VERSION}.${OpenCASCADE_MINOR_VERSION}.${OpenCASCADE_MAINTENANCE_VERSION}")

        # Add OpenCASCADE include directory
        list(APPEND Mayo_IncludeDirectories ${OpenCASCADE_INCLUDE_DIR})

        # Set OpenCASACADE_3RDPARTY_BINARY_DIRS variable that will contain the OpenCASCADE
        # 3rd-party binary directories(ie containing DLL files)
        set(OpenCASACADE_3RDPARTY_BINARY_DIRS)
        if(OpenCASCADE_VERSION VERSION_GREATER_EQUAL 7.8.0)
            list(
                APPEND OpenCASACADE_3RDPARTY_BINARY_DIRS
                ${TCL_DIR}
                ${TK_DIR}
                ${FREETYPE_DIR}
                ${FREEIMAGE_DIR}
                ${FFMPEG_DIR}
                ${TBB_DIR}
                ${OPENVR_DIR}
                ${JEMALLOC_DIR}
            )
        else()
            if(Mayo_TargetArchBitSize STREQUAL "32")
                list(APPEND OpenCASACADE_3RDPARTY_BINARY_DIRS ${CSF_OPT_BIN32})
            else()
                list(APPEND OpenCASACADE_3RDPARTY_BINARY_DIRS ${CSF_OPT_BIN64})
            endif()

            # Remove Qt binary path probably added in CSF_OPT_BIN(32/64) variable
            set(OpenCASCADE_QT_BINDIR)
            foreach(BINDIR ${OpenCASACADE_3RDPARTY_BINARY_DIRS})
                if(BINDIR MATCHES "qt5")
                    set(OpenCASCADE_QT_BINDIR ${BINDIR})
                endif()
            endforeach()
            list(REMOVE_ITEM OpenCASACADE_3RDPARTY_BINARY_DIRS ${OpenCASCADE_QT_BINDIR})
        endif()

        # Add OpenCASCADE library directories
        # Note: QtCreator option Run->Add_build_library_search_to_PATH will add to PATH env variable
        #       the contents of Mayo_LinkDirectories variable. For convenience, let's add also the
        #       directories containing the DLLs
        list(
            APPEND Mayo_LinkDirectories
            ${OpenCASCADE_BINARY_DIR}
            ${OpenCASCADE_LIBRARY_DIR}
            ${OpenCASACADE_3RDPARTY_BINARY_DIRS}
        )
    else()
        message(WARNING "OpenCASCADE not found from this location OpenCASCADE_DIR='${OpenCASCADE_DIR}' ")
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

    # Silent warnings C4996 and STL4015: the std::iterator class template(used as a base class to
    # provide typedefs) is deprecated in C++17
    if(MSVC AND OpenCASCADE_VERSION VERSION_LESS 7.7.0)
        list(APPEND Mayo_CompileDefinitions _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)
    endif()

    # Add OpenCASCADE libraries
    list(
        APPEND Mayo_LinkLibraries
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
        list(APPEND Mayo_LinkLibraries TKDEIGES)
        list(APPEND Mayo_LinkLibraries TKDESTEP)
        list(APPEND Mayo_LinkLibraries TKDESTL)
        list(APPEND Mayo_LinkLibraries TKDEVRML)
    else()
        list(APPEND Mayo_LinkLibraries TKIGES TKXDEIGES)
        list(APPEND Mayo_LinkLibraries TKSTEP TKSTEP209 TKSTEPAttr TKSTEPBase TKXDESTEP)
        list(APPEND Mayo_LinkLibraries TKSTL)
        list(APPEND Mayo_LinkLibraries TKVRML)
        if(OpenCASCADE_VERSION VERSION_GREATER_EQUAL 7.7.0)
            list(APPEND Mayo_LinkLibraries TKXDE)
        endif()
    endif()

    # OBJ/glTF support
    if(OpenCASCADE_VERSION VERSION_GREATER_EQUAL 7.4.0)
        list(APPEND Mayo_LinkLibraries TKRWMesh)
        if(OpenCASCADE_VERSION VERSION_GREATER_EQUAL 7.8.0)
            list(APPEND Mayo_LinkLibraries TKDEOBJ TKDEGLTF)
        endif()
    else()
        list(
            REMOVE_ITEM Mayo_SourceFiles
            ${PROJECT_SOURCE_DIR}/src/io_occ/io_occ_base_mesh.cpp
            ${PROJECT_SOURCE_DIR}/src/io_occ/io_occ_gltf_reader.cpp
            ${PROJECT_SOURCE_DIR}/src/io_occ/io_occ_obj_reader.cpp
        )
        message(STATUS "glTF reader disabled because OpenCascade < v7.4")
        message(STATUS "OBJ reader disabled because OpenCascade < v7.4")
    endif()

    if(OpenCASCADE_VERSION VERSION_LESS 7.5.0)
        list(REMOVE_ITEM Mayo_SourceFiles ${PROJECT_SOURCE_DIR}/src/io_occ/io_occ_gltf_writer.cpp)
        message(STATUS "glTF writer disabled because OpenCascade < v7.5")
    endif()

    if(OpenCASCADE_VERSION VERSION_LESS 7.6.0)
        list(REMOVE_ITEM Mayo_SourceFiles ${PROJECT_SOURCE_DIR}/src/io_occ/io_occ_obj_writer.cpp)
        message(STATUS "OBJ writer disabled because OpenCascade < v7.6")
    endif()

    # VRML support
    if(OpenCASCADE_VERSION VERSION_LESS 7.7.0)
        list(REMOVE_ITEM Mayo_SourceFiles ${PROJECT_SOURCE_DIR}/src/io_occ/io_occ_vrml_reader.cpp)
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
    list(
        APPEND Mayo_HeaderFiles
        ${PROJECT_SOURCE_DIR}/src/io_assimp/io_assimp.h
        ${PROJECT_SOURCE_DIR}/src/io_assimp/io_assimp_reader.h
    )
    list(
        APPEND Mayo_SourceFiles
        ${PROJECT_SOURCE_DIR}/src/io_assimp/io_assimp.cpp
        ${PROJECT_SOURCE_DIR}/src/io_assimp/io_assimp_reader.cpp
    )
    list(APPEND Mayo_LinkLibraries ${ASSIMP_LIBRARIES})
    list(APPEND Mayo_CompileDefinitions HAVE_ASSIMP)

    list(GET ASSIMP_INCLUDE_DIRS 0 assimp_IncludeDir)
    file(READ "${assimp_IncludeDir}/assimp/version.h" assimp_FileVersionH)
    if(NOT assimp_FileVersionH MATCHES "aiGetVersionPatch")
        list(APPEND Mayo_CompileDefinitions NO_ASSIMP_aiGetVersionPatch)
        message(STATUS "Assimp function aiGetVersionPatch() not available")
    endif()
endif()

##########
# gmio
##########

# TODO


##########
# Targets
##########

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(Mayo_AppIconMacOS images/appicon.icns)
    set_source_files_properties(images/appicon.icns PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")        
endif()

add_executable(
    mayo
    ${Mayo_SourceFiles}
    ${Mayo_HeaderFiles}
    ${Mayo_QtUiFiles}
    ${Mayo_QtI18nFiles}
    ${Mayo_QtQmFiles}
    mayo.qrc
    ${Mayo_RcIconsWin}
    ${Mayo_AppIconMacOS}
)

target_include_directories(
    mayo PRIVATE ${Mayo_IncludeDirectories}
)

target_compile_definitions(
    mayo PRIVATE ${Mayo_CompileDefinitions}
)

target_compile_options(
    mayo PRIVATE ${Mayo_CompileOptions}
)

target_link_directories(
    mayo PRIVATE ${Mayo_LinkDirectories}
)

target_link_libraries(
    mayo PRIVATE ${Mayo_LinkLibraries}
)

add_custom_target(
    OtherFiles SOURCES
    .github/workflows/ci_linux.yml
    .github/workflows/ci_linux_arm.yml
    .github/workflows/ci_macos.yml
    .github/workflows/ci_windows.yml
    images/credits.txt
    scripts/bump-version.rb
    scripts/gen-cmake-opencascade-vars.bat
    src/app/version.h.cmake
    README.md
)

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
