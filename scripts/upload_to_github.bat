@echo off
setlocal enabledelayedexpansion
chcp 936 > nul

:: Change to project root directory
cd /d "%~dp0\.."

echo ========================================
echo GitHub Upload Script
echo Author: Wang Rui (Zhejiang University)
echo ========================================
echo.

echo Project root: %CD%
echo.

echo This script will help you upload the project to GitHub.
echo.

:: Check if Git is installed
git --version >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo [ERROR] Git is not installed or not in PATH
    echo Please install Git first: https://git-scm.com/
    pause
    exit /b 1
)

echo [OK] Git is available
git --version
echo.

:: Check if this is already a Git repository
if exist ".git" (
    echo [INFO] This is already a Git repository
    echo Current status:
    git status --porcelain
    echo.
    
    set /p CONTINUE="Do you want to continue? (y/n): "
    if /i "!CONTINUE!" neq "y" (
        echo Operation cancelled.
        pause
        exit /b 0
    )
) else (
    echo [INFO] Initializing new Git repository...
    git init
    if %ERRORLEVEL% neq 0 (
        echo [ERROR] Failed to initialize Git repository
        pause
        exit /b 1
    )
    echo [OK] Git repository initialized
)

echo.
echo ========================================
echo Step 1: Configure Git User (if needed)
echo ========================================
echo.

:: Check if Git user is configured
git config user.name >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo Git user not configured. Please enter your information:
    set /p GIT_NAME="Enter your name: "
    set /p GIT_EMAIL="Enter your email: "
    
    git config user.name "!GIT_NAME!"
    git config user.email "!GIT_EMAIL!"
    
    echo [OK] Git user configured
) else (
    echo [OK] Git user already configured:
    echo Name: 
    git config user.name
    echo Email: 
    git config user.email
)

echo.
echo ========================================
echo Step 2: Add Files to Git
echo ========================================
echo.

echo Adding files to Git repository...
git add .
if %ERRORLEVEL% neq 0 (
    echo [ERROR] Failed to add files to Git
    pause
    exit /b 1
)

echo [OK] Files added to Git staging area
echo.

echo Files to be committed:
git status --porcelain
echo.

:: Commit changes
set /p COMMIT_MSG="Enter commit message (or press Enter for default): "
if "!COMMIT_MSG!" == "" (
    set COMMIT_MSG="Initial commit: Spray Trajectory Planning System v1.0"
)

echo.
echo Committing with message: "!COMMIT_MSG!"
git commit -m "!COMMIT_MSG!"
if %ERRORLEVEL% neq 0 (
    echo [ERROR] Failed to commit changes
    pause
    exit /b 1
)

echo [OK] Changes committed successfully
echo.

echo ========================================
echo Step 3: GitHub Repository Setup
echo ========================================
echo.

echo Please create a new repository on GitHub:
echo 1. Go to https://github.com/new
echo 2. Repository name: spray-trajectory-planning (or your preferred name)
echo 3. Description: Industrial spray trajectory planning system based on Qt, VTK and OpenCASCADE
echo 4. Set to Public or Private as needed
echo 5. DO NOT initialize with README, .gitignore, or license (we already have them)
echo 6. Click "Create repository"
echo.

set /p REPO_URL="Enter the GitHub repository URL (e.g., https://github.com/username/spray-trajectory-planning.git): "

if "!REPO_URL!" == "" (
    echo [ERROR] Repository URL is required
    pause
    exit /b 1
)

echo.
echo Adding remote origin: !REPO_URL!
git remote add origin "!REPO_URL!" 2>nul
if %ERRORLEVEL% neq 0 (
    echo [INFO] Remote origin already exists, updating...
    git remote set-url origin "!REPO_URL!"
)

echo [OK] Remote origin configured
echo.

echo ========================================
echo Step 4: Push to GitHub
echo ========================================
echo.

echo Pushing to GitHub...
echo This may take a while for the first push...
echo.

git branch -M main
git push -u origin main

if %ERRORLEVEL% eq 0 (
    echo.
    echo ========================================
    echo SUCCESS!
    echo ========================================
    echo.
    echo Your project has been successfully uploaded to GitHub!
    echo.
    echo Repository URL: !REPO_URL!
    echo.
    echo Next steps:
    echo 1. Visit your repository on GitHub
    echo 2. Add a detailed description
    echo 3. Add topics/tags for better discoverability
    echo 4. Consider adding GitHub Actions for CI/CD
    echo 5. Add collaborators if needed
    echo.
    echo Repository features:
    echo - Complete source code
    echo - Documentation in docs/ folder
    echo - Environment setup scripts
    echo - Test scripts and programs
    echo - MIT License
    echo - Comprehensive .gitignore
    echo.
) else (
    echo.
    echo ========================================
    echo PUSH FAILED
    echo ========================================
    echo.
    echo The push to GitHub failed. This might be due to:
    echo 1. Authentication issues (need to login to GitHub)
    echo 2. Repository doesn't exist or URL is incorrect
    echo 3. Network connectivity issues
    echo 4. Permission issues
    echo.
    echo Please check the error messages above and try again.
    echo.
    echo Common solutions:
    echo 1. Make sure you're logged into GitHub CLI or have SSH keys set up
    echo 2. Verify the repository URL is correct
    echo 3. Check if the repository exists on GitHub
    echo 4. Try using SSH URL instead of HTTPS (or vice versa)
    echo.
)

echo.
echo ========================================
echo Additional Information
echo ========================================
echo.

echo Project Statistics:
echo - Source files: 
dir /s /b src\*.cpp src\*.h | find /c /v ""
echo - Documentation files:
dir /s /b docs\*.md | find /c /v ""
echo - Test scripts:
dir /s /b tests\*.bat tests\*.py 2>nul | find /c /v ""
echo - Configuration files:
dir /s /b config\*.cmake config\*.ini 2>nul | find /c /v ""

echo.
echo Repository size (approximate):
for /f "tokens=3" %%a in ('dir /s /-c ^| find "File(s)"') do echo %%a bytes

echo.
pause