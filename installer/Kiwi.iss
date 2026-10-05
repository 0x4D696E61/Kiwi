#define AppVersion "0.2.08"

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
ChangesEnvironment=yes
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
Source: "..\build\debug\winsparkle-tool.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\msys64\clang64\bin\libc++.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\assets\kiwi.ico"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{autoprograms}\Kiwi"; Filename: "{app}\kiwi.exe"; WorkingDir: "{%USERPROFILE}"; IconFilename: "{app}\kiwi.ico"
Name: "{autodesktop}\Kiwi"; Filename: "{app}\kiwi.exe"; WorkingDir: "{%USERPROFILE}"; IconFilename: "{app}\kiwi.ico"; Tasks: desktopicon

[Code]
const
  EnvironmentKey = 'Environment';

function PathHas(Path: string; Item: string): Boolean;
begin
  Result := Pos(
    ';' + Uppercase(Item) + ';',
    ';' + Uppercase(Path) + ';'
  ) > 0;
end;

procedure AddKiwiPath;
var
  Path: string;
  AppPath: string;
begin
  AppPath := ExpandConstant('{app}');

  if not RegQueryStringValue(HKEY_CURRENT_USER, EnvironmentKey, 'Path', Path) then
    Path := '';

  if PathHas(Path, AppPath) then exit;

  if (Path <> '') and (Path[Length(Path)] <> ';') then
    Path := Path + ';';

  Path := Path + AppPath;

  RegWriteStringValue(HKEY_CURRENT_USER, EnvironmentKey, 'Path', Path);
end;

procedure RemoveKiwiPath;
var
  Path: string;
  AppPath: string;
  P: Integer;
begin
  AppPath := ExpandConstant('{app}');

  if not RegQueryStringValue(HKEY_CURRENT_USER, EnvironmentKey, 'Path', Path) then exit;

  P := Pos(
    ';' + Uppercase(AppPath) + ';',
    ';' + Uppercase(Path) + ';'
  );

  if P = 0 then exit;

  if P > 1 then P := P - 1;

  Delete(Path, P, Length(AppPath) + 1);

  RegWriteStringValue(HKEY_CURRENT_USER, EnvironmentKey, 'Path', Path);
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
  if CurStep = ssPostInstall then AddKiwiPath;
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  if CurUninstallStep = usUninstall then RemoveKiwiPath;
end;