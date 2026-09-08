#****************************************************************************
#* Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
#* SPDX-License-Identifier: BSD-2-Clause
#****************************************************************************

include(CTest)
enable_testing()

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

    target_compile_definitions(${TARGET} PRIVATE ${Mayo_CompileDefinitions} ${TEST_DEFINITIONS})
    target_compile_options(${TARGET} PRIVATE ${Mayo_CompileOptions} ${TEST_OPTIONS})
    target_link_libraries(${TARGET} PRIVATE ${TEST_LIBRARIES} Qt${QT_VERSION_MAJOR}::Test)
    set_target_properties(${TARGET}
        PROPERTIES
            WIN32_EXECUTABLE FALSE
            AUTOMOC ON
    )

    add_test(NAME ${TARGET} COMMAND ${TARGET})
    set_tests_properties(${TARGET} PROPERTIES WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
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
