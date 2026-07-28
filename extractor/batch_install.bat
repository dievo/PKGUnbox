@echo off
setlocal

set "dir=%~1"

IF "%dir%"=="" (
    echo Error: Please provide a directory containing .pkg files.
    echo Usage: %~nx0 "D:\path\to\pkg\directory"
    pause
    exit /b 1
)

IF NOT EXIST "%dir%" (
    echo Error: Directory not found: %dir%
    pause
    exit /b 1
)

set count=0
FOR %%i IN ("%dir%\*.pkg") DO (
    set /a count+=1
    call "%~dp0install_pkg.bat" "%%i" --batch
)

IF %count%==0 (
    echo No .pkg files found in %dir%
) ELSE (
    echo Batch install done (%count% file(s) processed^).
)

pause
