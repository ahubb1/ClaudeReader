; ClaudeReader installer (NSIS 3, Unicode)
;
; Installs for the current user (no administrator rights): %LOCALAPPDATA%\Programs\ClaudeReader,
; Start menu shortcut, optional desktop shortcut and comic file types, uninstaller in Apps & features.
; The library, settings and tags (%APPDATA%\ClaudeReader) are not touched, nor removed by the uninstaller.
;
; Built by installer/build-installer.sh:
;   makensis /DVERSION=1.2 /DVIVERSION=1.2.0.0 /DSTAGE=<folder with the files> /DOUTFILE=<setup.exe> ClaudeReader.nsi
;
; Updates (from ClaudeReader): "setup.exe /S /RELAUNCH" waits for ClaudeReader to close, installs over it with
; the choices of the first installation, then starts it again.

Unicode true
SetCompressor /SOLID lzma
ManifestDPIAware true

!include "MUI2.nsh"
!include "LogicLib.nsh"
!include "FileFunc.nsh"

!ifndef VERSION
    !define VERSION "0.0.0"
!endif
!ifndef VIVERSION
    !define VIVERSION "0.0.0.0"
!endif
!ifndef STAGE
    !define STAGE "stage"
!endif
!ifndef OUTFILE
    !define OUTFILE "ClaudeReader-windows-x64-setup.exe"
!endif

!define APP "ClaudeReader"
!define EXE "ClaudeReader.exe"
!define UNINSTALLER "Uninstall ClaudeReader.exe"
!define PUBLISHER "ClaudeReader contributors"
!define URL "https://github.com/ahubb1/ClaudeReader"
!define REGKEY "Software\ClaudeReader"
!define UNINSTKEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\ClaudeReader"
!define PROGID "ClaudeReader.Comic"

Name "${APP}"
Caption "${APP} ${VERSION} Setup"
OutFile "${OUTFILE}"
InstallDir "$LOCALAPPDATA\Programs\${APP}"
InstallDirRegKey HKCU "${REGKEY}" "InstallDir"
RequestExecutionLevel user
BrandingText "${APP} ${VERSION}"

VIProductVersion "${VIVERSION}"
VIAddVersionKey "ProductName" "${APP}"
VIAddVersionKey "ProductVersion" "${VERSION}"
VIAddVersionKey "FileVersion" "${VERSION}"
VIAddVersionKey "FileDescription" "${APP} installer"
VIAddVersionKey "CompanyName" "${PUBLISHER}"
VIAddVersionKey "LegalCopyright" "GPLv3 or later"

!define MUI_ICON "${STAGE}\ClaudeReader.ico"
!define MUI_UNICON "${STAGE}\ClaudeReader.ico"
!define MUI_ABORTWARNING
!define MUI_WELCOMEPAGE_TEXT "This will install ${APP} ${VERSION} for your Windows account (no administrator rights needed).$\r$\n$\r$\nYour library, settings and tags are kept. If ${APP} is open, close it first.$\r$\n$\r$\nClick Next to continue."
!define MUI_COMPONENTSPAGE_SMALLDESC
!define MUI_FINISHPAGE_RUN "$INSTDIR\${EXE}"
!define MUI_FINISHPAGE_RUN_TEXT "Start ${APP}"
!define MUI_UNCONFIRMPAGE_TEXT_TOP "${APP} will be removed from your computer. Your library, settings and tags are kept."

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "${STAGE}\LICENSE.txt"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_LANGUAGE "English"

Var Relaunch    ; 1: started by the ClaudeReader update (start it again at the end)

; Wait until ClaudeReader is closed (its exe can be written): 20 s, then ask
!macro WAIT_FOR_CLOSE prefix
Function ${prefix}WaitForClose
    StrCpy $1 0
    loop:
        ${IfNot} ${FileExists} "$INSTDIR\${EXE}"
            Return
        ${EndIf}
        ClearErrors
        FileOpen $0 "$INSTDIR\${EXE}" a
        ${IfNot} ${Errors}
            FileClose $0
            Return
        ${EndIf}
        IntOp $1 $1 + 1
        ${If} $1 < 40
            Sleep 500
            Goto loop
        ${EndIf}
        MessageBox MB_RETRYCANCEL|MB_ICONEXCLAMATION "${APP} is running. Close it, then click Retry." /SD IDCANCEL IDRETRY retry
        Abort
    retry:
        StrCpy $1 30
        Goto loop
FunctionEnd
!macroend
!insertmacro WAIT_FOR_CLOSE ""
!insertmacro WAIT_FOR_CLOSE "un."

; A comic file type: "Open with" ClaudeReader, and opened by it when no other program is its default
!macro ASSOCIATE ext
    WriteRegStr HKCU "Software\Classes\${ext}\OpenWithProgids" "${PROGID}" ""
    WriteRegStr HKCU "Software\Classes\Applications\${EXE}\SupportedTypes" "${ext}" ""
    ReadRegStr $0 HKCR "${ext}" ""
    ${If} $0 == ""
        WriteRegStr HKCU "Software\Classes\${ext}" "" "${PROGID}"
    ${EndIf}
!macroend

!macro UNASSOCIATE ext
    DeleteRegValue HKCU "Software\Classes\${ext}\OpenWithProgids" "${PROGID}"
    ReadRegStr $0 HKCU "Software\Classes\${ext}" ""
    ${If} $0 == "${PROGID}"
        DeleteRegValue HKCU "Software\Classes\${ext}" ""
    ${EndIf}
    ; Only when nothing else is in them (another program's entries are kept)
    DeleteRegKey /ifempty /ifnovalues HKCU "Software\Classes\${ext}\OpenWithProgids"
    DeleteRegKey /ifempty /ifnovalues HKCU "Software\Classes\${ext}"
!macroend

Section "${APP}" SecMain
    SectionIn RO
    Call WaitForClose
    SetOutPath "$INSTDIR"
    File "${STAGE}\${EXE}"
    File "${STAGE}\README.md"
    File "${STAGE}\CHANGES.txt"
    File "${STAGE}\LICENSE.txt"
    WriteUninstaller "$INSTDIR\${UNINSTALLER}"

    CreateShortcut "$SMPROGRAMS\${APP}.lnk" "$INSTDIR\${EXE}" "" "$INSTDIR\${EXE}" 0 SW_SHOWNORMAL "" "Read comics, manga and webtoons"
    ; Win+R "ClaudeReader"
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\App Paths\${EXE}" "" "$INSTDIR\${EXE}"

    WriteRegStr HKCU "${REGKEY}" "InstallDir" "$INSTDIR"
    WriteRegStr HKCU "${REGKEY}" "Version" "${VERSION}"

    ; Apps & features
    WriteRegStr HKCU "${UNINSTKEY}" "DisplayName" "${APP}"
    WriteRegStr HKCU "${UNINSTKEY}" "DisplayVersion" "${VERSION}"
    WriteRegStr HKCU "${UNINSTKEY}" "Publisher" "${PUBLISHER}"
    WriteRegStr HKCU "${UNINSTKEY}" "DisplayIcon" "$INSTDIR\${EXE},0"
    WriteRegStr HKCU "${UNINSTKEY}" "InstallLocation" "$INSTDIR"
    WriteRegStr HKCU "${UNINSTKEY}" "UninstallString" '"$INSTDIR\${UNINSTALLER}"'
    WriteRegStr HKCU "${UNINSTKEY}" "QuietUninstallString" '"$INSTDIR\${UNINSTALLER}" /S'
    WriteRegStr HKCU "${UNINSTKEY}" "URLInfoAbout" "${URL}"
    WriteRegStr HKCU "${UNINSTKEY}" "HelpLink" "${URL}/issues"
    WriteRegStr HKCU "${UNINSTKEY}" "URLUpdateInfo" "${URL}/releases"
    WriteRegDWORD HKCU "${UNINSTKEY}" "NoModify" 1
    WriteRegDWORD HKCU "${UNINSTKEY}" "NoRepair" 1
    ${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
    IntFmt $0 "0x%08X" $0
    WriteRegDWORD HKCU "${UNINSTKEY}" "EstimatedSize" "$0"
SectionEnd

Section "Desktop shortcut" SecDesktop
    ; An update does not bring back a shortcut the user removed
    ${If} $Relaunch == 1
    ${AndIfNot} ${FileExists} "$DESKTOP\${APP}.lnk"
        Goto done
    ${EndIf}
    CreateShortcut "$DESKTOP\${APP}.lnk" "$INSTDIR\${EXE}" "" "$INSTDIR\${EXE}" 0
    done:
SectionEnd

Section "Open comic files with ${APP}" SecAssoc
    WriteRegStr HKCU "Software\Classes\${PROGID}" "" "Comic book"
    WriteRegStr HKCU "Software\Classes\${PROGID}\DefaultIcon" "" "$INSTDIR\${EXE},0"
    WriteRegStr HKCU "Software\Classes\${PROGID}\shell\open\command" "" '"$INSTDIR\${EXE}" "%1"'
    WriteRegStr HKCU "Software\Classes\Applications\${EXE}\shell\open\command" "" '"$INSTDIR\${EXE}" "%1"'
    WriteRegStr HKCU "Software\Classes\Applications\${EXE}" "FriendlyAppName" "${APP}"
    !insertmacro ASSOCIATE ".cbz"
    !insertmacro ASSOCIATE ".cbr"
    !insertmacro ASSOCIATE ".cb7"
    !insertmacro ASSOCIATE ".cbt"
    ; Without becoming their default program
    WriteRegStr HKCU "Software\Classes\.pdf\OpenWithProgids" "${PROGID}" ""
    WriteRegStr HKCU "Software\Classes\.epub\OpenWithProgids" "${PROGID}" ""
    WriteRegStr HKCU "Software\Classes\Applications\${EXE}\SupportedTypes" ".pdf" ""
    WriteRegStr HKCU "Software\Classes\Applications\${EXE}\SupportedTypes" ".epub" ""
    System::Call 'shell32::SHChangeNotify(i 0x08000000, i 0, p 0, p 0)'   ; SHCNE_ASSOCCHANGED
SectionEnd

; Remember the choices (used by the updates)
Section "-Choices"
    ${If} ${SectionIsSelected} ${SecDesktop}
        WriteRegDWORD HKCU "${REGKEY}" "DesktopShortcut" 1
    ${Else}
        WriteRegDWORD HKCU "${REGKEY}" "DesktopShortcut" 0
    ${EndIf}
    ${If} ${SectionIsSelected} ${SecAssoc}
        WriteRegDWORD HKCU "${REGKEY}" "FileTypes" 1
    ${Else}
        WriteRegDWORD HKCU "${REGKEY}" "FileTypes" 0
    ${EndIf}
SectionEnd

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecMain} "The ${APP} program (single file, about 75 MB)."
    !insertmacro MUI_DESCRIPTION_TEXT ${SecDesktop} "A ${APP} shortcut on the desktop."
    !insertmacro MUI_DESCRIPTION_TEXT ${SecAssoc} "Open .cbz, .cbr, .cb7 and .cbt comics with ${APP} (when no other program opens them), and add ${APP} to Open with for PDF and EPUB files."
!insertmacro MUI_FUNCTION_DESCRIPTION_END

Function .onInit
    ${GetParameters} $0
    ClearErrors
    ${GetOptions} $0 "/RELAUNCH" $1
    ${IfNot} ${Errors}
        StrCpy $Relaunch 1
    ${EndIf}
    ; Installed before: the same choices
    ClearErrors
    ReadRegDWORD $0 HKCU "${REGKEY}" "DesktopShortcut"
    ${IfNot} ${Errors}
    ${AndIf} $0 == 0
        !insertmacro UnselectSection ${SecDesktop}
    ${EndIf}
    ClearErrors
    ReadRegDWORD $0 HKCU "${REGKEY}" "FileTypes"
    ${IfNot} ${Errors}
    ${AndIf} $0 == 0
        !insertmacro UnselectSection ${SecAssoc}
    ${EndIf}
FunctionEnd

Function .onInstSuccess
    ${If} $Relaunch == 1
        Exec '"$INSTDIR\${EXE}"'
    ${EndIf}
FunctionEnd

Section "Uninstall"
    Call un.WaitForClose
    Delete "$INSTDIR\${EXE}"
    Delete "$INSTDIR\${EXE}.old"
    Delete "$INSTDIR\README.md"
    Delete "$INSTDIR\CHANGES.txt"
    Delete "$INSTDIR\LICENSE.txt"
    Delete "$INSTDIR\${UNINSTALLER}"
    RMDir "$INSTDIR"

    Delete "$SMPROGRAMS\${APP}.lnk"
    Delete "$DESKTOP\${APP}.lnk"

    !insertmacro UNASSOCIATE ".cbz"
    !insertmacro UNASSOCIATE ".cbr"
    !insertmacro UNASSOCIATE ".cb7"
    !insertmacro UNASSOCIATE ".cbt"
    !insertmacro UNASSOCIATE ".pdf"
    !insertmacro UNASSOCIATE ".epub"
    DeleteRegKey HKCU "Software\Classes\${PROGID}"
    DeleteRegKey HKCU "Software\Classes\Applications\${EXE}"
    System::Call 'shell32::SHChangeNotify(i 0x08000000, i 0, p 0, p 0)'

    DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\App Paths\${EXE}"
    DeleteRegKey HKCU "${UNINSTKEY}"
    DeleteRegKey HKCU "${REGKEY}"
SectionEnd
