@echo off
setlocal enabledelayedexpansion

set PROJECT_ROOT=%~dp0
set PROJECT_ROOT=%PROJECT_ROOT:~0,-1%
set ENGINE_ROOT=C:\UESource\UnrealEngine5.8
set PROJECT=%PROJECT_ROOT%\project09.uproject
set CLIENT_ARCHIVE=%PROJECT_ROOT%\Saved\ClientBuild

echo ========================================
echo Building project09 Windows client package
echo ========================================
echo Project: %PROJECT%
echo Engine:  %ENGINE_ROOT%
echo Output:  %CLIENT_ARCHIVE%
echo.
echo NOTE: This script uses -skipcook.
echo       Cook Windows client content first with Launcher Editor Project Launcher.
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

if not exist "%PROJECT_ROOT%\Saved\Cooked\Windows" (
  echo ERROR: Missing cooked client content: %PROJECT_ROOT%\Saved\Cooked\Windows
  echo Cook Windows client content first, then rerun this script.
  pause
  exit /b 1
)

echo Cleaning previous client archive and staged build...
if exist "%CLIENT_ARCHIVE%" rmdir /s /q "%CLIENT_ARCHIVE%"
if exist "%PROJECT_ROOT%\Saved\StagedBuilds\Windows" rmdir /s /q "%PROJECT_ROOT%\Saved\StagedBuilds\Windows"

echo.
echo Building client target...
call "%ENGINE_ROOT%\Engine\Build\BatchFiles\Build.bat" project09 Win64 Development "-project=%PROJECT%" -WaitMutex
if errorlevel 1 (
  echo ERROR: Client build failed.
  pause
  exit /b 1
)

echo.
echo Staging and archiving client package...
call "%ENGINE_ROOT%\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun ^
  -project="%PROJECT%" ^
  -noP4 ^
  -platform=Win64 ^
  -clientconfig=Development ^
  -skipcook ^
  -stage ^
  -pak ^
  -archive ^
  -archivedirectory="%CLIENT_ARCHIVE%" ^
  -target="project09"
if errorlevel 1 (
  echo ERROR: Client packaging failed.
  pause
  exit /b 1
)

if not exist "%CLIENT_ARCHIVE%\Windows\project09\Binaries\Win64\project09.exe" (
  echo ERROR: Client executable was not found in the archived package.
  pause
  exit /b 1
)

echo.
echo Client package complete.
echo Output: %CLIENT_ARCHIVE%\Windows
echo.
pause
endlocal
