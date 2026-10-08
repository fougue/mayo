#****************************************************************************
#* Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
#* SPDX-License-Identifier: BSD-2-Clause
#****************************************************************************

# Detect OCCT compile definitions exported by its CMake package
set(OpenCASCADE_AllCompileDefinitions "")
file(GLOB OpenCASCADE_CompileDefsCMakeFiles
    "${OpenCASCADE_CMAKE_DIR}/OpenCASCADECompileDefinitions*.cmake"
)

foreach(CMakeFile IN LISTS OpenCASCADE_CompileDefsCMakeFiles)
    file(READ "${CMakeFile}" CMakeFileContents)
    string(APPEND OpenCASCADE_AllCompileDefinitions "\n${CMakeFileContents}")
endforeach()

function(Mayo_DetectOcctDefinition Definition Result)
    set(${Result} FALSE PARENT_SCOPE)

    if(OpenCASCADE_AllCompileDefinitions MATCHES ":${Definition}>")
        set(${Result} TRUE PARENT_SCOPE)
        message(STATUS "OpenCascade ${Definition}: yes")
    else()
        message(STATUS "OpenCascade ${Definition}: no")
    endif()
endfunction()