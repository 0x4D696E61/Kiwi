#pragma once

#include "../tui/Theme.hpp"

#include <algorithm>
#include <cctype>
#include <string>
#include <unordered_set>
#include <vector>

struct SyntaxState {
    bool blockComment = false;
};

inline bool syntaxWord(char ch) {
    return std::isalnum(static_cast<unsigned char>(ch)) || ch == '_';
}

inline std::vector<Rgb> highlightLine(const std::string& line, const std::string& ext, SyntaxState& state) {
    std::vector<Rgb> colors(line.size(), kiwiTheme.file);

    const bool cpp = ext == ".cpp" || ext == ".hpp" || ext == ".h" || ext == ".c" || ext == ".cc" || ext == ".cxx";
    const bool lua = ext == ".lua" || ext == ".luau";
    const bool python = ext == ".py";
    const bool js = ext == ".js" || ext == ".jsx" || ext == ".ts" || ext == ".tsx";
    const bool json = ext == ".json";
    const bool markdown = ext == ".md";

    if (markdown) {
        if (!line.empty() && line[0] == '#') std::fill(colors.begin(), colors.end(), kiwiTheme.keyword);
        else if (line.rfind("```", 0) == 0) std::fill(colors.begin(), colors.end(), kiwiTheme.function);
        else {
            for (std::size_t i = 0; i < line.size(); i++) {
                if (line[i] == '*' || line[i] == '`') colors[i] = kiwiTheme.keyword;
            }
        }
        return colors;
    }

    if (!cpp && !lua && !python && !js && !json) return colors;

    static const std::unordered_set<std::string> cppWords = {
        "alignas", "alignof", "auto", "bool", "break", "case", "catch", "char", "class",
        "const", "consteval", "constexpr", "continue", "decltype", "default", "delete",
        "do", "double", "else", "enum", "explicit", "extern", "false", "float", "for",
        "friend", "if", "inline", "int", "long", "namespace", "new", "nullptr", "private",
        "protected", "public", "return", "short", "signed", "sizeof", "static", "struct",
        "switch", "template", "this", "throw", "true", "try", "typedef", "typename",
        "union", "unsigned", "using", "virtual", "void", "volatile", "while"
    };

    static const std::unordered_set<std::string> luaWords = {
        "and", "break", "continue", "do", "else", "elseif", "end", "export", "false",
        "for", "function", "if", "in", "local", "nil", "not", "or", "repeat", "return",
        "then", "true", "type", "until", "while"
    };

    static const std::unordered_set<std::string> pythonWords = {
        "and", "as", "assert", "async", "await", "break", "class", "continue", "def",
        "del", "elif", "else", "except", "False", "finally", "for", "from", "global",
        "if", "import", "in", "is", "lambda", "None", "nonlocal", "not", "or", "pass",
        "raise", "return", "self", "True", "try", "while", "with", "yield"
    };

    static const std::unordered_set<std::string> jsWords = {
        "async", "await", "break", "case", "catch", "class", "const", "continue",
        "debugger", "default", "delete", "do", "else", "export", "extends", "false",
        "finally", "for", "from", "function", "if", "import", "in", "instanceof",
        "interface", "let", "new", "null", "of", "return", "static", "super", "switch",
        "this", "throw", "true", "try", "type", "typeof", "undefined", "var", "void",
        "while", "yield"
    };

    if (cpp) {
        const auto first = line.find_first_not_of(" \t");
        if (first != std::string::npos && line[first] == '#') {
            std::fill(colors.begin(), colors.end(), kiwiTheme.keyword);
            return colors;
        }
    }

    const std::string blockStart = lua ? "--[[" : "/*";
    const std::string blockEnd = lua ? "]]" : "*/";

    for (std::size_t i = 0; i < line.size();) {
        if (state.blockComment) {
            const auto end = line.find(blockEnd, i);
            const auto stop = end == std::string::npos ? line.size() : end + blockEnd.size();
            std::fill(colors.begin() + i, colors.begin() + stop, kiwiTheme.comment);
            i = stop;
            if (end == std::string::npos) break;
            state.blockComment = false;
            continue;
        }

        if ((cpp || js || lua) && line.compare(i, blockStart.size(), blockStart) == 0) {
            state.blockComment = true;
            continue;
        }

        const bool comment = ((cpp || js) && line.compare(i, 2, "//") == 0) ||
                             (lua && line.compare(i, 2, "--") == 0) ||
                             (python && line[i] == '#');

        if (comment) {
            std::fill(colors.begin() + i, colors.end(), kiwiTheme.comment);
            break;
        }

        if (line[i] == '"' || line[i] == '\'' || (js && line[i] == '`')) {
            const char quote = line[i];
            const std::size_t start = i++;

            while (i < line.size()) {
                if (line[i] == '\\' && i + 1 < line.size()) {
                    i += 2;
                    continue;
                }
                if (line[i++] == quote) break;
            }

            Rgb color = kiwiTheme.string;

            if (json) {
                std::size_t next = i;
                while (next < line.size() && std::isspace(static_cast<unsigned char>(line[next]))) next++;
                if (next < line.size() && line[next] == ':') color = kiwiTheme.function;
            }

            std::fill(colors.begin() + start, colors.begin() + i, color);
            continue;
        }

        if (std::isdigit(static_cast<unsigned char>(line[i]))) {
            const std::size_t start = i++;
            while (i < line.size() && (syntaxWord(line[i]) || line[i] == '.')) i++;
            std::fill(colors.begin() + start, colors.begin() + i, kiwiTheme.number);
            continue;
        }

        if (std::isalpha(static_cast<unsigned char>(line[i])) || line[i] == '_') {
            const std::size_t start = i++;
            while (i < line.size() && syntaxWord(line[i])) i++;

            const std::string word = line.substr(start, i - start);
            const bool keyword = (cpp && cppWords.find(word) != cppWords.end()) || (lua && luaWords.find(word) != luaWords.end()) || (python && pythonWords.find(word) != pythonWords.end()) || (js && jsWords.find(word) != jsWords.end()) || (json && (word == "true" || word == "false" || word == "null"));
            
            std::size_t next = i;
            while (next < line.size() && std::isspace(static_cast<unsigned char>(line[next]))) next++;

            if (keyword) std::fill(colors.begin() + start, colors.begin() + i, kiwiTheme.keyword);
            else if (!json && next < line.size() && line[next] == '(') {
                std::fill(colors.begin() + start, colors.begin() + i, kiwiTheme.function);
            }
            continue;
        }

        i++;
    }

    return colors;
}