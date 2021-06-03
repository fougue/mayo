@echo off

set DIR_SCRIPTS=%~dp0
lupdate -tr-function-alias ^
QT_TRANSLATE_NOOP+=MAYO_TEXT_ID,^
Q_DECLARE_TR_FUNCTIONS+=MAYO_DECLARE_TEXT_ID_FUNCTIONS,^
tr+=textId,^
tr+=textIdTr^
 %DIR_SCRIPTS%\..\i18n\i18n.pro
 
rem lupdate -no-obsolete
