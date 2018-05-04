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

    // Initialize the storage.
    Backend::Initializemanifeststorage();

    // Parse the input into commands.
    using Command_t = std::pair<std::string, std::vector<std::string_view>>;
    std::list<Command_t> Commands{{"help", {} }};
    for (int i = 0; i < argc; ++i)
    {
        // If we got the path as argv[0], skip.
        if (std::strstr(argv[0], "Desktop")) continue;

        // Is a command name or argument.
        if (Frontend::isCommand(argv[i])) Commands.emplace_back().first = argv[i];
        else Commands.back().second.push_back(argv[i]);
    }

    // Remove the help command fallback.
    if (Commands.size() > 1) Commands.pop_front();

    // Execute the command synchronously.
    for (auto &Item : Commands)
    {
        // Transform the command to lowercase.
        std::transform(Item.first.begin(), Item.first.end(), Item.first.begin(), [](auto &Item){ return (char)::tolower(Item); });

        // Status information.
        Infoprint(va("Executing: %s", Item.first.c_str()));

        // Terminate the execution on error.
        if (!Frontend::Executecommand(Item.first, Item.second))
        {
            Infoprint(va("Command \"%s\" failed (see log)", Item.first.c_str()));
            break;
        }
    }

    return 0;
}
