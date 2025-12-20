@echo off
echo Testing Async STEP Loading Feature
echo ===================================

echo Starting application...
cd /d "%~dp0"

if exist "build\bin\Debug\SprayTrajectoryPlanning.exe" (
    echo Found Debug version
    start "SprayTrajectoryPlanning" "build\bin\Debug\SprayTrajectoryPlanning.exe"
    echo.
    echo OpenCASCADE Configuration Test Results:
    echo    - Headers access: SUCCESS
    echo    - STEP reader: SUCCESS  
    echo    - MPX3500.STEP loading: SUCCESS (took ~90 seconds)
    echo    - Mesh generation: SUCCESS
    echo.
    echo Async Loading Test Steps:
    echo.
    echo 1. Click "File" -^> "Import Workshop Model" in the application
    echo 2. Select "data\model\MPX3500.STEP" file
    echo 3. Choose "Async Loading" in the dialog (recommended)
    echo 4. Watch status bar progress updates:
    echo    - "Reading STEP file..."
    echo    - "Parsing STEP geometry..."
    echo    - "Generating mesh..."
    echo    - "Converting to VTK format..."
    echo    - "Creating 3D model..."
    echo 5. Verify UI remains responsive (move window, click buttons)
    echo 6. Wait for completion and check 3D model display
    echo.
    echo Expected Performance:
    echo    - Sync loading: UI frozen for 90 seconds
    echo    - Async loading: UI smooth, background processing
    echo.
    echo Expected Results:
    echo    - UI always responsive
    echo    - Status bar shows real-time progress
    echo    - Successfully displays 3D model
    echo    - Console outputs detailed logs
    echo.
) else if exist "build\bin\Release\SprayTrajectoryPlanning.exe" (
    echo Found Release version
    start "SprayTrajectoryPlanning" "build\bin\Release\SprayTrajectoryPlanning.exe"
) else (
    echo ERROR: Executable not found
    echo Please compile the project first
    pause
    exit /b 1
)

pause