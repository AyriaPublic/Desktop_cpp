/*
    Initial author: Convery (tcn@ayria.se)
    Started: 05-05-2018
    License: MIT
    Notes:
        Provides a command to iterate through the manifests.
*/

#include "../Stdinclude.hpp"

// Print plugin-information in a unified way.
void Pluginprint(std::vector<size_t> &PluginIDs)
{
    // Print a nice header.
    Infoprint(va("%-15s  %-63s", "Pluginname", "Description"));
    Infoprint("--------------------------------------------------------------------------------");

    // Print out the description of each manifest.
    for (const auto &Item : PluginIDs)
    {
        if (auto Manifest = Backend::Fetchmanifest(Item))
        {
            Infoprint(va("%-.15s  %-.63s", Manifest->Friendlyname.c_str(), Manifest->Description.c_str()));
        }
    }
}

// Perform a weighted search.
std::vector<size_t> Manifestsearch(std::vector<std::string_view> &Criteria, float Gameweight, float Authorweight, float Nameweight, float Descriptionweight)
{
    std::unordered_map<size_t /* ID */, float /* Relevancy */> Manifests{};

    // Ensure that we have a criteria.
    if (Criteria.size() == 0) Criteria.push_back("");

    // Iterate over the given criteria.
    for (const auto &Item : Criteria)
    {
        // Find all games matching a criteria.
        for (const auto &Entry : Backend::Findmanifests::byGame(Item))
        {
            Manifests[Entry.first] += Entry.second * Gameweight;
        }

        // Find all names matching a criteria.
        for (const auto &Entry : Backend::Findmanifests::byName(Item))
        {
            Manifests[Entry.first] += Entry.second * Nameweight;
        }

        // Find all authors matching a criteria.
        for (const auto &Entry : Backend::Findmanifests::byAuthor(Item))
        {
            Manifests[Entry.first] += Entry.second * Authorweight;
        }

        // Find all descriptions matching a criteria.
        for (const auto &Entry : Backend::Findmanifests::byDescription(Item))
        {
            Manifests[Entry.first] += Entry.second * Descriptionweight;
        }
    }

    // Sort the manifests by relevancy.
    std::vector<std::pair<float, size_t>> Sortedvector; Sortedvector.reserve(Manifests.size());
    for (const auto &Item : Manifests) Sortedvector.push_back({ Item.second, Item.first });
    std::sort(Sortedvector.begin(), Sortedvector.end(), [](const auto &a, const auto &b) { return a.second > b.second; });

    // Return the results in order of relevancy.
    std::vector<size_t> Results; Results.reserve(Sortedvector.size());
    for (const auto &Item : Sortedvector) Results.push_back(Item.second);

    return Results;
}

// Perform a weighted search of all categories.
void Fullsearch(std::vector<std::string_view> Arguments)
{
    auto Localarray = Manifestsearch(Arguments, 1.0f, 1.0f, 1.0f, 0.8f);
    Infoprint(va("Found %u results:", Localarray.size()));
    Pluginprint(Localarray);
}

// Search by tags.
void Search(std::vector<Frontend::Argument_t> Arguments)
{
    std::unordered_map<size_t /* ID */, size_t /* Relevancy */> Manifests{};

    // Specialized search if we have no tags.
    if (Arguments.size() == 0) return Fullsearch({ "" });
    if (Arguments.size() == 1) return Fullsearch(Arguments[0].second);

    // Iterate over the tags.
    for (const auto &Item : Arguments)
    {
        // Search by name.
        if (std::strstr(Item.first.c_str(), "-n"))
        {
            for (const auto &Entry : Item.second)
            {
                for (const auto &ID : Backend::Findmanifests::byName(Entry))
                {
                    Manifests[ID.first]++;
                }
            }
        }

        // Search by game.
        if (std::strstr(Item.first.c_str(), "-g"))
        {
            for (const auto &Entry : Item.second)
            {
                for (const auto &ID : Backend::Findmanifests::byGame(Entry))
                {
                    Manifests[ID.first]++;
                }
            }
        }

        // Search by author.
        if (std::strstr(Item.first.c_str(), "-a"))
        {
            for (const auto &Entry : Item.second)
            {
                for (const auto &ID : Backend::Findmanifests::byAuthor(Entry))
                {
                    Manifests[ID.first]++;
                }
            }
        }

        // Search by description.
        if (std::strstr(Item.first.c_str(), "-d"))
        {
            std::string Mergedstring;

            for (const auto &Entry : Item.second)
            {
                Mergedstring += Entry;
            }

            for (const auto &ID : Backend::Findmanifests::byDescription(Mergedstring))
            {
                Manifests[ID.first]++;
            }
        }
    }

    // If we don't find any results, return an error.
    if(Manifests.size() == 0) return Infoprint("No plugins matches the specified criteria.");

    // Sort the manifests by relevancy.
    std::vector<std::pair<size_t, size_t>> Sortedvector; Sortedvector.reserve(Manifests.size());
    for (const auto &Item : Manifests) Sortedvector.push_back({ Item.second, Item.first });
    std::sort(Sortedvector.begin(), Sortedvector.end(), [](const auto &a, const auto &b) { return a.second > b.second; });

    // Only get the top 2 relevancy scores.
    auto Topscore = Sortedvector[0].second;
    auto Point = std::partition(Sortedvector.begin(), Sortedvector.end(), [&](const auto &Item) { return Item.second >= Topscore - 1; });
    Sortedvector.erase(Point, Sortedvector.end());

    // Limit the output to 20 results.
    if(Sortedvector.size() > 20) Sortedvector.resize(20);

    // Convert to pluginprint format.
    std::vector<size_t> Localarray; Localarray.reserve(Sortedvector.size());
    for (const auto &Item : Sortedvector) Localarray.push_back(Item.first);

    // Show the info to the user.
    Pluginprint(Localarray);
}

// Add the commands on startup.
namespace
{
    static struct Startup
    {
        Startup()
        {
            Frontend::Registercommand("search", { va("%-80s%-80s%-80s", "search for plugins, switches:",
                                                     "-n [name1 name2 ...] -g [game1 game2 ...]",
                                                     "-a [author 1 author2 ...] -d [description tokens ...]"),
                                      Search });
        }
    } Loader{};
}
