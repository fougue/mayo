@echo off

set DIR_SCRIPTS=%~dp0
lupdate -no-obsolete -tr-function-alias ^
QT_TRANSLATE_NOOP+=MAYO_TEXT_ID,^
Q_DECLARE_TR_FUNCTIONS+=MAYO_DECLARE_TEXT_ID_FUNCTIONS,^
tr+=textId,^
tr+=textIdTr^
 %DIR_SCRIPTS%\..\mayo.pro
 
