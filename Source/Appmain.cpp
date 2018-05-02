/*
    Initial author: Convery (tcn@ayria.se)
    Started: 02-05-2018
    License: MIT
    Notes:
        Provides the entrypoint for Windows and Nix.
*/

#include "Stdinclude.hpp"

// Universal entrypoint.
int main(int argc, char **argv)
{
    // Clear the previous sessions logfile.
    Clearlog();

    // Parse the input into commands.
    using Command_t = std::pair<std::string, std::vector<std::string_view>>;
    std::list<Command_t> Commands{{"help", {} }};
    for (int i = 0; i < argc; ++i)
    {
        // Is a command name or argument.
        if (Frontend::isCommand(argv[i])) Commands.emplace_back().first = argv[i];
        else Commands.back().second.push_back(argv[i]);
    }

    // Remove the help command fallback.
    if (Commands.size() > 1) Commands.pop_front();

    // Process the commands and return.
    for (const auto &Item : Commands)
    {
        // Status information.
        Infoprint(va("Executing: %s", Item.first.c_str()));

        // Result information.
        thread_local auto Commandargv = std::make_unique<std::string_view[]>(Item.second.size());
        if (!Frontend::Executecommand(Item.first, Item.second.size(), Commandargv.get()))
            Infoprint(va("Command \"%s\" failed (see log)", Item.first.c_str()));
    }

    return 0;
}
