<p align="center">
  <img src="assets/kiwi.png" alt="Kiwi Logo" width="160">
</p>

<h1 align="center">Kiwi 🥝</h1>

<p align="center">
  A lightweight, keyboard-driven terminal code editor written in C++23 for Windows.
</p>

<p align="center">
  <a href="https://github.com/0x4D696E61/Kiwi/releases/latest">
    <img src="https://img.shields.io/github/v/release/0x4D696E61/Kiwi?label=release">
  </a>
  <img src="https://img.shields.io/badge/C%2B%2B-23-blue">
  <img src="https://img.shields.io/badge/platform-Windows-blue">
  <a href="LICENSE">
    <img src="https://img.shields.io/github/license/0x4D696E61/Kiwi">
  </a>
</p>

<p align="center">
  <a href="https://github.com/0x4D696E61/Kiwi/releases/latest"><b>Download</b></a>
  ·
  <a href="#controls"><b>Controls</b></a>
  ·
  <a href="#commands"><b>Commands</b></a>
  ·
  <a href="#building-from-source"><b>Build</b></a>
</p>

## About

Kiwi is a terminal code editor built around keyboard navigation, modal editing, and a simple workspace.

It has its own terminal UI, file explorer, command system, recent files, configurable settings, and automatic updates.

Kiwi is currently in early development.

## Features

- NAV and EDIT editing modes
- Built-in file explorer
- Simple and Tree explorer modes
- File and directory workspaces
- Recent files
- Text and line selection
- Clipboard support
- Word navigation
- Undo and redo
- Command bar
- Configurable UI settings
- Automatic updates
- Direct file and workspace opening from the command line

## Installation

Download the Windows installer from the [**latest release**](https://github.com/0x4D696E61/Kiwi/releases/latest).

After installation, Kiwi can check for new versions automatically. Available updates can be installed directly from the Home screen.

## Controls

Kiwi uses two editor modes:

- **NAV** for navigation and commands
- **EDIT** for writing text

Press `Space` in NAV to enter EDIT.

Press and release `Left Ctrl` to return to NAV.

### NAV

| Key | Action |
| --- | --- |
| `W` | Move up |
| `A` | Move left |
| `S` | Move down |
| `D` | Move right |
| `Space` | Enter EDIT mode |
| `.` | Open command bar |
| `1` | Undo |
| `2` | Redo |
| `qq` | Previous word |
| `rr` | Next word |
| `xx` | Change word and enter EDIT |
| `v` | Toggle selection |
| `Shift + V` | Select current line |
| `cc` | Copy current line |
| `cv` | Copy selection |
| `ca` | Paste |

Moving while a selection is active extends the selection.

### EDIT

| Key | Action |
| --- | --- |
| Normal keys | Insert text |
| `Enter` | New line |
| `Backspace` | Delete character or merge lines |
| `Tab` | Insert 4 spaces |
| `Left Ctrl` | Return to NAV when released |

### Explorer

| Key | Action |
| --- | --- |
| `W` | Move up |
| `S` | Move down |
| `Enter` | Open file or directory |
| `Backspace` | Go to parent directory |
| `.` | Open command bar |
| `Left Ctrl + Space` | Switch between Explorer and Editor |

In Tree mode, `Enter` expands or collapses directories.

### Home

| Key | Action |
| --- | --- |
| `N` | Create a new file |
| `O` | Open a file |
| `S` | Open Settings |
| `1` - `9` | Open a recent file |
| `.` | Open command bar |

### Settings

| Key | Action |
| --- | --- |
| `A` / `W` | Previous setting |
| `D` / `S` | Next setting |
| `Enter` | Toggle setting |
| `Esc` | Close Settings |

Current settings include:

- Simple or Tree explorer
- Explorer/Editor separator
- NAV block cursor

## Commands

Press `.` to open the command bar.

### Files

| Command | Action |
| --- | --- |
| `.new <path>` | Create a new file |
| `.open <path>` | Open a file |
| `.save` / `.s` | Save |
| `.saveas <path>` | Save to another path |
| `.del <path>` / `.delete <path>` | Delete a file |

### Workspace

| Command | Action |
| --- | --- |
| `.home` | Return to Home |
| `.tree` | Toggle the file explorer |
| `.settings` | Open Settings |

### Exit

| Command | Action |
| --- | --- |
| `.q` / `.quit` | Quit |
| `.sq` / `.squit` / `.savequit` | Save and quit |
| `.fq` / `.fquit` / `.forcequit` | Force quit |

Normal quitting is blocked when the current file has unsaved changes.

## Opening files

Launch Kiwi normally:

```powershell
kiwi.exe
```

Open a file directly:

```powershell
kiwi.exe .\main.cpp
```

Or open a directory as a workspace:

```powershell
kiwi.exe .\MyProject
```

## Building from source

### Requirements

- Windows 11
- CMake 3.25+
- Ninja
- C++23-compatible compiler
- MSYS2 Clang64 recommended

### Build

```powershell
git clone https://github.com/0x4D696E61/Kiwi.git
cd Kiwi

cmake --preset debug
cmake --build --preset debug

.\build\debug\kiwi.exe
```

## Updates

Kiwi checks for updates when it starts.

When an update is available, the Home screen displays:

```text
Kiwi update available!  [I] Install  [L] Later
```

`I` downloads, verifies, and installs the update.

`L` dismisses the update for the current session.

## License

Kiwi is licensed under the [MIT License](LICENSE).

## Author

Created by **0x4D696E61**.