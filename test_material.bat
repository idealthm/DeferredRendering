@echo off
setlocal

set "ROOT=%~dp0"
set "SOURCE_DIR=%ROOT%Source"
set "VENDOR_INCLUDES=%ROOT%vendor\includes"
set "GLM_DIR=%ROOT%SubModule\glm"
set "OUTPUT=%TEMP%\material_test.exe"

echo === Building Material + Buffer + ShaderParser Integration Tests ===
g++ -std=c++17 ^
    -I "%SOURCE_DIR%" ^
    -I "%VENDOR_INCLUDES%" ^
    -I "%GLM_DIR%" ^
    -D SPIRV_REFLECT_USE_SYSTEM_SPIRV_H ^
    "%SOURCE_DIR%\Material\MaterialInterfaceTest.cpp" ^
    "%SOURCE_DIR%\Material\Material.cpp" ^
    "%SOURCE_DIR%\Material\MaterialInstance.cpp" ^
    "%SOURCE_DIR%\Buffer\Buffer.cpp" ^
    "%SOURCE_DIR%\UnifromBuffer\UniformBuffer.cpp" ^
    "%SOURCE_DIR%\SpirvReflect\ShaderParse.cpp" ^
    "%SOURCE_DIR%\SpirvReflect\spirv_reflect.cpp" ^
    -o "%OUTPUT%"

if %ERRORLEVEL% neq 0 (
    echo Build failed!
    exit /b %ERRORLEVEL%
)

echo === Running ===
echo.
"%OUTPUT%"

endlocal
