@ECHO OFF

devenv ..\FBE.sln /clean Release
devenv ..\FBE.sln /build Release

md .\Input
md .\Input\en-US\
md .\Input\ru-RU\
md .\Input\uk-UA\
    
copy ..\Release\FBE.exe .\Input\
copy ..\Release\en-US\FBE.exe.mui .\Input\en-US\
copy ..\Release\ru-RU\FBE.exe.mui .\Input\ru-RU\
copy ..\Release\uk-UA\FBE.exe.mui .\Input\uk-UA\
copy ..\Release\pcre.dll .\Input\
copy ..\Release\libhunspell.dll .\Input\
copy ..\Release\FBSHell.dll .\Input\
copy ..\Release\ExportHTML.dll .\Input\
copy ..\Release\FBV.exe .\Input\
copy ..\Release\SciLexer.dll .\Input\

xcopy ..\FBE\files\*.* .\Input\ /E /Y /D

makensis.exe MakeInstaller.nsi
pause