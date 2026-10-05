#include "CommandParser.hpp"

Command CmdParser::parse(const std::string& text) const {
    Command command;

    const std::size_t space = text.find(' ');
    const std::string name = text.substr(0, space);

    if (space != std::string::npos) {
        command.argument = text.substr(space + 1);
    }

    if (name == ".new") {
        command.type = CommandType::New;
    } else if (name == ".open") {
        command.type = CommandType::Open;
    } else if (name == ".save" || name == ".s") {
        command.type = CommandType::Save;
    } else if (name == ".sq" || name == ".squit" || name == ".savequit") {
        command.type = CommandType::SaveQuit;
    } else if (name == ".quit" || name == ".q") {
        command.type = CommandType::Quit;
    } else if (name == ".forcequit" || name == ".fq" || name == ".fquit") {
        command.type = CommandType::ForceQuit;
    } else if (name == ".saveas") {
        command.type = CommandType::SaveAs;
    } else if (name == ".goto") {
        command.type = CommandType::Goto;
    } else {
        command.type = CommandType::Unknown;
    }

    return command;
}