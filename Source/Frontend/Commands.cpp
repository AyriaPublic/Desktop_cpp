/*
    Initial author: Convery (tcn@ayria.se)
    Started: 02-05-2018
    License: MIT
    Notes:
        Provides command parsing and callbacks.
*/

#include "../Stdinclude.hpp"

namespace Frontend
{
    //using Command_t = struct { std::string Usagestring; std::function<bool(size_t Argc, std::string_view *Argv)> Callback; };
    std::map<std::string /* Commandname */, Command_t> *Commands{ nullptr };

    bool Executecommand(const std::string &Commandname, std::vector<std::string_view> Arguments)
    {
        assert(Commands);
        auto Callback = Commands->find(Commandname);
        if (Callback == Commands->end()) return false;

        return Callback->second.Callback(Arguments);
    }
    void Registercommand(std::string_view Commandname, Command_t Command)
    {
        if (!Commands) Commands = new std::map<std::string, Command_t>();
        Commands->emplace(Commandname, Command);
    }
    bool isCommand(std::string_view Commandname)
    {
        assert(Commands);
        return Commands->end() != std::find_if(Commands->cbegin(), Commands->cend(), [&](const auto &Item) -> bool
        {
            return 0 == std::strcmp(Item.first.c_str(), Commandname.data());
        });
    }

    bool Printusage(std::vector<std::string_view> Arguments)
    {
        assert(Commands);
        Infoprint("Usage:");

        for (const auto &Item : *Commands)
        {
            Infoprint(va("\t%-20s%s", Item.first.c_str(), Item.second.Usagestring.c_str()));
        }

        return true;
    }
    static struct Startup { Startup()
    {
        Registercommand("?", { "Displays usageinformation", Printusage });
        Registercommand("help", { "Displays usageinformation", Printusage });
    }} Loader{};
}
