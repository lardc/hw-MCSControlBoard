@echo off
setlocal enabledelayedexpansion

rem Delete only files with exact extensions .bin and .hex
for %%f in (*.bin *.hex) do (
    if /i "%%~xf"==".bin" del "%%f" 2>NUL
    if /i "%%~xf"==".hex" del "%%f" 2>NUL
)

rem If no arguments are passed, skip the hex generation and go to file renaming
rem Otherwise, execute the provided command for hex file generation
if "%~1"=="" goto blank

rem Combine all passed arguments into a single command
rem Remove any double quotes from the command
set hex_cmd=%*
set hex_cmd=!hex_cmd:"=!

rem Execute the hex generation command
%hex_cmd%

:blank

rem Extract the project name from the Git remote origin URL
rem The URL is split by "-" and ".", and the 3rd token is taken as the project name
for /f "tokens=3 delims=-." %%i in ('git config --local remote.origin.url') do set git_proj=%%i

rem Get the date of the last commit in the format YYYY.MM.DD_HH.MM
rem The date is extracted using git log with a custom format
for /f %%i in ('git log -1 "--pretty=%%cd" "--date=format:%%Y.%%m.%%d_%%H.%%M"') do set git_date=%%i

rem Construct the output file name using the project name and commit date
set f_name=%git_proj%_%git_date%

rem Check if the .binary file exists and copy it to the new file name
if exist "%git_proj%.binary" (
    copy /y "%git_proj%.binary" "%f_name%.bin" 1>NUL
    echo File %f_name%.bin created
)

rem Check if the .hex file exists and copy it to the new file name
if exist "%git_proj%.hex" (
    copy /y "%git_proj%.hex" "%f_name%.hex" 1>NUL
    echo File %f_name%.hex created
)

rem End the local environment changes
endlocal
