@echo off
setlocal
set "OUT_DIR=%~1"
set "TEMPLATE_DIR=%~2"
set "MAT_DIR=%~3"
rem Strip trailing backslashes
if "%OUT_DIR:~-1%"=="\" set "OUT_DIR=%OUT_DIR:~0,-1%"
if "%TEMPLATE_DIR:~-1%"=="\" set "TEMPLATE_DIR=%TEMPLATE_DIR:~0,-1%"
if "%MAT_DIR:~-1%"=="\" set "MAT_DIR=%MAT_DIR:~0,-1%"

if not exist "%OUT_DIR%" mkdir "%OUT_DIR%"
if not exist "CompiledMaterials" mkdir "CompiledMaterials"

echo === Compiling materials ===
for %%f in ("%MAT_DIR%\*.mat") do (
    echo matc: %%~nxf
    "%OUT_DIR%\ShaderCompiler.exe" "%%f" --template-dir "%TEMPLATE_DIR%" -o "%OUT_DIR%"
    if errorlevel 1 exit /b 1
    "%OUT_DIR%\ShaderCompiler.exe" "%%f" --template-dir "%TEMPLATE_DIR%" -o "CompiledMaterials"
    if errorlevel 1 exit /b 1
)

echo === Compiling lighting ===
"%OUT_DIR%\ShaderCompiler.exe" __lighting__ --template-dir "%TEMPLATE_DIR%" -o "%OUT_DIR%"
if errorlevel 1 exit /b 1
"%OUT_DIR%\ShaderCompiler.exe" __lighting__ --template-dir "%TEMPLATE_DIR%" -o "CompiledMaterials"
if errorlevel 1 exit /b 1

echo === Done ===
