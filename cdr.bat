@echo off

set "CDR_EXE=%~dp0_cdr.exe"

if "%~1"=="" (
    "%CDR_EXE%"
    goto :eof
)

if "%~1"=="-h" (
    "%CDR_EXE%" -h
    goto :eof
)

if "%~1"=="--help" (
    "%CDR_EXE%" --help
    goto :eof
)

set "TEMP_FILE=%TEMP%\cdr_output_%RANDOM%.tmp"
set "ARG=%~1"
if "%ARG:~-1%"=="\" set "ARG=%ARG:~0,-1%"
"%CDR_EXE%" "%ARG%" > "%TEMP_FILE%" 2>&1
set CDR_EXIT=%ERRORLEVEL%
set /p resolved_path=<"%TEMP_FILE%"
del "%TEMP_FILE%" >nul 2>&1

if %CDR_EXIT% NEQ 0 (
    if defined resolved_path echo %resolved_path%
    exit /b %CDR_EXIT%
)

pushd "%resolved_path%" 2>nul
if errorlevel 1 (
    echo Error: Failed to change to directory "%resolved_path%"
    exit /b 1
)
