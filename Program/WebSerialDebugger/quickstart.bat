@echo off
REM Quick Start Script for Web Serial Debugger
REM Usage: quickstart.bat [port]

echo ==========================================
echo   Web Serial Debugger - Quick Start
echo ==========================================

setlocal enabledelayedexpansion

REM Check Python
python --version >nul 2>&1
if errorlevel 1 (
    echo ❌ Error: Python is not installed or not in PATH
    exit /b 1
)

for /f "tokens=2" %%a in ('python --version 2^>^&1') do (
    echo ✅ Python version: %%a
)

REM Check/Install dependencies
echo.
echo 📦 Checking dependencies...
python -c "import flask, serial" >nul 2>&1
if errorlevel 1 (
    echo Installing required packages...
    pip install -r requirements.txt
) else (
    echo ✅ Dependencies already installed
)

REM Get port from argument or default
if "%~1"=="" (
    set PORT=5000
) else (
    set PORT=%~1
)

echo.
echo 🚀 Starting Web Serial Debugger...
echo    URL: http://localhost:%PORT%
echo.
echo Press Ctrl+C to stop
echo ==========================================

python app.py --port %PORT%
