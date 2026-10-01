<p align="center">
  <img src="assets/kiwi.png" alt="Kiwi Logo" width="160">
</p>

# Kiwi 🥝

A lightweight, keyboard-driven terminal code editor written in C++23 for Windows.

Kiwi is built around modal editing, fast keyboard navigation, simple commands, and a terminal-based workspace.

> Current version: **v0.1.02**

## Features

- Keyboard-driven modal editing with **NAV** and **EDIT** modes
- Built-in file explorer with Simple and Tree modes
- Editor and explorer focus switching
- File and directory workspace support
- Home screen with recent files
- Selection, line selection, copy, and paste
- Word-based navigation and editing
- Undo and redo
- Command bar for file operations and navigation
- Save As and file deletion
- Configurable explorer, separator, and NAV cursor
- Automatic update checking and installation
- Custom terminal UI and status bar
- Direct file or directory opening from the command line

## Installation

Download the latest Windows installer from the **Releases** section of this repository.

Run:

```text
Kiwi-Setup-0.1.02.exe
```

Kiwi includes an automatic updater. When a newer version is available, the Home screen will display:

```text
Kiwi update available!  [I] Install  [L] Later
```

`I` installs the update and `L` dismisses it for the current session.

## Modes

Kiwi uses two main editor modes.

### NAV

NAV is the default navigation mode.

Use it to move around the file, select text, copy and paste, navigate by words, undo or redo, and open the command bar.

### EDIT

EDIT mode is used for writing and modifying text.

Press `Space` while in NAV to enter EDIT mode.

Press and release `Left Ctrl` to return to NAV mode.

## Editor controls

### NAV mode

| Key | Action |
| --- | --- |
| `W` | Move cursor up |
| `A` | Move cursor left |
| `S` | Move cursor down |
| `D` | Move cursor right |
| `Space` | Enter EDIT mode |
| `Left Ctrl` | Return to NAV mode when released |
| `.` | Open command bar |
| `1` | Undo |
| `2` | Redo |
| `qq` | Move to previous word |
| `rr` | Move to next word |
| `xx` | Change current word and enter EDIT mode |
| `v` | Toggle text selection |
| `Shift + V` | Select current line |
| `cc` | Copy current line |
| `cv` | Copy selected text |
| `ca` | Paste clipboard contents |

Movement while selection is active expands or shrinks the selection.

### EDIT mode

| Key | Action |
| --- | --- |
| Normal keys | Insert text |
| `Enter` | Insert a new line |
| `Backspace` | Delete the previous character or merge lines |
| `Tab` | Insert 4 spaces |
| `Left Ctrl` | Return to NAV mode when released |

## Explorer controls

| Key | Action |
| --- | --- |
| `W` | Move selection up |
| `S` | Move selection down |
| `Enter` | Open a file or directory / toggle a folder in Tree mode |
| `Backspace` | Move to the parent directory |
| `.` | Open command bar |
| `Left Ctrl + Space` | Switch focus between Explorer and Editor |

The explorer can operate in either **Simple** or **Tree** mode.

Tree mode allows directories to be expanded directly inside the explorer.

## Home controls

| Key | Action |
| --- | --- |
| `N` | Start creating a new file |
| `O` | Start opening a file |
| `S` | Open Settings |
| `H` | Open Help |
| `1` - `9` | Open a recent file |
| `.` | Open command bar |

## Settings controls

| Key | Action |
| --- | --- |
| `A` / `W` | Previous setting |
| `D` / `S` | Next setting |
| `Enter` | Toggle selected setting |
| `Esc` | Close Settings |

Current settings include:

- Explorer mode: `TREE` / `SIMPLE`
- Vertical Explorer/Editor separator
- NAV block cursor

## Commands

Press `.` to open the command bar.

### Files

| Command | Action |
| --- | --- |
| `.new <path>` | Create a new file |
| `.open <path>` | Open a file |
| `.save` / `.s` | Save the current file |
| `.saveas <path>` | Save the current file to another path |
| `.del <path>` | Delete a file |
| `.delete <path>` | Delete a file |

### Navigation and interface

| Command | Action |
| --- | --- |
| `.home` | Return to the Home screen |
| `.tree` | Toggle the file explorer |
| `.settings` | Open Settings |

### Quitting

| Command | Action |
| --- | --- |
| `.q` / `.quit` | Quit Kiwi |
| `.sq` / `.squit` / `.savequit` | Save and quit |
| `.fq` / `.fquit` / `.forcequit` | Force quit without the normal unsaved-change protection |

Kiwi prevents normal quitting or switching files when unsaved changes would be lost.

## Opening files and workspaces

Kiwi can be launched normally:

```powershell
.\kiwi.exe
```

A file can be opened directly:

```powershell
.\kiwi.exe .\main.cpp
```

A directory can also be opened as a workspace:

```powershell
.\kiwi.exe .\MyProject
```

When a directory is supplied, Kiwi opens the workspace with focus on the file explorer.

## Building from source

### Requirements

- Windows 11
- CMake 3.25 or newer
- Ninja
- C++23-compatible compiler
- MSYS2 Clang64 recommended

### Build

Clone the Kiwi repository, enter its directory, then run:

```powershell
cmake --preset debug
cmake --build --preset debug
.\build\debug\kiwi.exe
```

## Updates

Kiwi checks for updates automatically when it starts.

When an update is available, it can be installed directly from Kiwi. Update installers are cryptographically verified before installation.

The updater is designed to perform the installation silently without requiring a separate update wizard.

## Status

Kiwi is currently in early development.

The project is usable, but controls, commands, UI behavior, and internal systems may continue to change between releases.

Current release:

```text
v0.1.02
```

## Contributing

Issues, bug reports, and suggestions are welcome.

## Author

Created by **0x4D696E61**.