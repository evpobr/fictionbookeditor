@ECHO OFF

devenv ..\FBE.sln /clean Release
devenv ..\FBE.sln /build Release

md .\Input
    
copy ..\Release\FBE.exe .\Input\
copy ..\Release\pcre.dll .\Input\
copy ..\Release\res_rus.dll .\Input\
copy ..\Release\res_ukr.dll .\Input\
copy ..\Release\FBSHell.dll .\Input\
copy ..\Release\ExportHTML.dll .\Input\
copy ..\Release\FBV.exe .\Input\

xcopy ..\FBE\files\*.* .\Input\ /E /Y /D

makensis.exe MakeInstaller.nsi
pause