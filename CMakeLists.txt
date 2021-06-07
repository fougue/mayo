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
        ${PROJECT_SOURCE_DIR}/*.cpp *.c
        ${PROJECT_SOURCE_DIR}/src/app/*.cpp *.c
        ${PROJECT_SOURCE_DIR}/src/intro/*.cpp *.c
        ${PROJECT_SOURCE_DIR}/src/project/*.cpp *.c
        ${PROJECT_SOURCE_DIR}/src/account/*.cpp *.c
        )

# Source Files Dump
foreach (_file ${SOURCE_FILES})
    message("Found: ${_file}")
endforeach ()
#-------------------------------------------------------------------

#-------------------------------------------------------------------
message("--> Searching header files....")
file(GLOB HEADER_FILES
        ${PROJECT_SOURCE_DIR}/*.h *.hpp
        ${PROJECT_SOURCE_DIR}/src/*.h *.hpp
        ${PROJECT_SOURCE_DIR}/src/intro/*.hpp *.h
        ${PROJECT_SOURCE_DIR}/src/project/*.hpp *.h
        ${PROJECT_SOURCE_DIR}/src/account/*.hpp *.h
        )

# Header Files Dump
foreach (_file ${HEADER_FILES})
    message("Found: ${_file}")
endforeach ()
#-------------------------------------------------------------------

#-------------------------------------------------------------------
message("--> Searching resource files...")
file(GLOB RESOURCE_FILES
        ${PROJECT_SOURCE_DIR}/resources/*.qrc
        )

# Resource Files Dump
foreach (_file ${RESOURCE_FILES})
    message("Found: ${_file}")
endforeach ()
#-------------------------------------------------------------------

#-------------------------------------------------------------------
message("--> Searching UI files....")
file(GLOB UI_FILES
        ${PROJECT_SOURCE_DIR}/*.cpp *.c
        ${PROJECT_SOURCE_DIR}/src/app/*.cpp
        ${PROJECT_SOURCE_DIR}/src/base/*.cpp
        ${PROJECT_SOURCE_DIR}/src/graphics/*.cpp
        ${PROJECT_SOURCE_DIR}/src/gui/*.cpp
        ${PROJECT_SOURCE_DIR}/src/io_gmio/*.cpp
        ${PROJECT_SOURCE_DIR}/src/io_occ/*.cpp
        )

# Source Files Dump
foreach (_file ${UI_FILES})
    message("Found: ${_file}")
endforeach ()
#-------------------------------------------------------------------

# Translation Files ------------------------------------------------
# YET NOT COMPATIBLE IN WINDOWS
option(CREATE_TRANSLATION "Create translation files" OFF)
if (${CREATE_TRANSLATION})
    set(TRANSLATION_FILES
            ${PROJECT_SOURCE_DIR}/resources/translations/turkish.ts
            ${PROJECT_SOURCE_DIR}/resources/translations/french.ts
            ${PROJECT_SOURCE_DIR}/resources/translations/russian.ts
            ${PROJECT_SOURCE_DIR}/resources/translations/italian.ts
            ${PROJECT_SOURCE_DIR}/resources/translations/chinese.ts
            ${PROJECT_SOURCE_DIR}/resources/translations/arabic.ts)

    # Translation Files Dump
    foreach (_file ${TRANSLATION_FILES})
        message("Found: ${_file}")
    endforeach ()
endif ()
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


if (${CREATE_TRANSLATION})
    find_package(Qt5LinguistTools)
    message("Qt5LinguistTools  : " ${Qt5LinguistTools_DIR})
endif ()

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


# ------------------------------------------------------------------------
add_executable(${PROJECT_NAME}
        ${SOURCE_FILES}
        ${HEADER_FILES}
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
        ${TabToolbar_LIBRARY}
        )

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