
# KIWI 🥝

A lightweight, keyboard-driven terminal code editor written in C++23 for Windows.

KIWI is built around modal editing, simple commands, and a terminal-based workspace. It's an independent project, currently in early development.

## Features

- Keyboard-driven editing with Navigation (Move) and Edit (Type/Insert) modes
- Built-in file explorer
- Home screen with recent files
- Command bar for file operations and navigation
- Syntax-free, distraction-free terminal interface
- Custom terminal UI

## Controls

| Key | Action |
| --- | --- |
| Space | Switch between Move and Type modes |
| W / A / S / D | Move the cursor in Move mode |
| Esc | Leave Type mode |
| Enter | Insert a new line in Type mode |
| Backspace | Delete text in Type mode |

### Commands

| Command | Action |
| --- | --- |
| `.new` | Create a new file |
| `.open` | Open a file |
| `.save` | Save the current file |
| `.saveas` | Save to a new path |
| `.q` / `.quit` | Quit KIWI |
| `.sq` / `.squit` | Save and quit |
| `.tree` | Open the file explorer from Home |

## Building from source

### Requirements

- Windows 11
- CMake 3.25 or newer
- Ninja
- C++23-compatible compiler (MSYS2 Clang64 recommended)

### Build

```powershell
git clone https://github.com/0x4D696E61/Kiwi.git
cd Kiwi

cmake --preset debug
cmake --build --preset debug
.\build\debug\kiwi.exe
```

To open a file directly:

```powershell
.\build\debug\kiwi.exe .\main.cpp
```

## Releases

KIWI is currently in early development. A Windows installer and integrated update system are planned.

Until the first packaged release is available, build KIWI from source using the instructions above.

## Contributing

Issues, bug reports, and suggestions are welcome.

## Author

Created by [0x4D696E61](https://github.com/0x4D696E61).
