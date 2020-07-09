; This file is a template for creating your own local setupvars.iss file included by setup.iss
; setupvars.iss is not versioned control as it is user-specific

; Directory where is located mayo.exe
#define AppBuildDir "C:\projects\mayo\build-mayo-Qt_5_11_1_MSVC_2017_x64\release"

; Base directory of Qt(same as environment variable QTDIR)
#define QtDir "C:\libs\Qt-5.11.1_msvc2017_x64\5.11.1\msvc2017_64"

; Directory for OpenCascade DLLs
#define OpenCascade_BinDir "C:\libs\OpenCASCADE-7.3.0-vc14-64\opencascade-7.3.0\win64\vc14\bin"

; Directory for OpenCascade sources
#define OpenCascade_SrcDir "C:\libs\OpenCASCADE-7.3.0-vc14-64\opencascade-7.3.0\src"

; Directory for OpenCascade 3rdparty ffmpeg DLLs
#define FFMPEG_BinDir "C:\libs\OpenCASCADE-7.3.0-vc14-64\ffmpeg-3.3-lgpl-64\bin"

; Directory for OpenCascade 3rdparty freeimage DLLs
#define FreeImage_BinDir "C:\libs\OpenCASCADE-7.3.0-vc14-64\freeimage-3.17.0-vc14-64\bin"

; Directory for OpenCascade 3rdparty freetype DLLs
#define FreeType_BinDir "C:\libs\OpenCASCADE-7.3.0-vc14-64\freetype-2.5.5-vc14-64\bin"

; Directory for OpenCascade 3rdparty tbb DLLs
#define Tbb_BinDir "C:\libs\OpenCASCADE-7.3.0-vc14-64\tbb_2017.0.100\bin\intel64\vc14"

; Directory for OpenCascade 3rdparty tcltk DLLs
#define TclTk_BinDir "C:\libs\OpenCASCADE-7.3.0-vc14-64\tcltk-86-64\bin"

; Directory for MSVC 2017 redistributable package (vcredist)
#define MsvcRedist_Dir "C:\Program Files (x86)\Microsoft Visual Studio\2017\Professional\VC\Redist\MSVC\14.14.26405"
