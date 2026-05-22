@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
cd /d D:\Dev\DeferredRendering
echo === Building spirv_reflect_test ===
cl /EHsc /std:c++17 /I"Source" /I"vendor/includes" /Fe:spirv_reflect_test.exe Source/SpirvReflect/SpirvReflectTest.cpp Source/SpirvReflect/spirv_reflect.c /link /SUBSYSTEM:CONSOLE
if %errorlevel% equ 0 (
    echo.
    echo === Running spirv_reflect_test ===
    echo.
    spirv_reflect_test.exe
) else (
    echo Build failed with error %errorlevel%
)
