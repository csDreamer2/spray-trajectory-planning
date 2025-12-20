@echo off
echo Testing Optimized Async STEP Loading
echo ====================================

echo Starting application...
cd /d "%~dp0"

if exist "build\bin\Debug\SprayTrajectoryPlanning.exe" (
    echo Found Debug version
    echo.
    echo OPTIMIZATION CHANGES:
    echo 1. Deferred VTK Actor creation (50ms delay)
    echo 2. Deferred rendering (100ms delay)  
    echo 3. Large model detection and optimization
    echo 4. Non-blocking progress updates
    echo.
    echo EXPECTED BEHAVIOR:
    echo - UI should remain responsive during loading
    echo - Progress updates should appear smoothly
    echo - Final rendering should happen after a short delay
    echo - No "Not Responding" in task manager
    echo.
    echo WATCH FOR DEBUG OUTPUT:
    echo - "Creating VTK Actor in deferred call..."
    echo - "Large model detected (N cells), optimizing rendering"
    echo - "Scheduling deferred rendering..."
    echo - "Performing deferred rendering for ModelType"
    echo.
    "build\bin\Debug\SprayTrajectoryPlanning.exe"
) else (
    echo ERROR: Executable not found
    pause
    exit /b 1
)

pause