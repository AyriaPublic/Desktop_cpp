/*
    Initial author: Convery (tcn@ayria.se)
    Started: 02-05-2018
    License: MIT
    Notes:
        Provides command parsing and callbacks.
*/

#pragma once
#include "../Stdinclude.hpp"

namespace Frontend
{
    using Command_t = struct { std::string Usagestring; std::function<bool(size_t Argc, std::string_view *Argv)> Callback; };

    bool Executecommand(const std::string &Commandname, size_t Argc, std::string_view *Argv);
    void Registercommand(std::string_view Commandname, Command_t Command);
    bool isCommand(std::string_view Commandname);
}
