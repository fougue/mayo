cmake_minimum_required(VERSION 3.10)
project(mayo)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules")

#-------------------------------------------------------------------
message("--> Searching source files....")
file(GLOB SOURCE_FILES
        ${PROJECT_SOURCE_DIR}/src/app/*.cpp
        ${PROJECT_SOURCE_DIR}/src/base/*.cpp
        ${PROJECT_SOURCE_DIR}/src/graphics/*.cpp
        ${PROJECT_SOURCE_DIR}/src/gui/*.cpp
        ${PROJECT_SOURCE_DIR}/src/io_occ/*.cpp
        ${PROJECT_SOURCE_DIR}/src/io_gmio/*.cpp
        )

# Source Files Dump
foreach (_file ${SOURCE_FILES})
    message("Found: ${_file}")
endforeach ()
#-------------------------------------------------------------------

#-------------------------------------------------------------------
message("--> Searching header files....")
file(GLOB HEADER_FILES
        ${PROJECT_SOURCE_DIR}/src/app/*.h
        ${PROJECT_SOURCE_DIR}/src/base/*.h
        ${PROJECT_SOURCE_DIR}/src/graphics/*.h
        ${PROJECT_SOURCE_DIR}/src/gui/*.h
        ${PROJECT_SOURCE_DIR}/src/io_occ/*.h
        ${PROJECT_SOURCE_DIR}/src/3rdparty/gsl/*
        ${PROJECT_SOURCE_DIR}/src/io_gmio/*.h
        )

# Header Files Dump
foreach (_file ${HEADER_FILES})
    message("Found: ${_file}")
endforeach ()
#-------------------------------------------------------------------

#-------------------------------------------------------------------
message("--> Searching resource files...")
file(GLOB RESOURCE_FILES
        ${PROJECT_SOURCE_DIR}/*.qrc
        )

# Resource Files Dump
foreach (_file ${RESOURCE_FILES})
    message("Found: ${_file}")
endforeach ()
#-------------------------------------------------------------------

#-------------------------------------------------------------------
message("--> Searching UI files....")
file(GLOB UI_FILES
        ${PROJECT_SOURCE_DIR}/src/app/*.ui
        )

#QT_WRAP_UI(${UI_FILES})

# Source Files Dump
foreach (_file ${UI_FILES})
    message("Found: ${_file}")
endforeach ()
#-------------------------------------------------------------------

option(USE_OCE "Use OpenCASCADE Community Edition " OFF)

if (USE_OCE)
    # OPENCASCADE COMMUNITY EDITION
    set(OCE_DIR /usr/lib/x86_64-linux-gnu/oce-0.17)
    find_package(OCE REQUIRED)
    include_directories(${OCE_INCLUDE_DIRS})
    SET(OpenCASCADE_LIBRARIES
            TKernel;TKMath
            TKG2d;TKG3d;TKGeomBase;TKBRep
            TKGeomAlgo;TKTopAlgo;TKPrim;TKBO;TKShHealing;TKBool;TKHLR;TKFillet;TKOffset;TKFeat;TKMesh;TKXMesh
            TKService;TKV3d;TKOpenGl;TKMeshVS
            TKCDF;TKLCAF;TKCAF;TKBinL;TKXmlL;TKBin;TKXml;TKTObj;TKBinTObj;TKXmlTObj #;TKVCAF #TKStdL;TKStd;
            TKXSBase;TKSTEPBase;TKSTEPAttr;TKSTEP209;TKSTEP;TKIGES;TKXCAF;TKXDEIGES;TKXDESTEP;TKSTL;TKVRML;TKXmlXCAF;TKBinXCAF; #TKRWMesh
            #TKDraw;TKTopTest;TKViewerTest;TKXSDRAW;TKDCAF;TKXDEDRAW;TKTObjDRAW;TKQADraw;
            -lGL -lGLU #-lglut
            TKernel TKMath TKSTL TKSTEP TKV3d TKOpenGl
            )
else ()
    ### OpenCASCADE Official Edition
    add_definitions(-DOCCT)
    if (WIN32)
        find_path(OpenCASCADE_DIR OpenCASCADEConfig.cmake
                C:\\Dev\\OpenCASCADE\\OpenCASCADEv740\\cmake)
    else ()
        find_path(OpenCASCADE_DIR OpenCASCADEConfig.cmake
                /opt/Dev/OpenCASCADE/lib/cmake/opencascade/
                /opt/opencascade-7.4.0/lib/cmake/opencascade/)
    endif ()
    find_package(OpenCASCADE REQUIRED)
    find_package(OpenCASCADE COMPONENTS FoundationClasses REQUIRED)
    find_package(OpenCASCADE COMPONENTS ModelingData REQUIRED)
    find_package(OpenCASCADE COMPONENTS DataExchange REQUIRED)
    include_directories(${OpenCASCADE_INCLUDE_DIR})

endif ()


if(${OpenCASCADE_VERSION} VERSION_GREATER_EQUAL 7.4.0)
    message("OpenCASCADE Version: ${OpenCASCADE_VERSION}")

    list(REMOVE_ITEM SOURCE_FILES
           ${PROJECT_SOURCE_DIR}/src/io_occ/io_occ_base_mesh.cpp
           ${PROJECT_SOURCE_DIR}/src/io_occ/io_occ_gltf_reader.cpp
           ${PROJECT_SOURCE_DIR}/src/io_occ/io_occ_obj.cpp
           )
endif()

#QT5
if (WIN32)
    find_path(Qt5_DIR Qt5Config.cmake
            C:\\Dev\\Qt\\5.12.10\\mingw73_64\\lib\\cmake\\Qt5
            )
endif ()

find_package(Qt5 COMPONENTS Core REQUIRED)
find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt5 COMPONENTS Gui REQUIRED)
find_package(Qt5 COMPONENTS Xml REQUIRED)
find_package(Qt5 COMPONENTS OpenGL REQUIRED)
find_package(Qt5 COMPONENTS Network REQUIRED)

message("--> Printing QT5 Modules")
message("Qt5               : " ${Qt5_DIR})
message("Qt5Core           : " ${Qt5Core_DIR})
message("Qt5Widgets        : " ${Qt5Widgets_DIR})
message("Qt5Gui            : " ${Qt5Gui_DIR})
message("Qt5OpenGL         : " ${Qt5OpenGL_DIR})

message("--> Printing Qt5 Libraries")
get_target_property(QtCore_location Qt5::Core LOCATION)
message("Qt5::Core Lib    : ${QtCore_location}")
get_target_property(QtWidgets_location Qt5::Widgets LOCATION)
message("Qt5::Widgets Lib : ${QtWidgets_location}")
get_target_property(QtGui_location Qt5::Gui LOCATION)
message("Qt5::Gui Lib     : ${QtGui_location}")

include_directories(
        src/app
        src/3rdparty
)

# ------------------------------------------------------------------------
add_executable(${PROJECT_NAME}
        ${SOURCE_FILES}
        ${HEADER_FILES}
        ${UI_FILES}
        ${RESOURCE_FILES}
        ${TRANSLATION_FILES}
        ${QM_FILES}
        )

target_link_libraries(${PROJECT_NAME} PRIVATE
        ${OpenCASCADE_LIBRARIES}
        ${OpenCASCADE_OCAF_LIBRARIES}
        Qt5::Core
        Qt5::Widgets
        Qt5::Xml
        Qt5::Network
        )

find_package(GMIO)

###########################################################################################
# first we can indicate the documentation build as an option and set it to ON by default
option(BUILD_DOC "Build documentation" OFF)

if (${BUILD_DOC})
    # check if Doxygen is installed
    find_package(Doxygen)
    if (DOXYGEN_FOUND)
        # set input and output files
        set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
        set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

        # request to configure the file
        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
        message("Doxygen build started")

        # note the option ALL which allows to build the docs together with the application
        add_custom_target(doc_doxygen ALL
                COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                COMMENT "Generating API documentation with Doxygen"
                VERBATIM)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT})

    else (DOXYGEN_FOUND)
        message("Doxygen need to be installed to generate the doxygen documentation")
    endif (DOXYGEN_FOUND)
endif ()
##################################################################