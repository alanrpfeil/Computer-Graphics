@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

REM q:quiet m:minimum n:normal
set "PARAMS=/m /nr:false /nologo /verbosity:q /t:Build /p:Platform=x86"

rem ========== build cmds ==========
call :build siglibs.sln Release
call :build siglibs.sln ReleaseDLL
call :build siglibs.sln Debug
pause
goto end

rem ========== functions ==========
:build
SETLOCAL
@echo Building: %1 %2...
MSBuild %1 %PARAMS% /p:Configuration=%2
ENDLOCAL
goto end

:end
