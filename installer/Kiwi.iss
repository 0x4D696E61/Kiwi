#define AppVersion "0.1.0"

[Setup]
AppId=0x4D696E61.Kiwi
AppName=Kiwi
AppVersion={#AppVersion}
AppVerName=Kiwi Version {#AppVersion}
UninstallDisplayName=Kiwi Version {#AppVersion}
AppPublisher=0x4D696E61
AppPublisherURL=https://github.com/0x4D696E61/Kiwi
DefaultDirName={localappdata}\Programs\Kiwi
DefaultGroupName=Kiwi
PrivilegesRequired=lowest
OutputDir=..\dist
OutputBaseFilename=Kiwi-Setup-{#AppVersion}
Compression=lzma
SolidCompression=yes
WizardStyle=modern
ArchitecturesAllowed=x64compatible
SetupIconFile=..\assets\kiwi.ico
UninstallDisplayIcon={app}\kiwi.ico

[Tasks]
Name: "desktopicon"; Description: "Create a desktop shortcut"; Flags: unchecked

[Files]
Source: "..\build\debug\kiwi.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\build\debug\WinSparkle.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\msys64\clang64\bin\libc++.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\assets\kiwi.ico"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{autoprograms}\Kiwi"; Filename: "{app}\kiwi.exe"; IconFilename: "{app}\kiwi.ico"
Name: "{autodesktop}\Kiwi"; Filename: "{app}\kiwi.exe"; IconFilename: "{app}\kiwi.ico"; Tasks: desktopicon
