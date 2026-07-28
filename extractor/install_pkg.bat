@echo off
setlocal EnableDelayedExpansion

set "par=%~2"
set "gamesDir=PATH_TO_GAMES_DIR"
set "addonsDir=PATH_TO_ADDONS_DIR"

IF "%gamesDir%"=="PATH_TO_GAMES_DIR" (
	echo You need to update gamesDir with the path where your PS4 games are installed.
	echo Example: set "gamesDir=D:\PS4 Games"
	IF NOT "%par%"=="--batch" pause
	exit /b 1
)

IF "%addonsDir%"=="PATH_TO_ADDONS_DIR" (
	echo You need to update addonsDir with the path where your PS4 DLC/addons are installed.
	echo Example: set "addonsDir=D:\PS4 Addons"
	IF NOT "%par%"=="--batch" pause
	exit /b 1
)

IF NOT EXIST "%~1" (
	echo Error: File not found: %~1
	IF NOT "%par%"=="--batch" pause
	exit /b 1
)

"%~dp0pkgunbox.exe" "%~1" --check-type
set "ret=%errorlevel%"

IF %ret%==101 (
	echo The file is a base game, installing to %gamesDir%
	"%~dp0pkgunbox.exe" "%~1" "%gamesDir%"
) ELSE IF %ret%==102 (
	echo The file is a game update, installing to %gamesDir%
	"%~dp0pkgunbox.exe" "%~1" "%gamesDir%"
) ELSE IF %ret%==103 (
	echo The file is a DLC, installing to %addonsDir%
	"%~dp0pkgunbox.exe" "%~1" "%addonsDir%"
) ELSE (
	echo An error occurred while detecting the package type.
	IF NOT "%par%"=="--batch" pause
	exit /b 1
)

IF NOT "%par%"=="--batch" pause
