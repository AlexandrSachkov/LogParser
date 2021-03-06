; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "Line Catcher"
#define MyAppVersion "1.1"
#define MyAppPublisher "Alexandr Sachkov"
#define MyAppURL "https://github.com/AlexandrSachkov/LogParser"
#define MyAppExeName "LineCatcher.exe"

#define QtInstallDir "D:\Qt\5.12.0\msvc2017_64"
#define LCCoreBuildDir "D:\Repositories\LogParser\bin64_core\Release"
#define LCUIBuildDir "D:\Repositories\LogParser\src\ui\build-PLP-Desktop_Qt_5_12_0_MSVC2017_64bit-Release\release"
#define MSVCDir "D:\Programs\Microsoft Visual Studio\2017\Community\VC\Redist\MSVC\14.16.27012"

[Setup]
; NOTE: The value of AppId uniquely identifies this application. Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{0391232B-C4A3-4235-8751-E5C39DE9E8F8}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={autopf}\{#MyAppName}
DisableProgramGroupPage=yes
LicenseFile=..\License.txt
InfoBeforeFile=Short Description.txt
InfoAfterFile=Thank you.txt
; Uncomment the following line to run in non administrative install mode (install for current user only.)
;PrivilegesRequired=lowest
SetupIconFile=..\resources\images\icon_xl.ico
UninstallDisplayIcon={app}\{#MyAppExeName}
Compression=lzma
SolidCompression=yes
WizardStyle=modern
OutputBaseFilename=LC_Setup

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "{#LCUIBuildDir}\LineCatcher.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#LCUIBuildDir}\LineCatcher_resource.res"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\resources\*"; DestDir: "{app}\resources"; Flags: ignoreversion recursesubdirs
Source: "..\scripts\*"; DestDir: "{app}\scripts"; Flags: ignoreversion recursesubdirs
Source: "..\License.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\README.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#LCCoreBuildDir}\lcCore.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#LCUIBuildDir}\lua51.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#LCUIBuildDir}\jit\*"; DestDir: "{app}\jit"; Flags: ignoreversion recursesubdirs
Source: "{#LCUIBuildDir}\luajit.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#LCUIBuildDir}\tbb.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#MSVCDir}\x64\Microsoft.VC141.CRT\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs
Source: "{#QtInstallDir}\bin\Qt5Core.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#QtInstallDir}\bin\Qt5Gui.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#QtInstallDir}\bin\Qt5Widgets.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#QtInstallDir}\plugins\styles\qwindowsvistastyle.dll"; DestDir: "{app}\styles"; Flags: ignoreversion
Source: "{#QtInstallDir}\plugins\platforms\qwindows.dll"; DestDir: "{app}\platforms"; Flags: ignoreversion
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{autoprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

