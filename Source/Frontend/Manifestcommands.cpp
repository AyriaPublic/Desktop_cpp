/*
    Initial author: Convery (tcn@ayria.se)
    Started: 02-05-2018
    License: MIT
    Notes:
        Provides a command to iterate through the manifests.
*/

#include "../Stdinclude.hpp"

// Download the latest manifest configuration.
bool Updatecommand(std::vector<std::string_view> Arguments)
{
    Infoprint("Checking for updates..");

    if (auto Error = Backend::Updatemanifeststorage())
    {
        switch (Error)
        {
            case Backend::Updateresult_t::Nointernet:
                Infoprint("Could not connect to the server, try again later.");
                break;
        }

        return false;
    }

    Infoprint("Storage is up to date.");
    return true;
}

// Search through the manifests.
namespace Search
{
    // Weighted relevancy.
    std::vector<size_t> Internal(std::vector<std::string_view> &Criteria, float Gameweight, float Authorweight, float Nameweight, float Descriptionweight)
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

    // Print plugin information.
    void Print(std::vector<size_t> &PluginIDs)
    {
        // Print out the description of each manifest.
        Infoprint(va("%-15s  %-63s", "Pluginname", "Description"));
        for (const auto &Item : PluginIDs)
        {
            if (auto Manifest = Backend::Fetchmanifest(Item))
            {
                Infoprint(va("%-15s  %-63s", Manifest->Friendlyname.c_str(), Manifest->Description.c_str()));
            }
        }
    }

    // Ordered:
    bool byName(std::vector<std::string_view> Arguments)
    {
        auto Localarray = Internal(Arguments, 1.2f, 0.8f, 5.0f, 0.3f);

        // Limit the output to 20 results.
        Infoprint(va("Found %u results%s:", Localarray.size(), Localarray.size() <= 20 ? "" : " (only showing top 20)"));
        if (Localarray.size() > 20) Localarray.resize(20);

        Print(Localarray);
        return true;
    }
    bool byGame(std::vector<std::string_view> Arguments)
    {
        auto Localarray = Internal(Arguments, 5.0f, 0.3f, 1.2f, 0.8f);

        // Limit the output to 20 results.
        Infoprint(va("Found %u results%s:", Localarray.size(), Localarray.size() <= 20 ? "" : " (only showing top 20)"));
        if (Localarray.size() > 20) Localarray.resize(20);

        Print(Localarray);
        return true;
    }
    bool byAuthor(std::vector<std::string_view> Arguments)
    {
        auto Localarray = Internal(Arguments, 0.4f, 5.0f, 1.2f, 0.8f);

        // Limit the output to 20 results.
        Infoprint(va("Found %u results%s:", Localarray.size(), Localarray.size() <= 20 ? "" : " (only showing top 20)"));
        if (Localarray.size() > 20) Localarray.resize(20);

        Print(Localarray);
        return true;
    }
    bool byDescription(std::vector<std::string_view> Arguments)
    {
        auto Localarray = Internal(Arguments, 0.4f, 0.4f, 0.4f, 1.2f);

        // Limit the output to 20 results.
        Infoprint(va("Found %u results%s:", Localarray.size(), Localarray.size() <= 20 ? "" : " (only showing top 20)"));
        if (Localarray.size() > 20) Localarray.resize(20);

        Print(Localarray);
        return true;
    }

    // Limited:
    bool All(std::vector<std::string_view> Arguments)
    {
        auto Localarray = Internal(Arguments, 1.0f, 1.0f, 1.0f, 0.8f);
        Infoprint(va("Found %u results:", Localarray.size()));

        Print(Localarray);
        return true;
    }
    bool top20(std::vector<std::string_view> Arguments)
    {
        auto Localarray = Internal(Arguments, 1.0f, 1.0f, 1.0f, 0.8f);

        // Limit the output to 20 results.
        Infoprint(va("Found %u results%s:", Localarray.size(), Localarray.size() <= 20 ? "" : " (only showing top 20)"));
        if (Localarray.size() > 20) Localarray.resize(20);

        Print(Localarray);
        return true;
    }
}

static struct Startup { Startup()
{
    // Update the on-disk storage.
    Frontend::Registercommand("update", { "download the latest plugin-info", Updatecommand });

    // Limited results.
    Frontend::Registercommand("search", { "find top 20 plugins by any criteria", Search::top20 });
    Frontend::Registercommand("search-full", { "find all plugins by any criteria", Search::All });

    // Ordered results.
    Frontend::Registercommand("search-name", { "find top 20 plugins ordered by name", Search::byName });
    Frontend::Registercommand("search-game", { "find top 20 plugins ordered by game", Search::byGame });
    Frontend::Registercommand("search-author", { "find top 20 plugins ordered by author", Search::byAuthor });
    Frontend::Registercommand("search-desc", { "find top 20 plugins ordered by description", Search::byDescription });
}} Loader{};
