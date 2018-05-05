/*
    Initial author: Convery (tcn@ayria.se)
    Started: 05-05-2018
    License: MIT
    Notes:
        Provides an interface for users to the system.
*/

#include "../Stdinclude.hpp"

// Internal storage for our commands.
std::map<std::string, Frontend::Command_t> *Commands{ nullptr };

// List the commands available.
void Printusage(std::vector<Frontend::Argument_t> Arguments)
{
    assert(Commands);
    Infoprint(va("%-15s  %-63s", "Command", "Description"));
    Infoprint("--------------------------------------------------------------------------------");

    for (const auto &Item : *Commands)
    {
        Infoprint(va("%-15s  %-.63s", Item.first.c_str(), Item.second.Description.c_str()));
        for (size_t i = 1; i < 4; ++i)
        {
            if (Item.second.Description.size() > 80 * i)
            {
                Infoprint(va("%-15s  %-.63s", "", Item.second.Description.c_str() + 80 * i));
            }
        }

    }
}

// Add the commands on startup.
namespace
{
    static struct Startup
    {
        Startup()
        {
            Frontend::Registercommand("?", { "Displays usageinformation", Printusage });
            Frontend::Registercommand("help", { "Displays usageinformation", Printusage });
        }
    } Loader{};
}

// Exported methods.
namespace Frontend
{
    void Registercommand(std::string Commandname, Command_t Command)
    {
        if (!Commands) Commands = new std::map<std::string, Command_t>();
        Commands->emplace(Commandname, Command);
    }
    void Executecommand(const std::string &Commandname, std::vector<Argument_t> Arguments)
    {
        assert(Commands);
        auto Callback = Commands->find(Commandname);
        if (Callback == Commands->end()) return;

        return Callback->second.Callback(Arguments);
    }
}
