#include <windows.h>

VS_VERSION_INFO VERSIONINFO
 FILEVERSION @Mayo_VersionMajor@,@Mayo_VersionMinor@,@Mayo_VersionPatch@,0
 PRODUCTVERSION @Mayo_VersionMajor@,@Mayo_VersionMinor@,@Mayo_VersionPatch@,0
 FILEFLAGSMASK VS_FFI_FILEFLAGSMASK
 FILEFLAGS 0
 FILEOS VOS_NT_WINDOWS32
 FILETYPE VFT_APP
 FILESUBTYPE 0
BEGIN
    BLOCK "StringFileInfo"
    BEGIN
        BLOCK "040904B0"
        BEGIN
            VALUE "CompanyName",      "@Mayo_CompanyName@"
            VALUE "FileDescription",  "@MayoApp_Description@"
            VALUE "FileVersion",      "@Mayo_Version@"
            VALUE "InternalName",     "mayo"
            VALUE "OriginalFilename", "mayo.exe"
            VALUE "ProductName",      "Mayo"
            VALUE "ProductVersion",   "@Mayo_Version@"
            VALUE "LegalCopyright",   "@Mayo_Copyright@"
        END
    END

    BLOCK "VarFileInfo"
    BEGIN
        VALUE "Translation", 0x0409, 1200
    END
END