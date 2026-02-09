@echo off

if ["%~1"]==[""] (
  @echo invalid arguments, please select configuration
  goto end
)

set "CONFIGURATION=%1"
set "SOURCE_DIRECTORY=%~dp0..\.."
set "SOLUTION_DIR=%~dp0..\..\solutions\utf8_msvc17_%CONFIGURATION%"

@mkdir %SOLUTION_DIR%

@pushd %SOLUTION_DIR%
CMake -G "Visual Studio 17 2022" -A Win32 %SOURCE_DIRECTORY% -DCMAKE_BUILD_TYPE:STRING=%CONFIGURATION% -DCMAKE_CONFIGURATION_TYPES:STRING=%CONFIGURATION% -DBUILD_TESTS=ON
@popd

:end

@exit /b %errorlevel%