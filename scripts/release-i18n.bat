@echo off

SET SCRIPTDIR=%~dp0
echo %SCRIPTDIR:~0,-1%
lrelease-pro %SCRIPTDIR%\..\mayo.pro
 
