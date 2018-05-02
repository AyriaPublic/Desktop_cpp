/*
    Initial author: Convery (tcn@ayria.se)
    Started: 02-05-2018
    License: MIT
    Notes:
        Provides parsing of the manifests directory.
*/

#include "../Stdinclude.hpp"
#include <nlohmann/json.hpp>

namespace Backend
{
    std::vector<Manifest_t> Manifeststorage;

    void Initializemanifeststorage()
    {
        for (const auto &Item : Listfiles("./Manifests", ".json"))
        {
            if (const auto Filebuffer = Readfile(va("./Manifests/%s", Item.c_str())); Filebuffer.size())
            {
                try
                {
                    Manifest_t Localmanifest;
                    auto Object = nlohmann::json::parse(Filebuffer.c_str());

                    // Parse the JSON manifest into the C++ representation.
                    for (const auto &Entry : Object["Games"]) Localmanifest.Games.push_back(Entry);
                    for (const auto &Entry : Object["Dependencies"]) Localmanifest.Dependencies.push_back(Entry);
                    Localmanifest.Author = Object["Author"].is_null() ? "Unknown" : Object["Author"].get<std::string>();
                    Localmanifest.Downloadlink = Object["Downloadlink"].is_null() ? "" : Object["Downloadlink"].get<std::string>();
                    Localmanifest.Description  = Object["Description"].is_null() ? "Unknown" : Object["Description"].get<std::string>();
                    Localmanifest.Sourcecodelink  = Object["Sourcecodelink"].is_null() ? "Closedsource" : Object["Sourcecodelink"].get<std::string>();
                    Localmanifest.Friendlyname = Object["Friendlyname"].is_null() ? Item.substr(0, Item.find_last_of('.')) : Object["Friendlyname"].get<std::string>();

                    // Transform the game-names and dependencies to make searching easier.
                    for (auto &Entry : Localmanifest.Games) std::transform(Entry.begin(), Entry.end(), Entry.begin(), [](auto Item) { return (char)::tolower(Item); });
                    for (auto &Entry : Localmanifest.Dependencies) std::transform(Entry.begin(), Entry.end(), Entry.begin(), [](auto Item) { return (char)::tolower(Item); });

                    // Make the manifest available in the storage.
                    Localmanifest.Index = Manifeststorage.size();
                    Manifeststorage.push_back(Localmanifest);
                }
                catch (std::exception &e)
                {
                    Debugprint(va("Failed to parse manifest \"%s\": %s", Item.c_str(), e.what()));
                }
            }
        }
    }
    Manifest_t Fetchmanifest(size_t Index)
    {
        if (Index >= Manifeststorage.size()) return {};
        return Manifeststorage[Index];
    }

    namespace Findmanifests
    {
        std::vector<size_t> byGame(std::string_view Criteria)
        {
            std::vector<size_t> Results{};

            std::for_each(Manifeststorage.cbegin(), Manifeststorage.cend(), [&](const auto &Item) -> void
            {
                for (const auto &Entry : Item.Games)
                {
                    if (std::strstr(Entry.c_str(), Criteria.data()))
                    {
                        Results.push_back(Item.Index);
                        break;
                    }
                }
            });

            return Results;
        }
        std::vector<size_t> byName(std::string_view Criteria)
        {
            std::vector<size_t> Results{};

            std::for_each(Manifeststorage.cbegin(), Manifeststorage.cend(), [&](const auto &Item) -> void
            {
                if (std::strstr(Item.Friendlyname.c_str(), Criteria.data()))
                {
                    Results.push_back(Item.Index);
                }
            });

            return Results;
        }
        std::vector<size_t> byAuthor(std::string_view Criteria)
        {
            std::vector<size_t> Results{};

            std::for_each(Manifeststorage.cbegin(), Manifeststorage.cend(), [&](const auto &Item) -> void
            {
                if (std::strstr(Item.Author.c_str(), Criteria.data()))
                {
                    Results.push_back(Item.Index);
                }
            });

            return Results;
        }
        std::vector<size_t> byDescription(std::string_view Criteria)
        {
            std::unordered_map<size_t, size_t> Positives;
            std::vector<std::string> Tokens{};
            std::vector<size_t> Results{};

            // Tokenize the input.
            for (const auto &Item : Criteria)
            {
                if (Item == ' ' || Item == '.' || Item == ',')
                {
                    Tokens.emplace_back();
                    continue;
                }

                Tokens.back().push_back(Item);
            }

            // Find as many tokens as possible.
            std::for_each(Manifeststorage.cbegin(), Manifeststorage.cend(), [&](const auto &Item) -> void
            {
                for (const auto &Entry : Tokens)
                {
                    if (std::strstr(Item.Description.c_str(), Entry.c_str()))
                    {
                        Positives[Item.Index]++;
                    }
                }
            });

            // Sort the map by number of tokens.
            std::vector<std::pair<size_t, size_t>> Sortedvector;
            for (const auto &Item : Positives) Sortedvector.push_back({ Item.second, Item.first });
            std::sort(Sortedvector.begin(), Sortedvector.end());

            // Return the results in order.
            for (const auto &Item : Sortedvector) Results.push_back(Item.second);
            return Results;
        }
    }
}
