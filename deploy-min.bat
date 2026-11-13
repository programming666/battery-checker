@echo off
setlocal
cd /d "%~dp0"

set "QT_BIN=D:\Qt\6.11.1\mingw_64\bin"
set "QT_PLUGINS=D:\Qt\6.11.1\mingw_64\plugins"
set "BUILD_DIR=build-min"
set "DIST=dist"

if not exist "%BUILD_DIR%\Battery-Chker.exe" (
    echo [x] %BUILD_DIR%\Battery-Chker.exe not found, build it first:
    echo     cmake -S . -B %BUILD_DIR% -G Ninja -DCMAKE_BUILD_TYPE=MinSizeRel -DCMAKE_PREFIX_PATH=D:/Qt/6.11.1/mingw_64
    echo     cmake --build %BUILD_DIR%
    exit /b 1
)

echo [1/3] reset %DIST%
if exist "%DIST%" rmdir /s /q "%DIST%"
mkdir "%DIST%"
if errorlevel 1 exit /b 1

echo [2/3] windeployqt
"%QT_BIN%\windeployqt.exe" --release --no-translations --no-opengl-sw --no-system-d3d-compiler --dir "%DIST%" "%BUILD_DIR%\Battery-Chker.exe"
if errorlevel 1 exit /b 1

echo [3/3] trim
for %%D in (tls networkinformation generic iconengines imageformats) do (
    if exist "%DIST%\%%D" rmdir /s /q "%DIST%\%%D"
)
if exist "%DIST%\Qt6Network.dll" del /q "%DIST%\Qt6Network.dll"
if exist "%DIST%\Qt6Svg.dll" del /q "%DIST%\Qt6Svg.dll"
mkdir "%DIST%\imageformats" 2>nul
copy /y "%QT_PLUGINS%\imageformats\qico.dll" "%DIST%\imageformats\" >nul
copy /y "%BUILD_DIR%\Battery-Chker.exe" "%DIST%\" >nul

echo.
echo Done. %DIST%\ contains:
dir /b "%DIST%"
