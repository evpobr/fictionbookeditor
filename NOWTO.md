# FictionBook Editor HowTo

## How to build

To build FBE you need Visual Studio 2015, Community Edition is suitable. You can try previous versions, but it's untested. Nothing else is required.

Don't forget to install [MSXML 4.0](https://www.microsoft.com/en-us/download/details.aspx?id=15697), this component is required to run FictionBook Editor.

## How to make installer

To make installer you need to install [NSIS Unicode](http://www.scratchpaper.com/). Build Release configuration of FBE, then go to Installer folder. Fist run MakeInstaller.bat to copy required files to Installer folder, and after this load MakeInstaller.nsi to NSIS and compile.