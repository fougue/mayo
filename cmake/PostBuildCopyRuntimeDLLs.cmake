#****************************************************************************
#* Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
#* SPDX-License-Identifier: BSD-2-Clause
#****************************************************************************

# -----------------------------------
#
# FILE PostBuildCopyRuntimeDLLs.cmake
#
# Deploy runtime dependencies required by Mayo executables, including target DLLs, Qt plugins, and
# OpenCASCADE 3rd-party DLLs
#
# Dependencies are copied to the corresponding target output directory using POST_BUILD commands.
# This allows the generated binaries to be run directly from the build tree without requiring the
# user to manually install or copy their runtime dependencies
#
# ⚠
# This is primarily needed on Windows, where 3rd-party DLLs must be available alongside the
# executables. On Linux and macOS, runtime library paths (RPATH) and system/package-manager
# installations generally handle third-party library lookup
#
# -----------------------------------

# Helper function to copy runtime DLLs of a target
function(mayo_copy_runtime_dlls TARGET)
    if(NOT TARGET "${TARGET}")
        return()
    endif()

    add_custom_command(
        TARGET ${TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_RUNTIME_DLLS:${TARGET}>
                $<TARGET_FILE_DIR:${TARGET}>
        COMMAND_EXPAND_LISTS
    )
endfunction()

# Find and copy Qt plugins for "mayo" target
if (Mayo_BuildApp)
    # Run "qmake -query QT_INSTALL_PLUGINS" to retrieve base folder of Qt plugins
    get_target_property(Qt_QMAKE_EXECUTABLE Qt${QT_VERSION_MAJOR}::qmake IMPORTED_LOCATION)
    execute_process(
        COMMAND ${Qt_QMAKE_EXECUTABLE} -query QT_INSTALL_PLUGINS
        OUTPUT_VARIABLE QtPluginsDir
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    # Copy required Qt plugins
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
endif()

# Copy runtime DLLs for all targets
foreach(Target mayo mayo-conv ${Mayo_TestTargets})
    mayo_copy_runtime_dlls(${Target})
endforeach()

# Copy OpenCascade 3rd-party DLLs
# -- First find at least one target, the destination dir for the copy will use $<TARGET_FILE_DIR>
set(FirstTarget)
foreach(Target mayo mayo-conv ${Mayo_TestTargets})
    if(TARGET ${Target})
        set(FirstTarget ${Target})
        break()
    endif()
endforeach()

# -- Then do the copy to $<TARGET_FILE_DIR:${FirstTarget}>
# -- ⚠ this assumes all Mayo targets have the same destination dir
if(FirstTarget)
    foreach(Occ3rdDLL ${OpenCASCADE_3RDPARTY_DLLS})
        add_custom_command(
            TARGET ${FirstTarget} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${Occ3rdDLL}" "$<TARGET_FILE_DIR:${FirstTarget}>"
        )
    endforeach()
endif()
