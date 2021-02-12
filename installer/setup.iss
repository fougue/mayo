#include "setupvars.iss"
#include "version.iss"

[Setup]
ArchitecturesInstallIn64BitMode=x64
AppId={{F1978C7C-3C90-477F-B634-B99746AA153D}
AppName={#TargetProduct}
AppVerName={#TargetProduct} v{#VersionNumber}
AppPublisher={#TargetCompany}
DefaultGroupName={#TargetCompany}
DefaultDirName={pf}\{#TargetCompany}\{#TargetProduct}
OutputBaseFilename={#TargetName}_v{#VersionNumber}_win{#TargetArch}_installer
Compression=lzma
SolidCompression=yes

[Icons]
Name: "{group}\{#TargetProduct}"; Filename: "{app}\{#TargetName}.exe"; WorkingDir: "{app}"; IconFileName: "{app}\{#TargetName}.exe"
Name: "{commondesktop}\{#TargetProduct}"; Filename: "{app}\{#TargetName}.exe"; Tasks: desktopicon; WorkingDir: "{app}"; IconFileName: "{app}\{#TargetName}.exe"

[Languages]
Name: "en"; MessagesFile: "compiler:Default.isl"
Name: "fr"; MessagesFile: "compiler:Languages\French.isl"
[CustomMessages]
en.installVcRuntime = Installing redistributable Visual Studio runtime ...
fr.installVcRuntime = Installation du package redistribuable Visual Studio ...

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

; First clear the setup folder (in case of a setup overwrite)
[InstallDelete]
Type: filesandordirs; Name: "{app}\*"

[Files]
Source: "{#AppBuildDir}\release\{#TargetName}.exe"; DestDir: "{app}"; Flags: ignoreversion

Source: "{#MsvcRedist_Dir}\vcredist_{#TargetArch}.exe"; DestDir: "{app}"; Flags: deleteafterinstall;
Source: "opencascade.conf"; DestDir: "{app}";
Source: "qt.conf"; DestDir: "{app}";

; Qt5
Source: "{#QtDir}\bin\Qt5Core.dll"; DestDir: "{app}"; Flags: ignoreversion;
Source: "{#QtDir}\bin\Qt5Gui.dll"; DestDir: "{app}"; Flags: ignoreversion;
Source: "{#QtDir}\bin\Qt5Svg.dll"; DestDir: "{app}"; Flags: ignoreversion;
Source: "{#QtDir}\bin\Qt5Widgets.dll"; DestDir: "{app}"; Flags: ignoreversion;
Source: "{#QtDir}\bin\Qt5WinExtras.dll"; DestDir: "{app}"; Flags: ignoreversion;
Source: "{#QtDir}\plugins\iconengines\qsvgicon.dll";  DestDir: "{app}\QtPlugins\iconengines"; Flags: ignoreversion
Source: "{#QtDir}\plugins\imageformats\qsvg.dll";  DestDir: "{app}\QtPlugins\imageformats"; Flags: ignoreversion
Source: "{#QtDir}\plugins\platforms\qwindows.dll";  DestDir: "{app}\QtPlugins\platforms"; Flags: ignoreversion

; OpenCascade
#include "opencascade_dlls.iss"

; OpenCascade resources
Source: "{#OpenCascade_SrcDir}\SHMessage\*.*"; DestDir: "{app}\OpenCascade\SHMessage"; Flags: ignoreversion
Source: "{#OpenCascade_SrcDir}\Shaders\*.*"; DestDir: "{app}\OpenCascade\Shaders"; Flags: ignoreversion
Source: "{#OpenCascade_SrcDir}\StdResource\*.*"; DestDir: "{app}\OpenCascade\StdResource"; Flags: ignoreversion
Source: "{#OpenCascade_SrcDir}\TObj\*.msg"; DestDir: "{app}\OpenCascade\TObj"; Flags: ignoreversion
Source: "{#OpenCascade_SrcDir}\Textures\*.*"; DestDir: "{app}\OpenCascade\Textures"; Flags: ignoreversion
Source: "{#OpenCascade_SrcDir}\XSMessage\*.*"; DestDir: "{app}\OpenCascade\XSMessage"; Flags: ignoreversion
Source: "{#OpenCascade_SrcDir}\XSTEPResource\*.*"; DestDir: "{app}\OpenCascade\XSTEPResource"; Flags: ignoreversion
Source: "{#OpenCascade_SrcDir}\XmlOcafResource\*.*"; DestDir: "{app}\OpenCascade\XmlOcafResource"; Flags: ignoreversion

; OpenCascade 3rdparty
Source: "{#Tbb_BinDir}\tbb.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#Tbb_BinDir}\tbbmalloc.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#FFMPEG_BinDir}\*.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#FreeImage_BinDir}\freeimage.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#FreeImage_BinDir}\freeimageplus.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#FreeType_BinDir}\freetype.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#OpenVR_BinDir}\*.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "{#TclTk_BinDir}\tcl86.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "{#TclTk_BinDir}\tk86.dll"; DestDir: "{app}"; Flags: ignoreversion

[Run]
Filename: "{app}\vcredist_{#TargetArch}.exe"; Parameters: "/q"; StatusMsg: "{cm:installVcRuntime}"
