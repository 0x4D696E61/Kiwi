cmake_minimum_required(VERSION 3.25)

project(Kiwi VERSION 0.2.04 LANGUAGES CXX RC)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
configure_file(
    src/updater/Version.hpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/generated/Version.hpp
    @ONLY
)

add_executable(kiwi
    src/app/main.cpp
    src/app/Application.cpp
    src/terminal/Terminal.cpp
    src/platform/win32/Console.cpp
    src/editor/Editor.cpp
    src/renderer/HomeRenderer.cpp
    src/cmds/CommandBar.cpp
    src/cmds/CommandParser.cpp
    src/buffer/Buffer.cpp
    src/renderer/EditorRenderer.cpp
    src/editor/EditHistory.cpp
    src/platform/win32/Clipboard.cpp
    src/explorer/Explorer.cpp
    src/renderer/ExplorerRenderer.cpp
    src/tui/Screen.cpp
    src/tui/TuiRenderer.cpp
    src/tui/Layout.cpp
    src/tui/Painter.cpp
    src/updater/Updater.cpp
    src/tutorial/Tutorial.cpp
    src/renderer/TutorialRenderer.cpp
    src/search/Search.cpp
)

target_sources(kiwi PRIVATE assets/kiwi.rc)
target_link_libraries(kiwi PRIVATE wininet)

target_include_directories(kiwi PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/generated
)

add_custom_command(TARGET kiwi POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/external/winsparkle/WinSparkle-0.9.4/x64/Release/WinSparkle.dll"
        "$<TARGET_FILE_DIR:kiwi>/WinSparkle.dll"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/external/winsparkle/WinSparkle-0.9.4/bin/winsparkle-tool.exe"
        "$<TARGET_FILE_DIR:kiwi>/winsparkle-tool.exe"
)