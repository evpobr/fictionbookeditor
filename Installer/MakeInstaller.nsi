Unicode true

SetCompressor /SOLID lzma

!define PRODUCT_NAME "FictionBook Editor"
!define PRODUCT_STAGE "Release"
!define PRODUCT_VERSION "${PRODUCT_STAGE} v2.7 beta1"
!define PRODUCT_VENDOR "FBE Team"
!define PRODUCT_NAME_VERSION "${PRODUCT_NAME} ${PRODUCT_VERSION}"
!define PRODUCT_URL "http://code.google.com/p/fictionbookeditor/"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\FBE.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"
!define PRODUCT_STARTMENU_REGVAL "NSIS:StartMenuDir"
!define LIBRARY_SHELL_EXTENSION

;--------------------------------
;Interface Configuration

  !define MUI_WELCOMEFINISHPAGE_BITMAP "..\\FBE\\res\\fbe.bmp"
  !define MUI_UNWELCOMEFINISHPAGE_BITMAP "..\\FBE\\res\\fbe.bmp"

RequestExecutionLevel admin

; Includes
!include "MUI2.nsh"
!include "Library.nsh"
!include "LogicLib.nsh"
!include "x64.nsh"

!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"
!define SERVER "http://fictionbookeditor.googlecode.com/files/"

; Installer pages

; Welcome page
!define MUI_WELCOMEPAGE_TITLE_3LINES
!insertmacro MUI_PAGE_WELCOME

; License page
!insertmacro MUI_PAGE_LICENSE $(License)
LicenseForceSelection radiobuttons

; Components page
!insertmacro MUI_PAGE_COMPONENTS

; Directory page
!insertmacro MUI_PAGE_DIRECTORY

; Start menu page
var ICONS_GROUP
!define MUI_STARTMENUPAGE_NODISABLE
!define MUI_STARTMENUPAGE_DEFAULTFOLDER "FictionBook Editor"
!define MUI_STARTMENUPAGE_REGISTRY_ROOT "${PRODUCT_UNINST_ROOT_KEY}"
!define MUI_STARTMENUPAGE_REGISTRY_KEY "${PRODUCT_UNINST_KEY}"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "${PRODUCT_STARTMENU_REGVAL}"
!insertmacro MUI_PAGE_STARTMENU Application $ICONS_GROUP

; Instfiles page
!insertmacro MUI_PAGE_INSTFILES

; Finish page
!define MUI_FINISHPAGE_RUN
!define MUI_FINISHPAGE_RUN_FUNCTION ExecAppFile
!define MUI_FINISHPAGE_TITLE_3LINES
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages

; Uninstaller welcome page
!define MUI_WELCOMEPAGE_TITLE_3LINES
;!insertmacro MUI_UNPAGE_WELCOME

; Uninstaller confirm page
!insertmacro MUI_UNPAGE_CONFIRM

; Uninstaller instfile page
!insertmacro MUI_UNPAGE_INSTFILES

; Uninstaller finish page
!define MUI_FINISHPAGE_TITLE_3LINES
!insertmacro MUI_UNPAGE_FINISH

; MUI end 

!define /date DATE "%d_%b"

; Language files
!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "Russian"
!insertmacro MUI_LANGUAGE "Ukrainian"

; Localized strings
!include "Localization\English.nsh"
!include "Localization\Russian.nsh"
!include "Localization\Ukrainian.nsh"

Name "${PRODUCT_NAME_VERSION}"
OutFile "${PRODUCT_NAME_VERSION}.exe"
InstallDir "$PROGRAMFILES\FictionBook Editor"
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""
ShowInstDetails show
ShowUnInstDetails show

Function .OnInstFailed
FunctionEnd
 
Function .OnInstSuccess
FunctionEnd

Function CheckMSXMLVersion
  ReadRegStr $0 HKCR "Msxml2.SAXXMLReader.6.0\CLSID" ""
  StrCmp $0 "" noxml
  ReadRegStr $1 HKCR "CLSID\$0\ProgID" ""
  StrCmp $1 "Msxml2.SAXXMLReader.6.0" ok
noxml:
  MessageBox MB_OK|MB_ICONSTOP $(ErrCheckMSXMLVersion)
  Quit
ok:
FunctionEnd

Function CheckIEVersion
  GetDllVersion "shdocvw.dll" $0 $1
  IntCmp $0 327730 ok 0 ok
  MessageBox MB_OK|MB_ICONSTOP $(ErrCheckIEVersion)
  Quit
ok:
FunctionEnd

Function CheckFBERunning
check:
  FindWindow $0 "FictionBookEditorFrame"
  IntCmp $0 0 ok1
  MessageBox MB_OK|MB_ICONSTOP $(ErrCheckFBERunning)
  Goto check
ok1:
  FindWindow $0 "" "FictionBook Validator"
  IntCmp $0 0 ok2
  MessageBox MB_OK|MB_ICONSTOP $(ErrCheckFBVRunning)
  Goto check
ok2:
FunctionEnd

Function un.onInit 
  !insertmacro MUI_LANGDLL_DISPLAY
FunctionEnd

Function un.OnUnInstFailed
FunctionEnd
 
Function un.OnUnInstSuccess
FunctionEnd

Function un.CheckFBERunning
check:
  FindWindow $0 "FictionBookEditorFrame"
  IntCmp $0 0 ok1
  MessageBox MB_OK|MB_ICONSTOP $(ErrCheckFBERunning)
  Goto check
ok1:
  FindWindow $0 "" "FictionBook Validator"
  IntCmp $0 0 ok2
  MessageBox MB_OK|MB_ICONSTOP $(ErrCheckFBVRunning)
  Goto check
ok2:
FunctionEnd

; added by SeNS
Function un.GetUserAppData
  Push $1
  Push $2
  Push $3
  Push $4  
 
  StrCpy $1 ""
  StrCpy $2 "0x001C" # CSIDL_LOCAL_APPDATA 0x001c // <user name>\Local Settings\Applicaiton Data (non roaming)
  StrCpy $3 ""
  StrCpy $4 ""
 
  System::Call 'shell32::SHGetSpecialFolderPath(i $HWNDPARENT, t .r1, i r2, i r3) i .r4'
 
  Pop $4
  Pop $3
  Pop $2
  Exch $1
FunctionEnd

Function ExecFBE
  Exec '"$INSTDIR\fbe.exe"'
FunctionEnd

Function ExecAppFile
  Call ExecFBE
FunctionEnd

Section !$(Main) MainSection_id
  SectionIn RO
  ReadRegStr $0 HKLM "SOFTWARE\Microsoft\Windows NT\CurrentVersion" CurrentVersion
  StrCmp $0 "" 0 nthere
  MessageBox MB_OK|MB_ICONSTOP $(ErrNTCurrentVersion)
  Quit
nthere:
  Call CheckMSXMLVersion
  Call CheckIEVersion
  Call CheckFBERunning

  ; create an FB2 progid
  WriteRegStr HKCR "FictionBook.2" "" "FictionBook"
  WriteRegStr HKCR "FictionBook.2\CurVer" "" "FictionBook.2"

  ; create an FB2 filetype
  WriteRegStr HKCR ".fb2" "" "FictionBook.2"
  WriteRegStr HKCR ".fb2" "PerceivedType" "Text"
  WriteRegStr HKCR ".fb2" "Content Type" "text/xml"
  WriteRegStr HKCR ".fb2" "DefaultIcon" "$INSTDIR\FBE.exe,-1"


  SetOutPath "$INSTDIR"
  File "..\Release\symbols.ini" 
  File "..\Release\blank.fb2"  
  File "..\Release\fb2.xsl"
  File "..\Release\eng.xsl"
  File "..\Release\rus.xsl"
  File "..\Release\ukr.xsl"
  File "..\Release\html.xsl"
  File "..\Release\FictionBook.xsd"
  File "..\Release\FictionBookGenres.xsd"
  File "..\Release\FictionBookLang.xsd"
  File "..\Release\FictionBookLinks.xsd"
  File "..\Release\genres.txt"
  File "..\Release\genres.rus.txt"
  File "..\Release\genres.txt_L"
  File "..\Release\genres.rus.txt_L"
  File "..\Release\genres.ukr.txt"
  File "..\Release\main.css"
  File "..\Release\main_fast.css"
  File "..\Release\main.html"
  File "..\Release\main.js"
  File "..\Release\libHunspell.dll"
  File "..\Release\SciLexer.dll"
  File "..\Release\pcre2-8.dll"
  File "..\Release\FBV.exe"
  File "..\Release\gdiplus.manifest"
  File "..\Release\gdiplus.dll"
  File "..\Release\gdiplus.cat"

  File "..\Release\gpl-3.0.txt"
  File "..\Release\gpl-3.0.ru.txt"
  File "..\Release\gpl-3.0.ua.txt"

  File "..\Release\contributors.txt"
  File "..\Release\copying.txt"
  
  File "..\Release\CHANGELOG.md"
  File "..\Release\HOWTO.md"
  File "..\Release\README.md"
  File "..\Release\TODO.md"

  SetOutPath "$INSTDIR\en-US"
  CreateDirectory "$INSTDIR\en-US"
  File "..\Release\en-US\FBE.exe.mui"
  SetOutPath "$INSTDIR\ru-RU"
  CreateDirectory "$INSTDIR\ru-RU"
  File "..\Release\ru-RU\FBE.exe.mui"
  SetOutPath "$INSTDIR\uk-UA"
  CreateDirectory "$INSTDIR\uk-UA"
  File "..\Release\uk-UA\FBE.exe.mui"


  !insertmacro InstallLib TLB NOTSHARED REBOOT_PROTECTED "..\Release\FBE.exe" "$INSTDIR\FBE.exe" "$INSTDIR"
  
  ; register verb
  ;WriteRegStr HKCR "FictionBook.2\shell\Open\Command" "" '"$INSTDIR\FBE.exe" "%1"'
  WriteRegStr HKCR "FictionBook.2\shell\Edit\Command" "" '"$INSTDIR\FBE.exe" "%1"'
  
  ; Write the installation path into the registry
  WriteRegStr HKLM "SOFTWARE\${PRODUCT_VENDOR}\${PRODUCT_NAME}" "InstallDir" "$INSTDIR"
  ; Uninstall info
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "DisplayName" "${PRODUCT_NAME_VERSION}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegExpandStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "InstallLocation" "$INSTDIR"
  WriteRegExpandStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "Publisher" "${PRODUCT_VENDOR}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "URLInfoAbout" "${PRODUCT_URL}"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "NoRepair" 1
  WriteRegExpandStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteUninstaller "$INSTDIR\uninst.exe"
	
  SetAutoClose false
SectionEnd

Section !$(Shell_Extension) ShExSection_id
; install the shell extension, this might be in use
  IfFileExists 0 nodll
  ClearErrors
  Delete "$INSTDIR\FBShell.dll"
  IfErrors 0 nodll
  ; delete failed, assume destination was in use
  ;UnRegDll "$INSTDIR\FBShell.dll"
${If} ${RunningX64}
!define LIBRARY_X64
${EndIf}
  !insertmacro UnInstallLib REGDLL NOTSHARED NOREBOOT_PROTECTED "$INSTDIR\FBShell.dll"
${If} ${RunningX64}
!undef LIBRARY_X64
${EndIf}

  IfErrors 0 nodll
  MessageBox MB_OK|MB_ICONSTOP $(ErrShellIntergation)
  Quit
nodll:
  ;File "Input\FBShell.dll"

${If} ${RunningX64}
!define LIBRARY_X64
!insertmacro InstallLib REGDLL NOTSHARED REBOOT_PROTECTED "..\x64\Release\FBShell.dll" "$INSTDIR\FBShell.dll" "$INSTDIR"
!undef LIBRARY_X64
${Else}  
  !insertmacro InstallLib REGDLL NOTSHARED REBOOT_PROTECTED "..\Release\FBShell.dll" "$INSTDIR\FBShell.dll" "$INSTDIR"
${EndIf}	
  ; register application
  WriteRegStr HKCR "Applications\FBE.exe" "FriendlyAppName" "FictionBook Editor"
  WriteRegStr HKCR "Applications\FBE.exe\SupportedTypes" ".fb2" ""
  WriteRegStr HKCR "Applications\FBE.exe\SupportedTypes" ".xml" ""
  WriteRegStr HKCR "Applications\FBE.exe\shell\Open\Command" "" '"$INSTDIR\FBE.exe" "%1"'
  WriteRegStr HKCR "Applications\FBE.exe\shell\Edit\Command" "" '"$INSTDIR\FBE.exe" "%1"'
  WriteRegStr HKLM "Software\Microsoft\IE Setup\DependentComponents" "FictionBook Editor" "5.5"
	
  ; refresh icons
  System::Call 'shell32.dll::SHChangeNotify(l, l, i, i) v (${SHCNE_ASSOCCHANGED}, ${SHCNF_IDLIST}, 0, 0)'
SectionEnd

Function CreateStartMenuShortcuts
  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
  CreateDirectory "$SMPROGRAMS\$ICONS_GROUP"
  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\FictionBook Editor.lnk" "$INSTDIR\FBE.exe"
  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\Uninstall FictionBook Editor.lnk" "$INSTDIR\uninst.exe"
  !insertmacro MUI_STARTMENU_WRITE_END
FunctionEnd

Function CreateDesktopShortcut
  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
  CreateShortCut "$DESKTOP\FictionBook Editor.lnk" "$INSTDIR\FBE.exe"
  !insertmacro MUI_STARTMENU_WRITE_END
FunctionEnd

SectionGroup /e !$(ShCutGroup) ShCutGroup_id
; Shortcuts
	Section $(Start_Menu_ShortCuts) Start_Menu_ShortCuts_id	
		Call CreateStartMenuShortcuts
	SectionEnd	
	Section $(Desktop_ShortCut) Desktop_ShortCut_id
		Call CreateDesktopShortcut
	SectionEnd
SectionGroupEnd

SectionGroup !$(PluginsGroup) PluginsGroup_id
; Plugins
	Section "ExportHTML" ExportHTML_Plugin_id
		SetOutPath "$INSTDIR"
		File "Input\ExportHTML.dll"
		RegDll "$INSTDIR\ExportHTML.dll"
	SectionEnd	
SectionGroupEnd

Section !$(Scripts) Scripts_id
;Scripts and dependances
	SetOutPath "$INSTDIR\Scripts"
	File /r Input\Scripts\*.*
	SetOutPath "$INSTDIR\TreeCmd"
	File /r Input\TreeCmd\*.*
	SetOutPath "$INSTDIR\HTML"
	File /r Input\HTML\*.*
        SetOutPath "$INSTDIR\Help"
	File /r Input\Help\*.*
SectionEnd

SubSection !$(Dictionaries) Dictionaries_id

        Section $(EnglishDict) Dict01
 	  SetOutPath "$INSTDIR\Dict"
	  SectionIn RO
	  File "Input\dict\en_US.dic"
	  File "Input\dict\en_US.aff"
        SectionEnd

        Section $(RussianDict) Dict02
 	  SetOutPath "$INSTDIR\Dict"
	  SectionIn RO
	  File "Input\dict\ru_RU.dic"
	  File "Input\dict\ru_RU.aff"
        SectionEnd

        Section /o $(UkrainianDict) Dict03
	  SetOutPath "$INSTDIR\Dict"
	  File "Input\dict\uk_UA.dic"
	  File "Input\dict\uk_UA.aff"
        SectionEnd

        Section /o $(ByelorussianDict) Dict04
	  AddSize  17500
	  inetc::get /caption $(ByelorussianDict)$(Dictionary) /banner $(DownloadingDict)$(ByelorussianDict)$(Dictionary) "${SERVER}be_BY.dic"  "$INSTDIR\Dict\be_BY.dic"
	  inetc::get /caption $(ByelorussianDict)$(Dictionary) /banner $(DownloadingDict)$(ByelorussianDict)$(Dictionary) "${SERVER}be_BY.aff"  "$INSTDIR\Dict\be_BY.aff"
        SectionEnd

        Section /o $(BulgarianDict) Dict05
	  AddSize  1000
	  inetc::get /caption $(BulgarianDict)$(Dictionary) /banner $(DownloadingDict)$(BulgarianDict)$(Dictionary) "${SERVER}bg_BG.dic"  "$INSTDIR\Dict\bg_BG.dic"
	  inetc::get /caption $(BulgarianDict)$(Dictionary) /banner $(DownloadingDict)$(BulgarianDict)$(Dictionary) "${SERVER}bg_BG.aff"  "$INSTDIR\Dict\bg_BG.aff"
        SectionEnd

        Section /o $(CzechDict) Dict06
	  AddSize  3700
	  inetc::get /caption $(CzechDict)$(Dictionary) /banner $(DownloadingDict)$(CzechDict)$(Dictionary) "${SERVER}cs_CZ.dic"  "$INSTDIR\Dict\cs_CZ.dic"
	  inetc::get /caption $(CzechDict)$(Dictionary) /banner $(DownloadingDict)$(CzechDict)$(Dictionary) "${SERVER}cs_CZ.aff"  "$INSTDIR\Dict\cs_CZ.aff"
        SectionEnd

        Section /o $(PolishDict) Dict07
	  AddSize  4240
	  inetc::get /caption $(PolishDict)$(Dictionary) /banner $(DownloadingDict)$(PolishDict)$(Dictionary) "${SERVER}pl_PL.dic"  "$INSTDIR\Dict\pl_PL.dic"
	  inetc::get /caption $(PolishDict)$(Dictionary) /banner $(DownloadingDict)$(PolishDict)$(Dictionary) "${SERVER}pl_PL.aff"  "$INSTDIR\Dict\pl_PL.aff"
        SectionEnd

        Section /o $(GermanDict) Dict08
	  AddSize  2120
	  inetc::get /caption $(GermanDict)$(Dictionary) /banner $(DownloadingDict)$(GermanDict)$(Dictionary) "${SERVER}de_DE.dic"  "$INSTDIR\Dict\de_DE.dic"
	  inetc::get /caption $(GermanDict)$(Dictionary) /banner $(DownloadingDict)$(GermanDict)$(Dictionary) "${SERVER}de_DE.aff"  "$INSTDIR\Dict\de_DE.aff"
        SectionEnd

        Section /o $(FrenchDict) Dict09
	  AddSize  1100
	  inetc::get /caption $(FrenchDict)$(Dictionary) /banner $(DownloadingDict)$(FrenchDict)$(Dictionary) "${SERVER}fr_FR.dic"  "$INSTDIR\Dict\fr_FR.dic"
	  inetc::get /caption $(FrenchDict)$(Dictionary) /banner $(DownloadingDict)$(FrenchDict)$(Dictionary) "${SERVER}fr_FR.aff"  "$INSTDIR\Dict\fr_FR.aff"
        SectionEnd

        Section /o $(SpanishDict) Dict10
	  AddSize  980
	  inetc::get /caption $(SpanishDict)$(Dictionary) /banner $(DownloadingDict)$(SpanishDict)$(Dictionary) "${SERVER}es_ES.dic"  "$INSTDIR\Dict\es_ES.dic"
	  inetc::get /caption $(SpanishDict)$(Dictionary) /banner $(DownloadingDict)$(SpanishDict)$(Dictionary) "${SERVER}es_ES.aff"  "$INSTDIR\Dict\es_ES.aff"
        SectionEnd

        Section /o $(ItalianDict) Dict11
	  AddSize  1280
	  inetc::get /caption $(ItalianDict)$(Dictionary) /banner $(DownloadingDict)$(ItalianDict)$(Dictionary) "${SERVER}it_IT.dic"  "$INSTDIR\Dict\it_IT.dic"
	  inetc::get /caption $(ItalianDict)$(Dictionary) /banner $(DownloadingDict)$(ItalianDict)$(Dictionary) "${SERVER}it_IT.aff"  "$INSTDIR\Dict\it_IT.aff"
        SectionEnd

SubSectionEnd

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${MainSection_id} $(DESC_Main)
  !insertmacro MUI_DESCRIPTION_TEXT ${ShExSection_id} $(DESC_ShExSection)
  !insertmacro MUI_DESCRIPTION_TEXT ${ShCutGroup_id} $(DESC_ShCutGroup)
  !insertmacro MUI_DESCRIPTION_TEXT ${Desktop_ShortCut_id} $(DESC_Desktop_ShortCut)
  !insertmacro MUI_DESCRIPTION_TEXT ${Start_Menu_ShortCuts_id} $(DESC_Start_Menu_ShortCuts)
  !insertmacro MUI_DESCRIPTION_TEXT ${PluginsGroup_id} $(DESC_PluginsGroup)
  !insertmacro MUI_DESCRIPTION_TEXT ${Scripts_id} $(DESC_Scripts)
  !insertmacro MUI_DESCRIPTION_TEXT ${Dictionaries_id} $(DESC_Dictionaries)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

Function un.DeleteShortcuts
  !insertmacro MUI_STARTMENU_GETFOLDER Application $ICONS_GROUP
  Delete "$DESKTOP\FictionBook Editor.lnk"
  Delete "$SMPROGRAMS\$ICONS_GROUP\Uninstall FictionBook Editor.lnk"
  Delete "$SMPROGRAMS\$ICONS_GROUP\FictionBook Editor.lnk"
  RMDir "$SMPROGRAMS\$ICONS_GROUP"
FunctionEnd

Section Uninstall

  Call un.CheckFBERunning

  ; remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
  DeleteRegKey HKLM "SOFTWARE\${PRODUCT_VENDOR}\${PRODUCT_NAME}"
  DeleteRegValue HKLM "Software\Microsoft\IE Setup\DependentComponents" "FictionBook Editor"

  ; remove typelib entry
  DeleteRegKey HKCR "Interface\{7269066E-2089-4408-B3F3-E8D75984D5A6}"
  DeleteRegKey HKCR "TypeLib\{37B16C7D-4400-4D7D-AA35-14C74E265EA4}"

  ; unregister typelib
;  Push "$INSTDIR\FBE.exe"
;  Call Un.RegisterTlb

  ; remove shell extension
${If} ${RunningX64}
!define LIBRARY_X64
${EndIf}
  !insertmacro UnInstallLib REGDLL NOTSHARED REBOOT_PROTECTED "$INSTDIR\FBShell.dll"
${If} ${RunningX64}
!undef LIBRARY_X64
${EndIf}  

  ; remove plugin
  UnRegDll "$INSTDIR\ExportHTML.dll"

  ; remove applications
  DeleteRegKey HKCR "Applications\FBE.exe"

  ; remove verbs; TODO: check if these really point to FBE
  DeleteRegKey HKCR "FictionBook.2\shell\Edit"
  
  Delete "$INSTDIR\uninst.exe"
  Delete "$INSTDIR\SciLexer.dll"
  Delete "$INSTDIR\pcre.dll"
  Delete "$INSTDIR\pcre2-8.dll"
  Delete "$INSTDIR\libHunspell.dll"  
  Delete "$INSTDIR\main.js"
  Delete "$INSTDIR\main.html"
  Delete "$INSTDIR\main.css"
  Delete "$INSTDIR\main_fast.css"
  Delete "$INSTDIR\imgprev.html"
  Delete "$INSTDIR\genres.txt"
  Delete "$INSTDIR\genres.rus.txt"
  Delete "$INSTDIR\genres.ukr.txt"
  Delete "$INSTDIR\genres.txt_L"
  Delete "$INSTDIR\genres.rus.txt_L"
  Delete "$INSTDIR\FictionBookLinks.xsd"
  Delete "$INSTDIR\FictionBookLang.xsd"
  Delete "$INSTDIR\FictionBookGenres.xsd"
  Delete "$INSTDIR\FictionBook.xsd"
  Delete "$INSTDIR\fb2.xsl"
  Delete "$INSTDIR\eng.xsl"
  Delete "$INSTDIR\rus.xsl"
  Delete "$INSTDIR\ukr.xsl"
  Delete "$INSTDIR\html.xsl"
  Delete "$INSTDIR\ExportHTML.dll"
  Delete "$INSTDIR\gdiplus.manifest"
  Delete "$INSTDIR\gdiplus.dll"
  Delete "$INSTDIR\gdiplus.cat"
  
  Delete "$INSTDIR\gpl-3.0.txt"
  Delete "$INSTDIR\gpl-3.0.ru.txt"
  Delete "$INSTDIR\gpl-3.0.ua.txt"

  Delete "$INSTDIR\contributors.txt"
  Delete "$INSTDIR\copying.txt"

  Delete "$INSTDIR\symbols.ini"

  ;Scripts
  RMDir /r "$INSTDIR\Dict"
  RMDir /r "$INSTDIR\Scripts"
  RMDir /r "$INSTDIR\TreeCmd"
  RMDir /r "$INSTDIR\HTML"
  RMDir /r "$INSTDIR\Help"
  RMDir /r "$INSTDIR\img"

  Delete "$INSTDIR\blank.fb2"
  Delete "$INSTDIR\FBV.exe"
  Delete "$INSTDIR\res_rus.dll"
  Delete "$INSTDIR\res_ukr.dll"
  
  Delete "$INSTDIR\CHANGELOG.md"
  Delete "$INSTDIR\NOWTO.md"  
  Delete "$INSTDIR\HOWTO.md"
  Delete "$INSTDIR\README.md"
  Delete "$INSTDIR\TODO.md"

  RMDir /r "$INSTDIR\en-US"
  RMDir /r "$INSTDIR\ru-RU"
  RMDir /r "$INSTDIR\uk-UA"

  !insertmacro UnInstallLib TLB NOTSHARED REBOOT_PROTECTED "$INSTDIR\FBE.exe"

; Delete program settings
  MessageBox MB_YESNO $(UninstAskSettings) IDNO DoNotDelete

    Call un.GetUserAppData
    Pop $0
    Delete "$0\FBE\Hotkeys.xml"
    Delete "$0\FBE\Settings.xml"
    Delete "$0\FBE\Words.xml"
    RMDir /r "$0\FBE"

    DeleteRegKey HKEY_CURRENT_USER "SOFTWARE\FBETeam"

DoNotDelete:

  !insertmacro UnInstallLib REGDLL NOTSHARED REBOOT_PROTECTED "$INSTDIR\FBShell.dll"

  Call un.DeleteShortcuts

  RMDir "$INSTDIR"

  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
  SetAutoClose false
SectionEnd

Function .onInit 
  !insertmacro MUI_LANGDLL_DISPLAY
FunctionEnd