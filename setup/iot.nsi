;signtool sign /f C:\Users\duffyj\source\repos\jde\Private\certs\jde-cpp.pfx /fd SHA256 Iot.exe
Outfile "Iot.exe"

InstallDir  $PROGRAMFILES64\jde-cpp

Section
  !include "winmessages.nsh"
  !define env_hklm 'HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment"'
  WriteRegExpandStr ${env_hklm} Jde_Iot_Connection DSN=jde_iot
  WriteRegExpandStr ${env_hklm} Jde_App_Connection DSN=jde_app
  SendMessage ${HWND_BROADCAST} ${WM_WININICHANGE} 0 "STR:Environment" /TIMEOUT=5000
SectionEnd
Section
  SetOutPath $INSTDIR\Iot
  File /r C:\Users\duffyj\source\repos\jde\IotWebsocket\source\.build\.bin\RelWithDebInfo\*.exe
SectionEnd
Section
  SetOutPath $INSTDIR\Iot
  File /r C:\Users\duffyj\source\repos\jde\IotWebsocket\source\.build\.bin\RelWithDebInfo\*.dll
SectionEnd
Section
  SetOutPath $INSTDIR\Iot
  File /r C:\Users\duffyj\source\repos\jde\installed\RelWithDebInfo\bin\*.dll
SectionEnd
Section
  SetOutPath $INSTDIR\Iot
  File "C:\Users\duffyj\source\repos\libraries\vcpkg\installed\x64-windows\bin\zlib1.dll"
SectionEnd
Section
  SetOutPath $INSTDIR\Iot
  File "C:\Program Files\OpenSSL-Win64\bin\libcrypto-3-x64.dll"
SectionEnd
Section
  SetOutPath $INSTDIR\Iot
  File "C:\Program Files\OpenSSL-Win64\bin\libssl-3-x64.dll"
SectionEnd
Section
  SetShellVarContext all
  SetOutPath $LOCALAPPDATA\jde-cpp\Iot
  File /r C:\Users\duffyj\source\repos\jde\IotWebsocket\config\*.json
SectionEnd
Section
  SetShellVarContext all
  SetOutPath $LOCALAPPDATA\jde-cpp\Iot\sql
  File /r C:\Users\duffyj\source\repos\jde\IotWebsocket\sql\*ms.sql
SectionEnd
Section
  SetOutPath $INSTDIR\AppServer
  File /r C:\Users\duffyj\source\repos\jde\AppServer\.build\.bin\RelWithDebInfo\*.exe
SectionEnd
Section
  SetOutPath $INSTDIR\AppServer
  File /r C:\Users\duffyj\source\repos\jde\AppServer\.build\.bin\RelWithDebInfo\*.dll
SectionEnd
Section
  SetOutPath $INSTDIR\AppServer
  File /r C:\Users\duffyj\source\repos\jde\installed\RelWithDebInfo\bin\*.dll
SectionEnd
Section
  SetOutPath $INSTDIR\AppServer
  File "C:\Users\duffyj\source\repos\libraries\vcpkg\installed\x64-windows\bin\zlib1.dll"
SectionEnd
Section
  SetOutPath $INSTDIR\AppServer
  File "C:\Program Files\OpenSSL-Win64\bin\libcrypto-3-x64.dll"
SectionEnd
Section
  SetOutPath $INSTDIR\AppServer
  File "C:\Program Files\OpenSSL-Win64\bin\libssl-3-x64.dll"
SectionEnd
Section
  SetShellVarContext all
  SetOutPath $LOCALAPPDATA\jde-cpp\AppServer
  File /r C:\Users\duffyj\source\repos\jde\AppServer\config\*.json
SectionEnd
Section
  SetShellVarContext all
  SetOutPath $LOCALAPPDATA\jde-cpp\AppServer\sql
  File /r C:\Users\duffyj\source\repos\jde\AppServer\sql\*ms.sql
SectionEnd
Section
  SetShellVarContext all
  SetOutPath $LOCALAPPDATA\jde-cpp\IotWeb
  File /r C:\Users\duffyj\source\repos\jde\web\my-workspace\browser\**.*
SectionEnd
Section
  SetShellVarContext all
  SetOutPath $DESKTOP
  File /r C:\Users\duffyj\source\repos\jde\IotWebsocket\scripts\*.ps1
SectionEnd
;Section "Visual Studio Runtime"
;  SetOutPath "$INSTDIR"
;  File "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Redist\MSVC\14.40.33807\vc_redist.x64.exe"
;  ExecWait "$INSTDIR\vcredist_x64.exe"
;  Delete "$INSTDIR\vcredist_x64.exe"
;SectionEnd
;Section "Uninstall"
  ;xDelete $INSTDIR\Iot\*.*
  ;xDelete $INSTDIR\AppServer\*.*
;  RMDir $INSTDIR\Iot
;  RMDir $INSTDIR\AppServer
;  RMDir $LOCALAPPDATA\jde-cpp\AppServer
;  RMDir $LOCALAPPDATA\jde-cpp\Iot
;  Delete $DESKTOP\JdeDBSetup.ps1
;  Delete $DESKTOP\IotWeb.bat
;SectionEnd