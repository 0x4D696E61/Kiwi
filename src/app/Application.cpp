#include "Application.hpp"
#include "AppState.hpp"
#include "Focus.hpp"
#include "Settings.hpp"

#include "../editor/Editor.hpp"

#include "../cmds/CommandBar.hpp"
#include "../cmds/CommandParser.hpp"

#include "../platform/win32/Console.hpp"
#include "../platform/win32/Clipboard.hpp"

#include "../terminal/Terminal.hpp"

#include "../buffer/Buffer.hpp"

#include "../renderer/EditorRenderer.hpp"
#include "../renderer/ExplorerRenderer.hpp"
#include "../renderer/HomeRenderer.hpp"

#include "../explorer/Explorer.hpp"

#include "../tui/Screen.hpp"
#include "../tui/TuiRenderer.hpp"

#include "../tui/Layout.hpp"
#include "../tui/Painter.hpp"
#include "../tui/Theme.hpp"

#include "../updater/Updater.hpp"

#include <algorithm>
#include <iostream>
#include <string>

#include <chrono>
#include <cmath>
#include <thread>

#include <cctype>
#include <cstdlib>
#include <filesystem>

#define NOMINMAX // fuck u windows <3
#include <windows.h>

static std::string fileLanguage(const std::filesystem::path& path) {
    const std::string ext = path.extension().string();
    if (ext == ".cpp" || ext == ".hpp" || ext == ".h" || ext == ".cc" || ext == ".cxx") return "C++";
    if (ext == ".c") return "C";
    if (ext == ".lua" || ext == ".luau") return "Luau";
    if (ext == ".py") return "Python";
    if (ext == ".js" || ext == ".jsx") return "JavaScript";
    if (ext == ".ts" || ext == ".tsx") return "TypeScript";
    if (ext == ".json") return "JSON";
    if (ext == ".md") return "Markdown";
    return "Text";
}

static void drawSettings(Screen& screen, const Settings& settings, int selected, float uTab) {
    const int w = std::min(78, screen.w() - 4);
    const int h = 16;
    if (w < 48 || screen.h() < h + 2) return;

    const int x = (screen.w() - w) / 2;
    const int y = (screen.h() - h) / 2;

    const Rgb background{36, 36, 36};
    const Rgb accent = kiwiTheme.matBracket;
    const Rgb white{225, 225, 225};
    const Rgb muted{130, 130, 130};

    Style bg;
    bg.bgRgb = background;
    bg.fgRgb = white;

    Style title = bg;
    title.bold = true;

    Style inactive = bg;
    inactive.fgRgb = muted;

    Style active = bg;
    active.bold = true;

    Style accentStyle = bg;
    accentStyle.fgRgb = accent;
    accentStyle.bold = true;

    auto roundedBox = [&](int left, int top, int width, int height, const Style& border) {
        for (int row = 0; row < height; row++) screen.text(left, top + row, std::string(width, ' '), bg);

        screen.glyph(left, top, "\xE2\x95\xAD", border);
        screen.glyph(left + width - 1, top, "\xE2\x95\xAE", border);
        screen.glyph(left, top + height - 1, "\xE2\x95\xB0", border);
        screen.glyph(left + width - 1, top + height - 1, "\xE2\x95\xAF", border);

        for (int col = 1; col < width - 1; col++) {
            screen.glyph(left + col, top, "\xE2\x94\x80", border);
            screen.glyph(left + col, top + height - 1, "\xE2\x94\x80", border);
        }

        for (int row = 1; row < height - 1; row++) {
            screen.glyph(left, top + row, "\xE2\x94\x82", border);
            screen.glyph(left + width - 1, top + row, "\xE2\x94\x82", border);
        }
    };

    roundedBox(x, y, w, h, inactive);

    screen.text(x + 2, y + 1, "SETTINGS", title);
    screen.text(x + w - 13, y + 1, "ESC", inactive);

    const std::string tabs[3] = {"Explorer", "Appearance", "Cursor"};
    const int gap = 2;
    const int tabW = (w - 4 - gap * 2) / 3;

    for (int i = 0; i < 3; i++) {
        const int tabX = x + 2 + i * (tabW + gap);
        const bool focused = selected == i;
        const Style& labelStyle = focused ? active : inactive;

        roundedBox(tabX, y + 3, tabW, 3, inactive);

        const int labelX = tabX + (tabW - static_cast<int>(tabs[i].size())) / 2;
        screen.text(labelX, y + 4, tabs[i], labelStyle);
    }

    const int underlineX = x + 2 + static_cast<int>(std::lround(uTab * (tabW + gap)));

    for (int col = 1; col < tabW - 1; col++) {
        screen.glyph(underlineX + col, y + 6, "\xE2\x94\x81", accentStyle);
    }

    roundedBox(x + 1, y + 8, w - 2, 5, inactive);

    const std::string headings[3] = {
        "Explorer mode",
        "Vertical separator",
        "NAV block cursor"
    };

    const std::string values[3] = {
        settings.tree ? "TREE" : "SIMPLE",
        settings.separator ? "ON" : "OFF",
        settings.blockCursor ? "ON" : "OFF"
    };

    const std::string descriptions[3] = {
        "Choose between normal navigation and expandable folders.",
        "Display a vertical line between Explorer and Editor.",
        "Use a block cursor in NAV mode instead of a thin bar."
    };

    screen.text(x + 3, y + 9, headings[selected], title);
    screen.text(x + 3, y + 10, "[ " + values[selected] + " ]", accentStyle);
    screen.text(x + 3, y + 11, descriptions[selected].substr(0, w - 6), inactive);

    screen.text(x + 2, y + 14, "A/D  Switch tabs     Enter  Toggle     Esc  Close", inactive);
}

static std::filesystem::path resPath(const std::string& val) {
    std::filesystem::path path = val;

    if (!val.empty() && val[0] == '~' && (val.size() == 1 || val[1] == '\\' || val[1] == '/')) {
        path = std::filesystem::current_path() / (val.size() > 1 ? val.substr(2) : "");
    }

    if (path.is_relative()) path = std::filesystem::current_path() / path;
    return path.lexically_normal();
}

Application::Application(int argc, char* argv[], Updater& updater) : argc_(argc), argv_(argv), updater_(updater) {}

int Application::run() {
    Terminal terminal;
    Console console;
    Clipboard clipboard;
    CmdBar cmdBar;
    CmdParser cmdParser;
    Buffer buffer;
    Editor editor;
    Explorer explorer;
    Settings settings;
    settings.load();
    explorer.setTree(settings.tree);

    bool updateLatuh = false;
    bool updateShown = false;

    const std::string updateMsg = "Kiwi update available!  [I] Install  [L] Later";
    bool settingsOpen = false;
    int settingIndex = 0;
    float uTab = 0.0f;
    int cursorShape = -1;

    AppState state = AppState::Home;
    Focus focus = Focus::Editor;
    Focus lcf = Focus::Editor; // the thing we last focused on

    std::string message;

    int scrollX = 0;
    int scrollY = 0;
    int explorerScroll = 0;

    bool hTreeOpen = false;
    bool explVis = true;
    bool wksr = false; //yeah.. //a day later, i forgot what this even means :)

    std::filesystem::path wksroot = std::filesystem::current_path();
    std::filesystem::path currDir = wksroot;
    std::filesystem::path startupf;

    bool hTreeNC = true;

    if (argc_ > 1) {
        std::error_code ec;
        const auto path = std::filesystem::absolute(std::filesystem::path(argv_[1]), ec).lexically_normal();

        if (ec || !std::filesystem::exists(path)) {
            std::cerr << "Kiwi: path does not exist: " << argv_[1] << '\n';
            return 1;
        }

        if (std::filesystem::is_directory(path)) {
            wksroot = path;
            currDir = path;
            state = AppState::Editor;
            focus = Focus::Explorer;
        } else if (std::filesystem::is_regular_file(path)) {
            wksroot = path.parent_path();
            currDir = wksroot;
            startupf = path;

            if (!buffer.open(path)) {
                std::cerr << "Kiwi: could not open: " << path.string() << '\n';
                return 1;
            }

            state = AppState::Editor;
            focus = Focus::Editor;
        } else {
            std::cerr << "Kiwi: unsupported path\n";
            return 1;
        }

        explorer.open(wksroot);
        wksr = true;
    }

    HomeRenderer home(terminal);
    EditorRenderer editorView(terminal);
    ExplorerRenderer explorerView;

    Screen screen(terminal.w(), terminal.h());
    TuiRenderer tui(terminal);

    auto resPath = [&](const std::string& val) -> std::filesystem::path {
        if (val.empty()) return {};

        std::filesystem::path path;

        if (val[0] == '~' && (val.size() == 1 || val[1] == '\\' || val[1] == '/')) {
            path = currDir / (val.size() == 1 ? "" : val.substr(2));
        } else {
            path = val;
        }

        if (path.is_relative()) path = currDir / path;
        return path.lexically_normal();
    };

    auto selVis = [&](const std::filesystem::path& target) -> bool {
    for (int i = 0; i < 100000; i++) {
        const int previous = explorer.selected();
        explorer.move(-1);
        if (explorer.selected() == previous) break;
    }

    for (int i = 0; i < 100000; i++) {
        const ExplorerEntry* entry = explorer.curr();
        if (entry && entry->path.lexically_normal() == target.lexically_normal()) return true;

        const int previous = explorer.selected();
        explorer.move(1);
        if (explorer.selected() == previous) break;
    }

    return false;
};

    auto revealFile = [&](const std::filesystem::path& file) {
        if (!wksr) {
            explorer.open(wksroot);
            wksr = true;
        }

        if (!explorer.tree()) return;

        const auto rel = file.lexically_relative(wksroot);
        if (rel.empty() || rel.is_absolute() || *rel.begin() == "..") return;

        std::filesystem::path folder = wksroot;

        for (const auto& part : rel.parent_path()) {
            if (part == "." || part.empty()) continue;
            folder /= part;

            if (!selVis(folder)) return;

            const int previous = explorer.selected();
            explorer.move(1);

            bool expanded = false;

            if (explorer.selected() != previous) {
                const ExplorerEntry* next = explorer.curr();

                if (next) {
                    const auto child = next->path.lexically_relative(folder);
                    expanded = !child.empty() && !child.is_absolute() && *child.begin() != ".." && next->path != folder;
                }

                explorer.move(-1);
            }

            if (!expanded) explorer.toggle();
        }

        selVis(file);
    };

    if (!startupf.empty()) {
        home.addRecent(startupf);
        revealFile(startupf);
    }

    auto drawFrame = [&]() {
        screen.resize(terminal.w(), terminal.h());
        screen.clear();
        screen.clearGlyphs();

        if (state == AppState::Home && settingsOpen) {
            drawSettings(screen, settings, settingIndex, uTab);
            tui.render(screen, 0, 0, false);
            return;
        }

        if (state == AppState::Home && hTreeOpen) {
            const int panelWidth = std::min(32, std::max(0, screen.w() - 40));
            const Rect panel{0, 0, panelWidth, std::max(0, screen.h() - 1)};
            Painter explorerPainter(screen, panel);

            const int explorerRows = std::max(1, explorerPainter.h() - 1);
            if (explorer.selected() < explorerScroll) explorerScroll = explorer.selected();
            if (explorer.selected() >= explorerScroll + explorerRows) explorerScroll = explorer.selected() - explorerRows + 1;
            explorerScroll = std::max(0, explorerScroll);

            if (panelWidth > 0) explorerView.render(explorer, explorerPainter, explorerScroll, focus == Focus::Explorer);

            if (panelWidth > 0) {
                Style separator;
                separator.fgRgb = kiwiTheme.separator;
                for (int y = 0; y < panel.h; y++) screen.set(panelWidth - 1, y, '|', separator);
            }

            if (hTreeNC) {
                terminal.clear();
                tui.invalidate();
                hTreeNC = false;
            }
            tui.render(screen, 0, 0, false);
            home.render(panelWidth);
            if (cmdBar.active()) home.renderCmdBar(cmdBar.text());
            else if (!message.empty()) home.renderMsg(message);
            else if (updater_.available() && !updateLatuh) home.renderMsg(updateMsg);
            return;
        }

        const Rect area{0, 0, screen.w(), std::max(0, screen.h() - 1)};

        LayoutItem explorerItem;
        explorerItem.size = explVis ? (buffer.path().empty() ? area.w : std::min(32, area.w)) : 0;

        LayoutItem editorItem;
        editorItem.flex = true;

        const auto panes = Layout::split(area, LayoutDirection::Horizontal, {explorerItem, editorItem});

        Painter explorerPainter(screen, panes[0].area);
        Painter editorPainter(screen, panes[1].area);

        const int gutter = editorView.gutterWidth(buffer);
        const int textWidth = std::max(1, panes[1].area.w - gutter);
        const int textHeight = std::max(1, panes[1].area.h);

        if (editor.cY() < scrollY) scrollY = editor.cY();
        if (editor.cY() >= scrollY + textHeight) scrollY = editor.cY() - textHeight + 1;

        if (editor.cX() < scrollX) scrollX = editor.cX();
        if (editor.cX() >= scrollX + textWidth) scrollX = editor.cX() - textWidth + 1;

        scrollX = std::max(0, scrollX);
        scrollY = std::max(0, scrollY);

        const int explorerRows = std::max(1, explorerPainter.h() - 1);

        if (explorer.selected() < explorerScroll) explorerScroll = explorer.selected();
        if (explorer.selected() >= explorerScroll + explorerRows) {
            explorerScroll = explorer.selected() - explorerRows + 1;
        }

        explorerScroll = std::max(0, explorerScroll);

        if (explVis) explorerView.render(explorer, explorerPainter, explorerScroll, focus == Focus::Explorer);
        if (!buffer.path().empty()) editorView.renderTui(buffer, editor, editorPainter, scrollX, scrollY);

        if (explVis && !buffer.path().empty() && settings.separator && panes[0].area.w > 0) {
            Style sep;
            sep.fgRgb = kiwiTheme.separator;
            const int x = panes[0].area.x + panes[0].area.w - 1;
            for (int y = 0; y < area.h; y++) screen.set(x, y, '|', sep);
        }

        int cursorX = 0;
        int cursorY = 0;
        bool showCursor = false;

        const int statusY = screen.h() - 1;
        const int commandY = statusY;
        const int barWidth = std::max(0, screen.w() - 1);

        if (statusY >= 0 && !cmdBar.active() && message.empty()) {
            const Rgb baseBg{40, 46, 59};
            const Rgb modeBg = focus == Focus::Explorer ? kiwiTheme.matBracket : (editor.isTypeMode() ? kiwiTheme.folder : kiwiTheme.matBracket);
            const Rgb infoBg{65, 73, 94};
            const Rgb posBg{130, 177, 163};

            Style bar;
            bar.bgRgb = baseBg;
            bar.fgRgb = kiwiTheme.file;

            for (int x = 0; x < screen.w(); x++) screen.set(x, statusY, ' ', bar);

            const std::string mode = focus == Focus::Explorer ? "EXPLORER" : (editor.isTypeMode() ? "EDIT" : "NAV"); // TYPE and MOVE are Internal in Kiwi, user exposed variables are EDIT, NAV
            const std::string modeText = " " + mode + " ";
            const std::string name = buffer.path().empty() ? currDir.filename().string() : buffer.path().filename().string() + (buffer.modified() ? " [+]" : "");

            const int total = std::max(1, static_cast<int>(buffer.lines().size()));
            const int percent = std::clamp((editor.cY() + 1) * 100 / total, 0, 100);

            const std::string infoText = " " + fileLanguage(buffer.path()) + "  UTF-8 ";
            const std::string posText = " " + std::to_string(percent) + "%  " + std::to_string(editor.cY() + 1) + ":" + std::to_string(editor.cX() + 1) + " ";

            Style modeStyle;
            modeStyle.bgRgb = modeBg;
            modeStyle.fgRgb = Rgb{20, 25, 33};
            modeStyle.bold = true;

            Style leftArrow;
            leftArrow.fgRgb = modeBg;
            leftArrow.bgRgb = baseBg;

            Style infoStyle;
            infoStyle.bgRgb = infoBg;
            infoStyle.fgRgb = kiwiTheme.file;

            Style infoArrow;
            infoArrow.fgRgb = infoBg;
            infoArrow.bgRgb = baseBg;

            Style posStyle;
            posStyle.bgRgb = posBg;
            posStyle.fgRgb = Rgb{20, 25, 33};
            posStyle.bold = true;

            Style posArrow;
            posArrow.fgRgb = posBg;
            posArrow.bgRgb = infoBg;

            const int fileX = static_cast<int>(modeText.size()) + 1;
            const int rightWidth = static_cast<int>(infoText.size() + posText.size()) + 2;
            const int rightX = screen.w() - rightWidth;
            const bool showRight = !buffer.path().empty() && rightX > fileX + 2;

            if (screen.w() > static_cast<int>(modeText.size())) {
                screen.text(0, statusY, modeText, modeStyle);
                screen.glyph(static_cast<int>(modeText.size()), statusY, "\xEE\x82\xB0", leftArrow);
            }

            const int nameWidth = std::max(0, (showRight ? rightX : screen.w()) - fileX - 1);
            if (nameWidth > 0) screen.text(fileX, statusY, name.substr(0, nameWidth), bar);

            if (showRight) {
                int x = rightX;

                screen.glyph(x++, statusY, "\xEE\x82\xB2", infoArrow);
                screen.text(x, statusY, infoText, infoStyle);
                x += static_cast<int>(infoText.size());

                screen.glyph(x++, statusY, "\xEE\x82\xB2", posArrow);
                screen.text(x, statusY, posText, posStyle);
            }
        }

        if (commandY >= 0 && (cmdBar.active() || !message.empty())) {
            if (cmdBar.active()) {
                Style background;
                background.bgRgb = Rgb{36, 36, 36};
                background.fgRgb = kiwiTheme.file;

                Style label = background;
                label.bgRgb = kiwiTheme.matBracket;
                label.fgRgb = Rgb{255, 255, 255};
                label.bold = true;

                Style arrow = background;
                arrow.fgRgb = kiwiTheme.matBracket;

                for (int x = 0; x < screen.w(); x++) screen.set(x, commandY, ' ', background);

                if (screen.w() >= 5) screen.text(0, commandY, " CMD ", label);
                if (screen.w() > 5) screen.glyph(5, commandY, "\xEE\x82\xB0", arrow);
                if (screen.w() > 7) screen.text(7, commandY, cmdBar.text().substr(0, screen.w() - 7), background);

                cursorX = std::min(7 + static_cast<int>(cmdBar.text().length()), std::max(0, screen.w() - 1));
                cursorY = commandY;
                showCursor = true;
            } else if (!message.empty()) {
                screen.text(0, commandY, message.substr(0, barWidth));
            }
        }

        if (!cmdBar.active() && focus == Focus::Editor && !buffer.path().empty()) {
            const int localX = gutter + editor.cX() - scrollX;
            const int localY = editor.cY() - scrollY;

            cursorX = panes[1].area.x + localX;
            cursorY = panes[1].area.y + localY;

            showCursor = localX >= gutter && localX < panes[1].area.w && localY >= 0 && localY < panes[1].area.h;
        }

        if (settingsOpen) {
            drawSettings(screen, settings, settingIndex, uTab);
            showCursor = false;
        }

        tui.render(screen, cursorX, cursorY, showCursor);
        const bool block = settings.blockCursor && !settingsOpen && !cmdBar.active() && focus == Focus::Editor && !editor.isTypeMode();
        if (cursorShape != static_cast<int>(block)) {
            std::cout << (block ? "\x1b[2 q" : "\x1b[6 q") << std::flush;
            cursorShape = static_cast<int>(block);
        }
    };

    auto renderHome = [&]() {
        home.render();
        updateShown = false;
    };

    terminal.clear();
    tui.invalidate();

    if (state == AppState::Editor) drawFrame();
    else renderHome();

    while (true) {
        const KeyEvent event = console.readKey();
        const char key = event.character;

        if (state == AppState::Home && !settingsOpen && !cmdBar.active() && updater_.available() && !updateLatuh && message.empty()) {
            if (!updateShown) {
                if (hTreeOpen) drawFrame();
                else home.renderMsg(updateMsg);
                updateShown = true;
            }

            if (event.keyDown && (key == 'i' || key == 'I')) {
                updater_.install();
                continue;
            }

            if (event.keyDown && (key == 'l' || key == 'L')) {
                updateLatuh = true;
                updateShown = false;

                if (hTreeOpen) drawFrame();
                else renderHome();
                continue;
            }
        } else {
            updateShown = false;
        }

        if (settingsOpen) {
            if (!event.keyDown) continue;

            if (key == 27) {
                settingsOpen = false;

                if (state == AppState::Home) {
                    terminal.clear();
                    tui.invalidate();
                    if (hTreeOpen) drawFrame();
                    else renderHome();
                } else {
                    drawFrame();
                }

                continue;
            }

            int nextIndex = settingIndex;

            if (key == 'a' || key == 'w') nextIndex = (settingIndex + 2) % 3;
            if (key == 'd' || key == 's') nextIndex = (settingIndex + 1) % 3;

            if (nextIndex != settingIndex) {
                const float start = uTab;
                settingIndex = nextIndex;

                for (int frame = 1; frame <= 12; frame++) {
                    const float t = static_cast<float>(frame) / 12.0f;
                    const float eased = t * t * (3.0f - 2.0f * t);

                    uTab = start + (static_cast<float>(settingIndex) - start) * eased;

                    drawFrame();
                    std::this_thread::sleep_for(std::chrono::milliseconds(12));
                }

                uTab = static_cast<float>(settingIndex);
                continue;
            }

            if (key == '\r' || event.keyCode == VK_RETURN) {
                if (settingIndex == 0) {
                    settings.tree = !settings.tree;
                    explorer.setTree(settings.tree);
                    explorerScroll = 0;
                } else if (settingIndex == 1) {
                    settings.separator = !settings.separator;
                } else {
                    settings.blockCursor = !settings.blockCursor;
                }

                if (!settings.save()) message = "Could not save settings";
            }

            drawFrame();
            continue;
        }

        if (event.focSwitch) {
            if (state == AppState::Editor && explVis && !cmdBar.active()) {
                if (focus == Focus::Explorer) {
                    focus = lcf;
                } else {
                    lcf = focus;
                    focus = Focus::Explorer;
                }

                drawFrame();
            }

            continue;
        }

        if (event.leftCtrlDown && !event.leftCtrl) continue;

        // lCtrl release switches back to NAV
        if (state == AppState::Editor && event.leftCtrl && !event.keyDown) {
            if (!event.ctrlUsed && focus == Focus::Editor) {
                editor.enterMoveMode();
                message.clear();
                drawFrame();
            }

            continue;
        }

        if (event.leftCtrl && event.keyDown) continue;
        if (!event.keyDown) continue;

        if (((state == AppState::Editor && explVis) || (state == AppState::Home && hTreeOpen)) && focus == Focus::Explorer && !cmdBar.active()) {
            if (key == '.') {
                cmdBar.start();
                message.clear();
            } else if (key == 'w') {
                explorer.move(-1);
                message.clear();
            } else if (key == 's') {
                explorer.move(1);
                message.clear();
            } else if (key == '\b' || event.keyCode == VK_BACK) {
                const auto parent = currDir.parent_path();

                if (!parent.empty() && parent != currDir) {
                    if (currDir == wksroot) {
                        wksroot = parent;
                        currDir = parent;
                        explorer.open(parent);
                        explorerScroll = 0;
                    } else {
                        currDir = parent;

                        if (explorer.tree()) {
                            selVis(currDir);
                        } else {
                            explorer.up();
                            explorerScroll = 0;
                        }
                    }
                }

                message.clear();
            } else if (key == '\r' || event.keyCode == VK_RETURN) {
                const ExplorerEntry* entry = explorer.curr();

                if (entry) {
                    const auto path = entry->path;
                    const bool dir = entry->dir;

                    if (dir) {
                        if (explorer.tree()) {
                            explorer.toggle();
                        } else {
                            explorer.open(path);
                            explorerScroll = 0;
                        }

                        currDir = path;

                        message.clear();
                    } else if (buffer.modified()) {
                        message = "Unsaved changes! Use .save first.";
                    } else if (buffer.open(path)) {
                        state = AppState::Editor;
                        home.addRecent(buffer.path());
                        editor.reset();
                        scrollX = 0;
                        scrollY = 0;
                        focus = Focus::Editor;
                        lcf = Focus::Editor;
                        currDir = path.parent_path();
                        revealFile(path);
                        message.clear();
                    } else {
                        message = "Could not open file";
                    }
                }
            }

            drawFrame();
            continue;
        }

        // Handle editor input unless the cmdbar is active
        if (state == AppState::Editor && !cmdBar.active()) {
            if (key == '.' && !editor.isTypeMode()) {
                cmdBar.start();
                message.clear();
                drawFrame();
                continue;
            }

            if (!editor.isTypeMode() && key == '1') {
                editor.undo(buffer);
                message.clear();
                drawFrame();
                continue;
            }

            if (!editor.isTypeMode() && key == '2') {
                editor.redo(buffer);
                message.clear();
                drawFrame();
                continue;
            }

            editor.handleKey(key, buffer, clipboard);
            message.clear();
            drawFrame();
            continue;
        }

        // Handles Prefix
        if (!cmdBar.active()) {

                if (state == AppState::Home) {
                    if (key == 'n' || key == 'o') {
                        cmdBar.start();

                        const std::string prefix = key == 'n' ? "new " : "open ";
                        for (const char character : prefix) cmdBar.handleKey(character);

                        home.renderCmdBar(cmdBar.text());
                        continue;
                    }

                    if (key == 's') {
                        settingsOpen = true;
                        settingIndex = 0;
                        uTab = 0.0f;
                        message.clear();

                        terminal.clear();
                        tui.invalidate();
                        drawFrame();
                        continue;
                    }

                    if (key == 'h') {
                        home.renderMsg("The .help tutorial is coming later.");
                        continue;
                    }

                    if (key >= '1' && key <= '9') {
                        const auto path = home.recent(key - '1');
                        if (path.empty()) continue;

                        if (!buffer.open(path)) {
                            home.renderMsg("Could not open recent file");
                            continue;
                        }

                        cmdBar.cancel();
                        state = AppState::Editor;
                        editor.reset();
                        scrollX = 0;
                        scrollY = 0;
                        focus = Focus::Editor;
                        lcf = Focus::Editor;
                        message.clear();

                        currDir = buffer.path().parent_path();
                        revealFile(buffer.path());
                        explorerScroll = 0;
                        home.addRecent(buffer.path());

                        terminal.clear();
                        tui.invalidate();
                        drawFrame();
                        continue;
                    }
                }

            if (key == '.') {
                cmdBar.start();
                message.clear();

                if (state == AppState::Editor) {
                    drawFrame();
                } else {
                    home.renderCmdBar(cmdBar.text());
                }
            }

            // Allow ESC to also clear the bottom msg
            if (key == 27) {
                message.clear();

                if (state == AppState::Editor) {
                    drawFrame();
                } else {
                    home.renderCmdBar("");
                }
            }

            continue;
        }

        // Handles Escape
        if (key == 27) {
            cmdBar.cancel();
            message.clear();

            if (state == AppState::Editor) {
                drawFrame();
            } else {
                home.renderCmdBar("");
            }

            continue;
        }

            if (key == '\r') {

                if (cmdBar.text() == "home" || cmdBar.text() == ".home") {
                    cmdBar.cancel();

                    if (state == AppState::Editor && buffer.modified()) {
                        message = "Unsaved Changes! Use .save first.";
                        drawFrame();
                        continue;
                    }

                    state = AppState::Home;
                    settingsOpen = false;
                    focus = hTreeOpen ? Focus::Explorer : Focus::Editor;
                    message.clear();

                    terminal.clear();
                    tui.invalidate();
                    if (hTreeOpen) drawFrame();
                    else renderHome();
                    continue;
                }

                std::string typed = cmdBar.text();
                std::transform(typed.begin(), typed.end(), typed.begin(), [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });

                if (typed == "tree" || typed == ".tree") {
                    cmdBar.cancel();
                    message.clear();

                    if (state == AppState::Home) {
                        hTreeOpen = !hTreeOpen;
                        if (hTreeOpen) hTreeNC = true;

                        if (hTreeOpen && !wksr) {
                            explorer.open(wksroot);
                            wksr = true;
                        }

                        focus = hTreeOpen ? Focus::Explorer : Focus::Editor;
                    } else {
                        explVis = !explVis;
                        if (!explVis && focus == Focus::Explorer) focus = Focus::Editor;
                    }

                    if (state == AppState::Home && !hTreeOpen) {
                        terminal.clear();
                        tui.invalidate();
                        renderHome();
                    } else {
                        drawFrame();
                    }

                    continue;
                }

                if (cmdBar.text() == "settings" || cmdBar.text() == ".settings") {
                    cmdBar.cancel();
                    settingsOpen = true;
                    settingIndex = 0;
                    uTab = 0.0f;
                    message.clear();

                    if (state == AppState::Home) {
                        terminal.clear();
                        tui.invalidate();
                    }

                    drawFrame();
                    continue;
                }

                const bool sDel = typed == ".del" || typed.rfind(".del ", 0) == 0;
                const bool lDel = typed == ".delete" || typed.rfind(".delete ", 0) == 0;

                if (sDel || lDel) {
                    const std::string raw = cmdBar.text();
                    const std::size_t prefLen = lDel ? 7 : 4;
                    std::string argument = raw.size() > prefLen ? raw.substr(prefLen + 1) : "";

                    cmdBar.cancel();

                    auto showResult = [&]() {
                        if (state == AppState::Home && !hTreeOpen) home.renderMsg(message);
                        else drawFrame();
                    };

                    const std::size_t first = argument.find_first_not_of(" \t");

                    if (first == std::string::npos) {
                        message = "Error: .del requires a file path";
                        showResult();
                        continue;
                    }

                    argument.erase(0, first);

                    const std::filesystem::path target = resPath(argument);
                    std::error_code ec;

                    const bool delOpenf = !buffer.path().empty() && std::filesystem::absolute(buffer.path(), ec).lexically_normal() == target;

                    if (delOpenf && buffer.modified()) {
                        message = "Unsaved changes! Save the file first.";
                        showResult();
                        continue;
                    }

                    if (!std::filesystem::is_regular_file(target, ec) || ec) {
                        message = "File does not exist or is not a regular file";
                        showResult();
                        continue;
                    }

                    if (!std::filesystem::remove(target, ec) || ec) {
                        message = "Could not delete file";
                        showResult();
                        continue;
                    }

                    if (delOpenf) {
                        buffer = Buffer{};
                        editor.reset();
                        state = AppState::Editor;
                        focus = Focus::Explorer;
                        lcf = Focus::Explorer;
                        explVis = true;
                        currDir = target.parent_path();
                        scrollX = 0;
                        scrollY = 0;
                    }

                    explorer.refresh();
                    message = "Deleted " + target.filename().string();

                    if (state == AppState::Editor) {
                        terminal.clear();
                        tui.invalidate();
                    }

                    showResult();
                    continue;
                }

            const Command command = cmdParser.parse(cmdBar.text());

            // SaveQuit
            if (command.type == CommandType::SaveQuit) {
                if (state != AppState::Editor) {
                    cmdBar.cancel();
                    home.renderMsg("No file is open");
                    continue;
                }

                if (!buffer.save()) {
                    cmdBar.cancel();
                    message = "Could not save file";
                    drawFrame();
                    continue;
                }

                break;
            }

            if (command.type == CommandType::ForceQuit) break;

            if (command.type == CommandType::Quit) {
                if (state == AppState::Editor && buffer.modified()) {
                    cmdBar.cancel();
                    message = "Unsaved changes! use .save first.";
                    drawFrame();
                    continue;
                }

                break;
            }

            if (command.type == CommandType::New) {
                if (command.argument.empty()) {
                    cmdBar.cancel();

                    if (state == AppState::Editor) {
                        message = "Error: .new requires a path";
                        drawFrame();
                    } else {
                        home.renderMsg("Error: .new requires a path");
                    }

                    continue;
                }

                if (!buffer.create(resPath(command.argument))) {
                    cmdBar.cancel();

                    if (state == AppState::Editor) {
                        message = "Error: could not create file";
                        drawFrame();
                    } else {
                        home.renderMsg("Error: could not create file");
                    }

                    continue;
                }

                cmdBar.cancel();
                state = AppState::Editor;
                home.addRecent(buffer.path());
                editor.reset();
                editor.handleKey(' ', buffer, clipboard);
                scrollX = 0;
                scrollY = 0;

                focus = Focus::Editor;
                lcf = Focus::Editor;

                message.clear();
                currDir = buffer.path().parent_path();
                explorer.refresh();
                revealFile(buffer.path());
                explorerScroll = 0;

                terminal.clear();
                tui.invalidate();
                drawFrame();
                continue;
            }

            // Open
            if (command.type == CommandType::Open) {
                if (command.argument.empty()) {
                    cmdBar.cancel();

                    if (state == AppState::Editor) {
                        message = "Error: .open requires a path";
                        drawFrame();
                    } else {
                        home.renderMsg("Error: .open requires a path");
                    }

                    continue;
                }

                if (!buffer.open(resPath(command.argument))) {
                    cmdBar.cancel();

                    if (state == AppState::Editor) {
                        message = "Error: could not open file";
                        drawFrame();
                    } else {
                        home.renderMsg("Error: could not open file");
                    }

                    continue;
                }

                cmdBar.cancel();
                state = AppState::Editor;
                home.addRecent(buffer.path());
                editor.reset();
                scrollX = 0;
                scrollY = 0;

                focus = Focus::Editor;
                lcf = Focus::Editor;

                message.clear();
                currDir = buffer.path().parent_path();
                revealFile(buffer.path());
                explorerScroll = 0;

                terminal.clear();
                tui.invalidate();
                drawFrame();
                continue;
            }

            // Save
            if (command.type == CommandType::Save) {
                cmdBar.cancel();

                if (state != AppState::Editor) {
                    home.renderMsg("No file is open");
                } else {
                    message = buffer.save() ? "Saved" : "Could not save file";
                    drawFrame();
                }

                continue;
            }

            // SaveAs
            if (command.type == CommandType::SaveAs) {
                cmdBar.cancel();

                if (state != AppState::Editor) {
                    home.renderMsg("No file is open");
                } else if (command.argument.empty()) {
                    message = "Error: .saveas requires a path";
                    drawFrame();
                } else if (!buffer.saveAs(resPath(command.argument))) {
                    message = "Could not save file";
                    drawFrame();
                } else {
                    home.addRecent(buffer.path());

                    currDir = buffer.path().parent_path();
                    explorer.refresh();
                    revealFile(buffer.path());
                    explorerScroll = 0;

                    message = "Saved as " + buffer.path().string();
                    drawFrame();
                }

                continue;
            }

            cmdBar.cancel();
            message.clear();

            if (state == AppState::Editor) {
                drawFrame();
            } else {
                home.renderCmdBar("");
            }

            continue;
        }

        cmdBar.handleKey(key);

        if (state == AppState::Editor) {
            drawFrame();
        } else {
            home.renderCmdBar(cmdBar.text());
        }
    }

    std::cout << "\x1b[0 q" << std::flush;
    terminal.showCursor();
    return 0;
}