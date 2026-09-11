#****************************************************************************
#* Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
#* SPDX-License-Identifier: BSD-2-Clause
#****************************************************************************

# -----------------------------------
#
# FILE BuildTests.cmake
#
# Configure the Mayo test suite, including test executables and CTest integration
#
# Common test input files are copied to the build directory in "<build-dir>/test/inputs"
# Each test module is built as a separate executable and registered with CTest
#
# -----------------------------------

include(CTest)
enable_testing()

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test)

# Hide BUILD_TESTING option from CMake configuration interfaces
# Mayo uses its own option (Mayo_BuildTests)
set_property(CACHE BUILD_TESTING PROPERTY TYPE INTERNAL)

# Copy common input files into build dir
file(GLOB MayoTests_InputFiles ${PROJECT_SOURCE_DIR}/tests/inputs/*.*)
file(COPY ${MayoTests_InputFiles} DESTINATION ${CMAKE_BINARY_DIR}/tests/inputs)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/tests/outputs)

# Helper function
function(mayo_add_test MODULE)
    cmake_parse_arguments(TEST
        ""
        ""
        "SOURCES;LIBRARIES;DEFINITIONS;OPTIONS"
        ${ARGN}
    )

    set(TARGET "test-${MODULE}")
    add_executable(${TARGET} ${TEST_SOURCES})

    target_compile_definitions(
        ${TARGET} PRIVATE ${Mayo_CompileDefinitions} ${TEST_DEFINITIONS}
    )
    target_compile_options(
        ${TARGET} PRIVATE ${Mayo_CompileOptions} ${TEST_OPTIONS}
    )
    target_link_libraries(
        ${TARGET} PRIVATE ${TEST_LIBRARIES} Qt${QT_VERSION_MAJOR}::Test
    )
    set_target_properties(${TARGET}
        PROPERTIES
            WIN32_EXECUTABLE FALSE
            AUTOMOC ON
    )

    set(TEST_COMMAND ${TARGET})
    if(Mayo_TestOutputFilenameTemplate)
        string(
            REPLACE "<test>" "${TARGET}"
            TEST_OUTPUT_FILENAME "${Mayo_TestOutputFilenameTemplate}"
        )
        list(APPEND TEST_COMMAND -o "${TEST_OUTPUT_FILENAME},${Mayo_TestOutputFormat}")
    else()
        list(APPEND TEST_COMMAND "-${Mayo_TestOutputFormat}")
    endif()

    add_test(NAME ${TARGET} COMMAND ${TEST_COMMAND})

    set_tests_properties(${TARGET} PROPERTIES WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

    set(Mayo_TestTargets ${Mayo_TestTargets} ${TARGET} PARENT_SCOPE)
endfunction()

# test-base
file(
    GLOB MayoTestBase_SourceFiles
    ${PROJECT_SOURCE_DIR}/tests/test_base*.h
    ${PROJECT_SOURCE_DIR}/tests/test_base*.cpp
)
mayo_add_test(
    base
    SOURCES ${MayoTestBase_SourceFiles}  LIBRARIES MayoCoreLib
)

# test-io
file(
    GLOB MayoTestIO_SourceFiles
    ${PROJECT_SOURCE_DIR}/tests/test_io*.h
    ${PROJECT_SOURCE_DIR}/tests/test_io*.cpp
)
mayo_add_test(
    io
    SOURCES ${MayoTestIO_SourceFiles}  LIBRARIES MayoCoreLib MayoIOLib
)

# test-graphics
file(
    GLOB MayoTestGraphics_SourceFiles
    ${PROJECT_SOURCE_DIR}/tests/test_graphics*.h
    ${PROJECT_SOURCE_DIR}/tests/test_graphics*.cpp
)
mayo_add_test(
    graphics
    SOURCES ${MayoTestGraphics_SourceFiles}  LIBRARIES MayoCoreLib
)

# test-measure
file(
    GLOB MayoTestMeasure_SourceFiles
    ${PROJECT_SOURCE_DIR}/tests/test_measure*.h
    ${PROJECT_SOURCE_DIR}/tests/test_measure*.cpp
    ${PROJECT_SOURCE_DIR}/src/measure/measure*.cpp
)
mayo_add_test(
    measure
    SOURCES ${MayoTestMeasure_SourceFiles}  LIBRARIES MayoCoreLib MayoIOLib
)

# test-app
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui)

file(
    GLOB MayoTestApp_SourceFiles
    ${PROJECT_SOURCE_DIR}/tests/test_app*.h
    ${PROJECT_SOURCE_DIR}/tests/test_app*.cpp
)
mayo_add_test(
    app
    SOURCES
        ${MayoTestApp_SourceFiles}
        ${PROJECT_SOURCE_DIR}/src/app/app_module.cpp
        ${PROJECT_SOURCE_DIR}/src/app/app_module_properties.cpp
        ${PROJECT_SOURCE_DIR}/src/app/app_ui_state.cpp
        ${PROJECT_SOURCE_DIR}/src/app/brep_meshing.cpp
        ${PROJECT_SOURCE_DIR}/src/app/document_files_watcher.cpp
        ${PROJECT_SOURCE_DIR}/src/app/qstring_utils.cpp
        ${PROJECT_SOURCE_DIR}/src/app/qtgui_utils.cpp
        ${PROJECT_SOURCE_DIR}/src/app/recent_files.cpp
        ${PROJECT_SOURCE_DIR}/src/qtcommon/qtcore_utils.cpp
    LIBRARIES
        MayoCoreLib
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
)
