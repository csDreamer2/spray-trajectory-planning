@echo off
echo Testing Async STEP Loading with Debug Output
echo ============================================

echo Starting application with console output...
cd /d "%~dp0"

if exist "build\bin\Debug\SprayTrajectoryPlanning.exe" (
    echo Found Debug version
    echo.
    echo IMPORTANT: Watch the console output for debug messages
    echo Expected debug output sequence:
    echo 1. "Starting async STEP model loading"
    echo 2. "Worker thread started, sending load request"
    echo 3. "WORKER THREAD: Starting STEP file loading"
    echo 4. "WORKER: Emitting progress update - Reading STEP file"
    echo 5. Progress updates from worker thread
    echo 6. "MAIN THREAD: Async STEP loading completed"
    echo.
    echo If you don't see these messages, the async loading is not working.
    echo.
    "build\bin\Debug\SprayTrajectoryPlanning.exe"
) else (
    echo ERROR: Executable not found
    pause
    exit /b 1
)

pause