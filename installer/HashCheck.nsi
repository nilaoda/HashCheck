SetCompressor /FINAL /SOLID lzma

!include MUI2.nsh
!include x64.nsh

Unicode true

Name "HashCheck"
!ifndef OUTFILE_SUFFIX
!define OUTFILE_SUFFIX ""
!endif
OutFile "HashCheckSetup-v2.4.0.56${OUTFILE_SUFFIX}.exe"

RequestExecutionLevel admin
ManifestSupportedOS all

!define MUI_ICON ..\HashCheck.ico
!define MUI_ABORTWARNING

!insertmacro MUI_PAGE_WELCOME
!define MUI_PAGE_CUSTOMFUNCTION_SHOW change_license_font
!insertmacro MUI_PAGE_LICENSE "..\license.txt"
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

; Change to a smaller monospace font to more nicely display the license
;
Function change_license_font
    FindWindow $0 "#32770" "" $HWNDPARENT
    CreateFont $1 "Lucida Console" "7"
    GetDlgItem $0 $0 1000
    SendMessage $0 ${WM_SETFONT} $1 1
FunctionEnd

; These are the same languages supported by HashCheck
;
!insertmacro MUI_LANGUAGE "English" ; the first language is the default
!insertmacro MUI_LANGUAGE "TradChinese"
!insertmacro MUI_LANGUAGE "SimpChinese"
!insertmacro MUI_LANGUAGE "Czech"
!insertmacro MUI_LANGUAGE "German"
!insertmacro MUI_LANGUAGE "Greek"
!insertmacro MUI_LANGUAGE "SpanishInternational"
!insertmacro MUI_LANGUAGE "French"
!insertmacro MUI_LANGUAGE "Italian"
!insertmacro MUI_LANGUAGE "Japanese"
!insertmacro MUI_LANGUAGE "Korean"
!insertmacro MUI_LANGUAGE "Dutch"
!insertmacro MUI_LANGUAGE "Polish"
!insertmacro MUI_LANGUAGE "PortugueseBR"
!insertmacro MUI_LANGUAGE "Portuguese"
!insertmacro MUI_LANGUAGE "Romanian"
!insertmacro MUI_LANGUAGE "Russian"
!insertmacro MUI_LANGUAGE "Swedish"
!insertmacro MUI_LANGUAGE "Turkish"
!insertmacro MUI_LANGUAGE "Ukrainian"
!insertmacro MUI_LANGUAGE "Catalan"

VIProductVersion "2.4.0.56"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductName" "HashCheck Shell Extension"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductVersion" "2.4.0.56"
VIAddVersionKey /LANG=${LANG_ENGLISH} "Comments" "Installer distributed from https://github.com/gurnec/HashCheck/releases"
VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalCopyright" "Copyright © 2008-2016 Kai Liu, Christopher Gurnee, Tim Schlueter, et al. All rights reserved."
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileDescription" "Installer (x64/ARM64) from https://github.com/gurnec/HashCheck/releases"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileVersion" "2.4.0.56"

; With solid compression, files that are required before the
; actual installation should be stored first in the data block,
; because this will make the installer start faster.
;
!insertmacro MUI_RESERVEFILE_LANGDLL

; The install script
;
Section

    !ifdef ARCH_ARM64
        ${DisableX64FSRedirection}
        SetOutPath "$WINDIR\System32\ShellExt"
        File /oname=$WINDIR\System32\ShellExt\HashCheck.dll ..\Bin\ARM64\Release\HashCheck.dll
        IfFileExists "$WINDIR\System32\ShellExt\HashCheck.dll" +2 0
        Goto abort_on_error
        ExecWait '"$WINDIR\System32\regsvr32.exe" /i /n /s "$WINDIR\System32\ShellExt\HashCheck.dll"' $0
        ${If} $0 != 0
            MessageBox MB_ICONSTOP|MB_OK "Registration failed (code $0)."
            Quit
        ${EndIf}
        ${EnableX64FSRedirection}
    !else
        ${If} ${RunningX64}
            ${DisableX64FSRedirection}

            ; Install the 64-bit dll
            SetOutPath "$WINDIR\System32\ShellExt"
            File /oname=$WINDIR\System32\ShellExt\HashCheck.dll ..\Bin\x64\Release\HashCheck.dll
            IfFileExists "$WINDIR\System32\ShellExt\HashCheck.dll" +2 0
            Goto abort_on_error
            ExecWait '"$WINDIR\System32\regsvr32.exe" /i /n /s "$WINDIR\System32\ShellExt\HashCheck.dll"' $0
            ${If} $0 != 0
                MessageBox MB_ICONSTOP|MB_OK "Registration failed (code $0)."
                Quit
            ${EndIf}
            ; One of these 64-bit dlls exists and is undeletable if and
            ; only if it was in use and therefore a reboot is now required
            Delete /REBOOTOK $WINDIR\System32\ShellExt\HashCheck.dll.0
            Delete /REBOOTOK $WINDIR\System32\ShellExt\HashCheck.dll.1
            Delete /REBOOTOK $WINDIR\System32\ShellExt\HashCheck.dll.2
            Delete /REBOOTOK $WINDIR\System32\ShellExt\HashCheck.dll.3
            Delete /REBOOTOK $WINDIR\System32\ShellExt\HashCheck.dll.4
            Delete /REBOOTOK $WINDIR\System32\ShellExt\HashCheck.dll.5
            Delete /REBOOTOK $WINDIR\System32\ShellExt\HashCheck.dll.6
            Delete /REBOOTOK $WINDIR\System32\ShellExt\HashCheck.dll.7
            Delete /REBOOTOK $WINDIR\System32\ShellExt\HashCheck.dll.8
            Delete /REBOOTOK $WINDIR\System32\ShellExt\HashCheck.dll.9

            ${EnableX64FSRedirection}
        ${Else}
            MessageBox MB_ICONSTOP|MB_OK "This installer requires 64-bit Windows."
            Quit
        ${EndIf}
    !endif

    Delete /REBOOTOK $WINDIR\System32\ShellExt\HashCheck.dll.0
    Delete /REBOOTOK $WINDIR\System32\ShellExt\HashCheck.dll.1
    Delete /REBOOTOK $WINDIR\System32\ShellExt\HashCheck.dll.2
    Delete /REBOOTOK $WINDIR\System32\ShellExt\HashCheck.dll.3
    Delete /REBOOTOK $WINDIR\System32\ShellExt\HashCheck.dll.4
    Delete /REBOOTOK $WINDIR\System32\ShellExt\HashCheck.dll.5
    Delete /REBOOTOK $WINDIR\System32\ShellExt\HashCheck.dll.6
    Delete /REBOOTOK $WINDIR\System32\ShellExt\HashCheck.dll.7
    Delete /REBOOTOK $WINDIR\System32\ShellExt\HashCheck.dll.8
    Delete /REBOOTOK $WINDIR\System32\ShellExt\HashCheck.dll.9

    Return

    abort_on_error:
        IfSilent +2
        MessageBox MB_ICONSTOP|MB_OK "An unexpected error occurred during installation"
        Quit

SectionEnd

Function .onInit
    SetShellVarContext current
    System::Call 'kernel32::SetEnvironmentVariable(t, t) i("TEMP", "$LOCALAPPDATA\\Temp")'
    System::Call 'kernel32::SetEnvironmentVariable(t, t) i("TMP", "$LOCALAPPDATA\\Temp")'
    !insertmacro MUI_LANGDLL_DISPLAY
FunctionEnd
