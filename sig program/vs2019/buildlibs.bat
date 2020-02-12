@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

rem Set compilation parameters:
set "PARAMS=/m /nr:false /nologo /verbosity:q /t:Build" REM q:quiet m:minimum n:normal

rem Check Parameters:
if [%1] == [] goto buildall
if [%2] == [] goto end

rem ========= build specific config ==========
call :build siglibs.sln %1 %2
goto end

rem ========== default build cmds ==========
:buildall
call :build siglibs.sln Release x86
call :build siglibs.sln ReleaseDLL x86
call :build siglibs.sln Debug x86
call :build siglibs.sln Release x64
call :build siglibs.sln ReleaseDLL x64
call :build siglibs.sln Debug x64
pause
goto end

rem ========== functions ==========
:build
SETLOCAL
@echo Building: %1 %2 %3...
MSBuild %1 %PARAMS% /p:Configuration=%2 /p:Platform=%3
ENDLOCAL
goto end

:end
