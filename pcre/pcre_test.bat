@REM This is a generated file.
@echo off
setlocal
SET srcdir="D:\Users\evpobr\Documents\GitHubVisualStudio\pcre"
SET pcretest="D:\Users\evpobr\Documents\GitHubVisualStudio\pcre\DEBUG\pcretest.exe"
if not [%CMAKE_CONFIG_TYPE%]==[] SET pcretest="D:\Users\evpobr\Documents\GitHubVisualStudio\pcre\%CMAKE_CONFIG_TYPE%\pcretest.exe"
call %srcdir%\RunTest.Bat
if errorlevel 1 exit /b 1
echo RunTest.bat tests successfully completed
