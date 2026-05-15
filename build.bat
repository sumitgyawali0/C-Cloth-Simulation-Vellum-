@echo off
echo Building Cloth Simulation...

if not exist "build" mkdir build
cd build

echo Configuring with CMake...
cmake .. -G "Visual Studio 16 2019" -A x64

if %ERRORLEVEL% NEQ 0 (
    echo CMake configuration failed!
    echo Please ensure you have CMake and Visual Studio installed.
    pause
    exit /b 1
)

echo Building project...
cmake --build . --config Release

if %ERRORLEVEL% NEQ 0 (
    echo Build failed!
    pause
    exit /b 1
)

echo Build successful!
echo.
echo To run the simulation:
echo   cd build\bin\Release
echo   ClothSimulation.exe
echo.
echo Controls:
echo   R - Reset cloth
echo   Space - Toggle render mode
echo   W - Toggle wireframe
echo   Up/Down - Adjust constraint iterations
echo.
pause

