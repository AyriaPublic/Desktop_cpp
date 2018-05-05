/*
    Initial author: Convery (tcn@ayria.se)
    Started: 05-05-2018
    License: MIT
    Notes:
        Provides an interface for users to the system.
*/

#pragma once
#include "../Stdinclude.hpp"

namespace Frontend
{
    using Argument_t = std::pair<std::string, std::vector<std::string_view>>;
    using Commandcallback = std::function<void (std::vector<Argument_t>)>;

    struct Command_t
    {
        std::string Description;
        Commandcallback Callback;
    };
    void Registercommand(std::string Commandname, Command_t Command);
    void Executecommand(const std::string &Commandname, std::vector<Argument_t> Arguments);
}
