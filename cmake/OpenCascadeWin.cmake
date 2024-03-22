set(OpenCASCADE_DIR CACHE PATH "Path where is located OpenCASCADE env.bat file")

# OpenCASCADE_DIR may actually points to opencascade/cmake
if(NOT EXISTS ${OpenCASCADE_DIR}/env.bat)
    cmake_path(GET OpenCASCADE_DIR PARENT_PATH OpenCASCADE_DIR_PARENT_PATH)
    if(EXISTS ${OpenCASCADE_DIR_PARENT_PATH}/env.bat)
        set(OpenCASCADE_DIR ${OpenCASCADE_DIR_PARENT_PATH})
    endif()
endif()

# Generate file "OpenCascadeWin_env.cmake"
if(EXISTS "${OpenCASCADE_DIR}/env.bat")
    execute_process(
        COMMAND cmd /c ${PROJECT_SOURCE_DIR}/scripts/gen-cmake-opencascade-vars.bat "${OpenCASCADE_DIR}"
        OUTPUT_FILE ${CMAKE_BINARY_DIR}/OpenCascadeWin_env.cmake
    )
    include(${CMAKE_BINARY_DIR}/OpenCascadeWin_env.cmake)
endif()

# Set OpenCASCADE variables
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

    # Add OpenCASCADE "HAVE_RAPIDJSON" compile flags
    if(OpenCASCADE_HAVE_RAPIDJSON STREQUAL "true")
        list(APPEND Mayo_CompileDefinitions HAVE_RAPIDJSON)
    elseif(OpenCASCADE_VERSION VERSION_GREATER_EQUAL 7.8.0)
        file(GLOB OpenCASCADE_CompileDefsCMakeFiles ${OpenCASCADE_DIR}/cmake/OpenCASCADECompileDefinitions*.cmake)
        foreach(CMakeFile ${OpenCASCADE_CompileDefsCMakeFiles})
            file(READ "${CMakeFile}" CMakeFileContents)
            if(CMakeFileContents MATCHES ":HAVE_RAPIDJSON>")
                list(APPEND Mayo_CompileDefinitions HAVE_RAPIDJSON)
                break()
            endif()
        endforeach()
    endif()

    # Set OpenCASCADE_3RDPARTY_BINARY_DIRS variable that will contain the OpenCASCADE
    # 3rd-party binary directories(ie containing DLL files)
    set(OpenCASCADE_3RDPARTY_BINARY_DIRS)
    if(OpenCASCADE_VERSION VERSION_GREATER_EQUAL 7.8.0)
        list(
            APPEND OpenCASCADE_3RDPARTY_BINARY_DIRS
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
            list(APPEND OpenCASCADE_3RDPARTY_BINARY_DIRS ${CSF_OPT_BIN32})
        else()
            list(APPEND OpenCASCADE_3RDPARTY_BINARY_DIRS ${CSF_OPT_BIN64})
        endif()

        # Remove Qt binary path probably added in CSF_OPT_BIN(32/64) variable
        set(OpenCASCADE_QT_BINDIR)
        foreach(BINDIR ${OpenCASCADE_3RDPARTY_BINARY_DIRS})
            if(BINDIR MATCHES "qt5")
                set(OpenCASCADE_QT_BINDIR ${BINDIR})
            endif()
        endforeach()
        list(REMOVE_ITEM OpenCASCADE_3RDPARTY_BINARY_DIRS ${OpenCASCADE_QT_BINDIR})
    endif()

    # Create imported target for each toolkit
    set(
        OpenCASCADE_TOOLKITS
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
        list(APPEND OpenCASCADE_TOOLKITS  TKDE TKDEIGES TKDESTEP TKDESTL TKDEVRML)
    else()
        list(APPEND OpenCASCADE_TOOLKITS  TKIGES TKXDEIGES)
        list(APPEND OpenCASCADE_TOOLKITS  TKSTEP TKSTEP209 TKSTEPAttr TKSTEPBase TKXDESTEP)
        list(APPEND OpenCASCADE_TOOLKITS  TKSTL)
        list(APPEND OpenCASCADE_TOOLKITS  TKVRML)
        if(OpenCASCADE_VERSION VERSION_GREATER_EQUAL 7.7.0)
            list(APPEND OpenCASCADE_TOOLKITS  TKXDE)
        endif()
    endif()

    if(OpenCASCADE_VERSION VERSION_GREATER_EQUAL 7.4.0)
        list(APPEND OpenCASCADE_TOOLKITS  TKRWMesh)
        if(OpenCASCADE_VERSION VERSION_GREATER_EQUAL 7.8.0)
            list(APPEND OpenCASCADE_TOOLKITS  TKDEOBJ TKDEGLTF)
        endif()
    endif()

    foreach(OccTK ${OpenCASCADE_TOOLKITS})
        add_library(${OccTK} SHARED IMPORTED)
        set_target_properties(
            ${OccTK} PROPERTIES
            IMPORTED_IMPLIB "${OpenCASCADE_LIBRARY_DIR}/${OccTK}.lib"
            IMPORTED_LOCATION "${OpenCASCADE_BINARY_DIR}/${OccTK}.dll"
        )
    endforeach()

    # List all 3rd-party DLLs required by OpenCASCADE
    set(OpenCASCADE_3RDPARTY_DLLS)
    foreach(OccBINDIR ${OpenCASCADE_3RDPARTY_BINARY_DIRS})
        file(GLOB OccBINDIR_DLLS ${OccBINDIR}/*.dll)
        list(APPEND OpenCASCADE_3RDPARTY_DLLS ${OccBINDIR_DLLS})
    endforeach()
else()
    message(WARNING "OpenCASCADE not found from this location OpenCASCADE_DIR='${OpenCASCADE_DIR}' ")
endif()
