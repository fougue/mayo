; #define MsvcVersionName StringChange(GetEnv("QMAKESPEC"), "win32-", "")
#define MsvcVersionName "msvc2017-x64"
#define TargetArch "x64"
#define VersionNumber "0.1"

#include "setupvars.iss"

[Setup]
ArchitecturesInstallIn64BitMode=x64
AppId={{F1978C7C-3C90-477F-B634-B99746AA153D}
AppName=Mayo
AppVerName=Mayo v{#VersionNumber}
AppPublisher=Fougue
DefaultGroupName=Fougue
DefaultDirName={pf}\Fougue\Mayo
OutputBaseFilename=mayo_v{#VersionNumber}_win{#TargetArch}_installer
Compression=lzma
SolidCompression=yes

[Icons]
Name: "{group}\Mayo"; Filename: "{app}\run.bat"; WorkingDir: "{app}"; IconFileName: "{app}\mayo.exe"
Name: "{commondesktop}\Mayo"; Filename: "{app}\run.bat"; Tasks: desktopicon; WorkingDir: "{app}"; IconFileName: "{app}\mayo.exe"

[Languages]
Name: "en"; MessagesFile: "compiler:Default.isl"
Name: "fr"; MessagesFile: "compiler:Languages\French.isl"
[CustomMessages]
en.installVcRuntime = Installing redistributable {#MsvcVersionName} runtime ...
fr.installVcRuntime = Installation du package redistribuable {#MsvcVersionName} ...

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "{#AppBuildDir}\mayo.exe"; DestDir: "{app}"; Flags: ignoreversion
; Source: "{#AppBuildDir}\i18n\*.qm"; DestDir: "{app}\i18n"; Flags: ignoreversion

Source: "run.bat"; DestDir: "{app}"; Flags: ignoreversion
Source: "setpaths.bat"; DestDir: "{app}"; Flags: ignoreversion

; First clear the setup folder (in case of a setup overwrite)
[InstallDelete]
Type: filesandordirs; Name: "{app}\*"

[Files]
Source: "{#MsvcRedist_Dir}\vcredist_{#TargetArch}.exe"; DestDir: "{app}\3rdparty"; Flags: deleteafterinstall;
Source: "qt.conf"; DestDir: "{app}";

; Qt5
Source: "{#QtDir}\bin\Qt5Core.dll"; DestDir: "{app}\3rdparty"; Flags: ignoreversion;
Source: "{#QtDir}\bin\Qt5Gui.dll"; DestDir: "{app}\3rdparty"; Flags: ignoreversion;
Source: "{#QtDir}\bin\Qt5Svg.dll"; DestDir: "{app}\3rdparty"; Flags: ignoreversion;
Source: "{#QtDir}\bin\Qt5Widgets.dll"; DestDir: "{app}\3rdparty"; Flags: ignoreversion;
Source: "{#QtDir}\plugins\iconengines\qsvgicon.dll";  DestDir: "{app}\3rdparty\plugins\iconengines"; Flags: ignoreversion
Source: "{#QtDir}\plugins\imageformats\qsvg.dll";  DestDir: "{app}\3rdparty\plugins\imageformats"; Flags: ignoreversion
Source: "{#QtDir}\plugins\platforms\qwindows.dll";  DestDir: "{app}\3rdparty\plugins\platforms"; Flags: ignoreversion

; OpenCascade
Source: "{#OpenCascade_BinDir}\TKBO.dll"; DestDir: "{app}\3rdparty"; Flags: ignoreversion
Source: "{#OpenCascade_BinDir}\TKBOOL.dll"; DestDir: "{app}\3rdparty"; Flags: ignoreversion
Source: "{#OpenCascade_BinDir}\TKBRep.dll"; DestDir: "{app}\3rdparty"; Flags: ignoreversion
Source: "{#OpenCascade_BinDir}\TKCAF.dll"; DestDir: "{app}\3rdparty"; Flags: ignoreversion
Source: "{#OpenCascade_BinDir}\TKCDF.dll"; DestDir: "{app}\3rdparty"; Flags: ignoreversion
Source: "{#OpenCascade_BinDir}\TKDraw.dll"; DestDir: "{app}\3rdparty"; Flags: ignoreversion
Source: "{#OpenCascade_BinDir}\TKernel.dll"; DestDir: "{app}\3rdparty"; Flags: ignoreversion
Source: "{#OpenCascade_BinDir}\TKFillet.dll"; DestDir: "{app}\3rdparty"; Flags: ignoreversion
Source: "{#OpenCascade_BinDir}\TKG2d.dll"; DestDir: "{app}\3rdparty"; Flags: ignoreversion
Source: "{#OpenCascade_BinDir}\TKG3d.dll"; DestDir: "{app}\3rdparty"; Flags: ignoreversion
Source: "{#OpenCascade_BinDir}\TKGeomAlgo.dll"; DestDir: "{app}\3rdparty"; Flags: ignoreversion
Source: "{#OpenCascade_BinDir}\TKGeomBase.dll"; DestDir: "{app}\3rdparty"; Flags: ignoreversion
Source: "{#OpenCascade_BinDir}\TKHLR.dll"; DestDir: "{app}\3rdparty"; Flags: ignoreversion
Source: "{#OpenCascade_BinDir}\TKIGES.dll"; DestDir: "{app}\3rdparty"; Flags: ignoreversion
Source: "{#OpenCascade_BinDir}\TKLCAF.dll"; DestDir: "{app}\3rdparty"; Flags: ignoreversion
Source: "{#OpenCascade_BinDir}\TKMath.dll"; DestDir: "{app}\3rdparty"; Flags: ignoreversion
Source: "{#OpenCascade_BinDir}\TKMesh.dll"; DestDir: "{app}\3rdparty"; Flags: ignoreversion
Source: "{#OpenCascade_BinDir}\TKMeshVS.dll"; DestDir: "{app}\3rdparty"; Flags: ignoreversion
Source: "{#OpenCascade_BinDir}\TKOffset.dll"; DestDir: "{app}\3rdparty"; Flags: ignoreversion
Source: "{#OpenCascade_BinDir}\TKOpenGl.dll"; DestDir: "{app}\3rdparty"; Flags: ignoreversion
Source: "{#OpenCascade_BinDir}\TKPrim.dll"; DestDir: "{app}\3rdparty"; Flags: ignoreversion
Source: "{#OpenCascade_BinDir}\TKService.dll"; DestDir: "{app}\3rdparty"; Flags: ignoreversion
Source: "{#OpenCascade_BinDir}\TKShHealing.dll"; DestDir: "{app}\3rdparty"; Flags: ignoreversion
Source: "{#OpenCascade_BinDir}\TKSTEP.dll"; DestDir: "{app}\3rdparty"; Flags: ignoreversion
Source: "{#OpenCascade_BinDir}\TKSTEP209.dll"; DestDir: "{app}\3rdparty"; Flags: ignoreversion
Source: "{#OpenCascade_BinDir}\TKSTEPAttr.dll"; DestDir: "{app}\3rdparty"; Flags: ignoreversion
Source: "{#OpenCascade_BinDir}\TKSTEPBase.dll"; DestDir: "{app}\3rdparty"; Flags: ignoreversion
Source: "{#OpenCascade_BinDir}\TKSTL.dll"; DestDir: "{app}\3rdparty"; Flags: ignoreversion
Source: "{#OpenCascade_BinDir}\TKTopAlgo.dll"; DestDir: "{app}\3rdparty"; Flags: ignoreversion
Source: "{#OpenCascade_BinDir}\TKV3d.dll"; DestDir: "{app}\3rdparty"; Flags: ignoreversion
Source: "{#OpenCascade_BinDir}\TKVCAF.dll"; DestDir: "{app}\3rdparty"; Flags: ignoreversion
Source: "{#OpenCascade_BinDir}\TKViewerTest.dll"; DestDir: "{app}\3rdparty"; Flags: ignoreversion
Source: "{#OpenCascade_BinDir}\TKVRML.dll"; DestDir: "{app}\3rdparty"; Flags: ignoreversion
Source: "{#OpenCascade_BinDir}\TKXCAF.dll"; DestDir: "{app}\3rdparty"; Flags: ignoreversion
Source: "{#OpenCascade_BinDir}\TKXDEIGES.dll"; DestDir: "{app}\3rdparty"; Flags: ignoreversion
Source: "{#OpenCascade_BinDir}\TKXDESTEP.dll"; DestDir: "{app}\3rdparty"; Flags: ignoreversion
Source: "{#OpenCascade_BinDir}\TKXSBase.dll"; DestDir: "{app}\3rdparty"; Flags: ignoreversion
Source: "{#OpenCascade_BinDir}\TKXSDRAW.dll"; DestDir: "{app}\3rdparty"; Flags: ignoreversion

; OpenCascade resources
Source: "{#OpenCascade_SrcDir}\SHMessage\*.*"; DestDir: "{app}\3rdparty\occ\SHMessage"; Flags: ignoreversion
Source: "{#OpenCascade_SrcDir}\Textures\*.*"; DestDir: "{app}\3rdparty\occ\Textures"; Flags: ignoreversion
Source: "{#OpenCascade_SrcDir}\Shaders\*.*"; DestDir: "{app}\3rdparty\occ\Shaders"; Flags: ignoreversion
Source: "{#OpenCascade_SrcDir}\XSMessage\*.*"; DestDir: "{app}\3rdparty\occ\XSMessage"; Flags: ignoreversion
Source: "{#OpenCascade_SrcDir}\TObj\*.msg"; DestDir: "{app}\3rdparty\occ\TObj"; Flags: ignoreversion
Source: "{#OpenCascade_SrcDir}\StdResource\*.*"; DestDir: "{app}\3rdparty\occ\StdResource"; Flags: ignoreversion
Source: "{#OpenCascade_SrcDir}\XSTEPResource\*.*"; DestDir: "{app}\3rdparty\occ\XSTEPResource"; Flags: ignoreversion
Source: "{#OpenCascade_SrcDir}\XmlOcafResource\*.*"; DestDir: "{app}\3rdparty\occ\XmlOcafResource"; Flags: ignoreversion

; OpenCascade 3rdparty
Source: "{#Tbb_BinDir}\tbb.dll"; DestDir: "{app}\3rdparty"; Flags: ignoreversion
Source: "{#Tbb_BinDir}\tbbmalloc.dll"; DestDir: "{app}\3rdparty"; Flags: ignoreversion
Source: "{#FFMPEG_BinDir}\avcodec*.dll"; DestDir: "{app}\3rdparty"; Flags: ignoreversion
Source: "{#FFMPEG_BinDir}\avformat*.dll"; DestDir: "{app}\3rdparty"; Flags: ignoreversion
Source: "{#FFMPEG_BinDir}\avutil*.dll"; DestDir: "{app}\3rdparty"; Flags: ignoreversion
Source: "{#FFMPEG_BinDir}\swresample*.dll"; DestDir: "{app}\3rdparty"; Flags: ignoreversion
Source: "{#FFMPEG_BinDir}\swscale*.dll"; DestDir: "{app}\3rdparty"; Flags: ignoreversion
Source: "{#FreeImage_BinDir}\freeimage.dll"; DestDir: "{app}\3rdparty"; Flags: ignoreversion
Source: "{#FreeImage_BinDir}\freeimageplus.dll"; DestDir: "{app}\3rdparty"; Flags: ignoreversion
Source: "{#FreeType_BinDir}\freetype.dll"; DestDir: "{app}\3rdparty"; Flags: ignoreversion
Source: "{#TclTk_BinDir}\tcl86.dll"; DestDir: "{app}\3rdparty"; Flags: ignoreversion
Source: "{#TclTk_BinDir}\tk86.dll"; DestDir: "{app}\3rdparty"; Flags: ignoreversion
Source: "{#TclTk_BinDir}\zlib1.dll"; DestDir: "{app}\3rdparty"; Flags: ignoreversion

[Run]
Filename: "{app}\3rdparty\vcredist_{#TargetArch}.exe"; Parameters: "/q"; StatusMsg: "{cm:installVcRuntime}"


[Code]
procedure CurStepChanged(CurStep: TSetupStep);
begin
  if CurStep = ssPostInstall then
  begin
    // Update the file setpaths.bat
    SaveStringToFile(ExpandConstant('{app}\setpaths.bat'),
                     ExpandConstant('set APP_PATH={app}'),
                     True)
  end;
end;

