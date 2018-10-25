@echo off
call setpaths.bat

set MMGT_OPT=1
set MMGT_CLEAR=1
set MMGT_REENTRANT=0
set CSF_LANGUAGE=us
set CSF_EXCEPTION_PROMPT=1

set CSF_SHMessage=%APP_PATH%\3rdparty\occ\SHMessage
set CSF_MDTVTexturesDirectory=%APP_PATH%\3rdparty\occ\Textures
set CSF_ShadersDirectory=%APP_PATH%\3rdparty\occ\Shaders
set CSF_XSMessage=%APP_PATH%\3rdparty\occ\XSMessage
set CSF_TObjMessage=%APP_PATH%\3rdparty\occ\TObj
set CSF_StandardDefaults=%APP_PATH%\3rdparty\occ\StdResource
set CSF_PluginDefaults=%APP_PATH%\3rdparty\occ\StdResource
set CSF_XCAFDefaults=%APP_PATH%\3rdparty\occ\StdResource
set CSF_TObjDefaults=%APP_PATH%\3rdparty\occ\StdResource
set CSF_StandardLiteDefaults=%APP_PATH%\3rdparty\occ\StdResource
set CSF_IGESDefaults=%APP_PATH%\3rdparty\occ\XSTEPResource
set CSF_STEPDefaults=%APP_PATH%\3rdparty\occ\XSTEPResource
set CSF_XmlOcafResource=%APP_PATH%\3rdparty\occ\XmlOcafResource
set CSF_MIGRATION_TYPES=%APP_PATH%\3rdparty\occ\StdResource\MigrationSheet.txt

set PATH=%APP_PATH%\;%APP_PATH%\3rdparty;%PATH%

start mayo.exe %OPTIONS%
exit
