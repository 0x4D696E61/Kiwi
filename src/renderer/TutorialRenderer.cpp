#include "TutorialRenderer.hpp"

#include "../tutorial/Tutorial.hpp"

#include "../tui/Screen.hpp"
#include "../tui/Theme.hpp"

#include <algorithm>
#include <string>
#include <vector>

void TutorialRenderer::render(Screen& screen, const Tutorial& tutorial) {
    const int w = std::min(76, screen.w() - 4);
    const int h = std::min(22, screen.h() - 2);

    if (w < 48 || h < 16) return;

    const int x = (screen.w() - w) / 2;
    const int y = (screen.h() - h) / 2;

    const Rgb background{36, 36, 36};
    const Rgb white{225, 225, 225};
    const Rgb muted{130, 130, 130};

    Style bg;
    bg.bgRgb = background;
    bg.fgRgb = white;

    Style title = bg;
    title.bold = true;

    Style inactive = bg;
    inactive.fgRgb = muted;

    Style accent = bg;
    accent.fgRgb = kiwiTheme.matBracket;
    accent.bold = true;

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

    struct Page {
        std::string title;
        std::vector<std::string> lines;
    };

    const std::vector<Page> pages = {
        {
            "Welcome to Kiwi",
            {
                "Kiwi uses two main editing modes:",
                "",
                "NAV    Move around and perform actions",
                "EDIT   Write and modify text",
                "",
                "Press Space in NAV to enter EDIT.",
                "Press and release Left Ctrl to return to NAV.",
                "",
                "Your current mode is shown in the status bar."
            }
        },
        {
            "Navigation",
            {
                "NAV mode is where you move around your file.",
                "",
                "W      Move up",
                "A      Move left",
                "S      Move down",
                "D      Move right",
                "",
                "qq     Move to the previous word",
                "rr     Move to the next word"
            }
        },
        {
            "Editing",
            {
                "Press Space in NAV to enter EDIT.",
                "",
                "Type normally to insert text.",
                "",
                "Enter       Create a new line",
                "Backspace   Delete text or merge lines",
                "Tab         Insert 4 spaces",
                "",
                "xx          Change a word and enter EDIT [Works only in NAV Mode. (For vim users: equivalent to cw)]"
            }
        },
        {
            "Selection",
            {
                "Selections are controlled from NAV mode.",
                "",
                "v           Start or stop a selection",
                "Shift + V   Select the current line",
                "",
                "Use W, A, S and D while selecting to",
                "expand or shrink the selection."
            }
        },
        {
            "Clipboard",
            {
                "Kiwi uses the Windows clipboard.",
                "",
                "cc     Copy the current line",
                "cv     Copy the current selection",
                "ca     Paste",
                "",
                "Copied text can also be used in other",
                "Windows applications."
            }
        },
        {
            "Explorer",
            {
                "The Explorer lets you move through your workspace.",
                "",
                "W                    Move up",
                "S                    Move down",
                "Enter                Open or expand",
                "Backspace            Parent directory",
                "Left Ctrl + Space    Switch focus",
                "",
                "Explorer mode can be SIMPLE or TREE."
            }
        },
        {
            "Commands",
            {
                "Press . in NAV to open the command bar.",
                "",
                ".new <path>       Create a file",
                ".open <path>      Open a file",
                ".save             Save",
                ".saveas <path>    Save as",
                ".tree             Toggle Explorer",
                ".settings         Open Settings",
                ".home             Return Home",
                ".q                Quit"
            }
        },
        {
            "You're ready",
            {
                "That's the basics of Kiwi.",
                "",
                "NAV is where you move and perform actions.",
                "EDIT is where you write.",
                "",
                "Space enters EDIT.",
                "Left Ctrl returns to NAV.",
                ". opens the command bar.",
                "",
                "Now go edit something :)"
            }
        }
    };

    const int page = std::clamp(tutorial.page(), 0, static_cast<int>(pages.size()) - 1);
    const Page& current = pages[page];

    roundedBox(x, y, w, h, inactive);

    screen.text(x + 2, y + 1, "KIWI TUTORIAL", title);

    const std::string ctr = std::to_string(page + 1) + "/" + std::to_string(tutorial.pages());
    screen.text(x + w - static_cast<int>(ctr.size()) - 2, y + 1, ctr, inactive);

    screen.text(x + 2, y + 3, current.title, accent);

    int row = y + 5;

    for (const std::string& line : current.lines) {
        if (row >= y + h - 3) break;

        screen.text(x + 3, row, line.substr(0, w - 6), bg);
        row++;
    }

    screen.text(x + 2, y + h - 2, "A  Previous     D  Next     Esc  Close", inactive);
}