@echo off
setlocal enabledelayedexpansion

set PROJECT_ROOT=%~dp0
set PROJECT_ROOT=%PROJECT_ROOT:~0,-1%
set ENGINE_ROOT=C:\UE4Source\UnrealEngine
set PROJECT=%PROJECT_ROOT%\project09.uproject
set SERVER_ARCHIVE=%PROJECT_ROOT%\Saved\ServerBuild
set SERVER_BIN=%SERVER_ARCHIVE%\WindowsServer\project09\Binaries\Win64
set SERVER_START_SCRIPT=%SERVER_BIN%\start_server.bat
set ROOT_START_SCRIPT=%PROJECT_ROOT%\start_packaged_server.bat

echo ========================================
echo Building project09 Windows server package
echo ========================================
echo Project: %PROJECT%
echo Engine:  %ENGINE_ROOT%
echo Output:  %SERVER_ARCHIVE%
echo.
echo NOTE: This script uses -skipcook.
echo       Cook WindowsServer content first with Launcher Editor Project Launcher.
echo       Do not delete Saved\Cooked before running this script.
echo.

if not exist "%ENGINE_ROOT%\Engine\Build\BatchFiles\Build.bat" (
  echo ERROR: Build.bat not found under %ENGINE_ROOT%.
  pause
  exit /b 1
)

if not exist "%PROJECT%" (
  echo ERROR: Project file not found: %PROJECT%
  pause
  exit /b 1
)

if not exist "%PROJECT_ROOT%\Saved\Cooked\WindowsServer" (
  echo ERROR: Missing cooked server content: %PROJECT_ROOT%\Saved\Cooked\WindowsServer
  echo Cook WindowsServer content first, then rerun this script.
  pause
  exit /b 1
)

echo Cleaning previous server archive and staged build...
tasklist /FI "IMAGENAME eq project09Server.exe" | find /I "project09Server.exe" >nul
if not errorlevel 1 (
  echo ERROR: project09Server.exe is still running and may lock the server build folder.
  echo Close the dedicated server window or run: taskkill /IM project09Server.exe /F
  pause
  exit /b 1
)

if exist "%SERVER_ARCHIVE%" rmdir /s /q "%SERVER_ARCHIVE%"
if exist "%PROJECT_ROOT%\Saved\StagedBuilds\WindowsServer" rmdir /s /q "%PROJECT_ROOT%\Saved\StagedBuilds\WindowsServer"

echo.
echo Building server target...
call "%ENGINE_ROOT%\Engine\Build\BatchFiles\Build.bat" project09Server Win64 Development "-project=%PROJECT%" -WaitMutex
if errorlevel 1 (
  echo ERROR: Server build failed.
  pause
  exit /b 1
)

echo.
echo Staging and archiving server package...
call "%ENGINE_ROOT%\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun ^
  -project="%PROJECT%" ^
  -noP4 ^
  -server ^
  -noclient ^
  -serverplatform=Win64 ^
  -serverconfig=Development ^
  -skipcook ^
  -stage ^
  -pak ^
  -archive ^
  -archivedirectory="%SERVER_ARCHIVE%" ^
  -target="project09Server"
if errorlevel 1 (
  echo ERROR: Server packaging failed.
  pause
  exit /b 1
)

echo.
echo Writing server startup script...
if not exist "%SERVER_BIN%" (
  echo ERROR: Server binary directory not found: %SERVER_BIN%
  pause
  exit /b 1
)

call :WriteStartServerScript "%SERVER_START_SCRIPT%" "%SERVER_BIN%\project09Server.exe" "%SERVER_BIN%\server.log" "%SERVER_BIN%"
if errorlevel 1 (
  echo ERROR: Failed to write startup script: %SERVER_START_SCRIPT%
  pause
  exit /b 1
)

call :WriteStartServerScript "%ROOT_START_SCRIPT%" "%SERVER_BIN%\project09Server.exe" "%SERVER_BIN%\server.log" "%SERVER_BIN%"
if errorlevel 1 (
  echo ERROR: Failed to write startup script: %ROOT_START_SCRIPT%
  pause
  exit /b 1
)

echo.
echo Server package complete.
echo Output: %SERVER_ARCHIVE%\WindowsServer
echo Startup script: %SERVER_START_SCRIPT%
echo Root startup script: %ROOT_START_SCRIPT%
echo.
endlocal
exit /b 0

:WriteStartServerScript
set SCRIPT_PATH=%~1
set SCRIPT_SERVER_EXE=%~2
set SCRIPT_LOG_FILE=%~3
set SCRIPT_WORK_DIR=%~4

> "%SCRIPT_PATH%" echo @echo off
>> "%SCRIPT_PATH%" echo setlocal
>> "%SCRIPT_PATH%" echo cd /d "%SCRIPT_WORK_DIR%"
>> "%SCRIPT_PATH%" echo set SERVER_EXE=%SCRIPT_SERVER_EXE%
>> "%SCRIPT_PATH%" echo set MAP=/Game/Maps/csgo
>> "%SCRIPT_PATH%" echo set PORT=7777
>> "%SCRIPT_PATH%" echo set LOG_FILE=%SCRIPT_LOG_FILE%
>> "%SCRIPT_PATH%" echo echo Starting project09 dedicated server...
>> "%SCRIPT_PATH%" echo echo Work dir: %%CD%%
>> "%SCRIPT_PATH%" echo echo Exe: %%SERVER_EXE%%
>> "%SCRIPT_PATH%" echo echo Map: %%MAP%%
>> "%SCRIPT_PATH%" echo echo Port: %%PORT%%
>> "%SCRIPT_PATH%" echo echo Log: %%LOG_FILE%%
>> "%SCRIPT_PATH%" echo echo.
>> "%SCRIPT_PATH%" echo if not exist "%%SERVER_EXE%%" ^(
>> "%SCRIPT_PATH%" echo   echo ERROR: Missing server executable: %%SERVER_EXE%%
>> "%SCRIPT_PATH%" echo   goto fail
>> "%SCRIPT_PATH%" echo ^)
>> "%SCRIPT_PATH%" echo "%%SERVER_EXE%%" %%MAP%% -log -port=%%PORT%% -nullrhi -unattended -abslog="%%LOG_FILE%%"
>> "%SCRIPT_PATH%" echo set EXITCODE=%%ERRORLEVEL%%
>> "%SCRIPT_PATH%" echo echo.
>> "%SCRIPT_PATH%" echo echo Server exited with code %%EXITCODE%%.
>> "%SCRIPT_PATH%" echo if not "%%EXITCODE%%"=="0" goto fail
>> "%SCRIPT_PATH%" echo pause
>> "%SCRIPT_PATH%" echo exit /b 0
>> "%SCRIPT_PATH%" echo :fail
>> "%SCRIPT_PATH%" echo echo.
>> "%SCRIPT_PATH%" echo echo Startup failed. Check the messages above and %%LOG_FILE%% if it exists.
>> "%SCRIPT_PATH%" echo pause
>> "%SCRIPT_PATH%" echo exit /b 1
exit /b %ERRORLEVEL%
