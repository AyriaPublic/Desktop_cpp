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
    int Index = 0;
    std::string Command;

    // Clear the previous sessions logfile.
    Clearlog();

    // Fetch the command, maybe skip the first.
    if (std::strstr(argv[Index], "Desktop")) Index++;
    if(Index < argc) Command = argv[Index++];
    else Command = "help";

    // Parse the input into a arguments.
    std::vector<Frontend::Argument_t> Arguments{ {} };
    for (; Index < argc; ++Index)
    {
        // Create a new argument per switch.
        if (argv[Index][0] == '-') Arguments.emplace_back().first = argv[Index];
        else Arguments.back().second.push_back(argv[Index]);
    }

    // Transform the command to lowercase.
    std::transform(Command.begin(), Command.end(), Command.begin(), [](const auto &Item){ return (char)::tolower(Item); });

    // Process the command.
    {
        // Status information.
        Infoprint(va("Executing: %s", Command.c_str()));

        Frontend::Executecommand(Command, Arguments);
    }

    // Wait for any torrents in the backend to finish.
    if (Backend::Activetorrents())
    {
        std::this_thread::sleep_for(std::chrono::seconds(2));

        while (Backend::Activetorrents())
        {
            Infoprint(va("Waiting for %u torrents..", Backend::Activetorrents()));
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    }

    return 0;
}
