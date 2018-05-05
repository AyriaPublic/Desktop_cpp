/*
    Initial author: Convery (tcn@ayria.se)
    Started: 05-05-2018
    License: MIT
    Notes:
        Provides parsing of the manifests directory.
*/

#include "../Stdinclude.hpp"
#include <nlohmann/json.hpp>
#include <curl/curl.h>

// All the manifests currently searchable.
std::vector<Backend::Manifest_t> Manifeststorage;

// Perform a HTTP request.
size_t CURLWrite(void *ptr, size_t Size, size_t nmemb, std::string *Data)
{
    Data->append((char*)ptr, Size * nmemb);
    return Size * nmemb;
};
std::string CURLFetch(std::string_view URL)
{
    std::string Response{};

    // Initialize libCURL.
    curl_global_init(CURL_GLOBAL_DEFAULT);

    // Create the request.
    if (const auto CURLHandle = curl_easy_init())
    {
        // libCURL request-options.
        curl_easy_setopt(CURLHandle, CURLOPT_URL, URL);
        curl_easy_setopt(CURLHandle, CURLOPT_TIMEOUT, 20L);
        curl_easy_setopt(CURLHandle, CURLOPT_NOPROGRESS, 1L);
        curl_easy_setopt(CURLHandle, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(CURLHandle, CURLOPT_WRITEDATA, &Response);
        curl_easy_setopt(CURLHandle, CURLOPT_WRITEFUNCTION, CURLWrite);
        curl_easy_setopt(CURLHandle, CURLOPT_USERAGENT, "AYRIA Desktopclient");

        // Perform the request and catch any errors.
        if (const auto Error = curl_easy_perform(CURLHandle))
        {
            Infoprint(va("HTTP error: %s", curl_easy_strerror(Error)));
        }

        curl_easy_cleanup(CURLHandle);
    }

    return Response;
}

// Read from the disk and internal map.
void Initializemanifeststorage(bool Force = false)
{
    static bool Initialized = false;
    if (Initialized && !Force) return;
    Initialized = true;

    for (const auto &Item : Listfiles("./Manifests", ".json"))
    {
        if (const auto Filebuffer = Readfile(va("./Manifests/%s", Item.c_str())); Filebuffer.size())
        {
            try
            {
                Backend::Manifest_t Localmanifest;
                auto Object = nlohmann::json::parse(Filebuffer.c_str());

                // Parse the JSON manifest into the C++ representation.
                for (const auto &Entry : Object["Games"]) Localmanifest.Games.push_back(Entry);
                for (const auto &Entry : Object["Dependencies"]) Localmanifest.Dependencies.push_back(Entry);
                Localmanifest.Author = Object["Author"].is_null() ? "Unknown" : Object["Author"].get<std::string>();
                Localmanifest.Description = Object["Description"].is_null() ? "" : Object["Description"].get<std::string>();
                Localmanifest.Downloadlink = Object["Downloadlink"].is_null() ? "" : Object["Downloadlink"].get<std::string>();
                Localmanifest.Sourcecodelink = Object["Sourcecodelink"].is_null() ? "Closedsource" : Object["Sourcecodelink"].get<std::string>();
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

// Exported methods.
namespace Backend
{
    // Update the on-disk data.
    Updateresult_t Updatemanifests()
    {
        std::string Versionstring{};

        // Fetch the last commit information.
        if (const auto Response = CURLFetch("https://api.github.com/repos/AyriaPublic/Desktop_cpp/commits/master"); Response.size())
        {
            try { Versionstring = nlohmann::json::parse(Response.c_str())["sha"].get<std::string>(); }
            catch (const std::exception &e) { Infoprint(va("Update-parse error: %s", e.what())); return Updateresult_t::Missingversion; }
        }

        // Check if we already have the latest data.
        if (Fileexists(va("./Downloads/%s", Versionstring.c_str())))
            return Updateresult_t::Missingwork;

        // Fetch the latest repository archive.
        if (const auto Response = CURLFetch("https://codeload.github.com/AyriaPublic/Desktop_cpp/zip/master"); Response.size())
        {
            // Ensure that the directories exists.
            #if defined(_WIN32)
            _mkdir("./Manifests/");
            _mkdir("./Downloads/");
            #else
            mkdir("./Manifests/");
            mkdir("./Downloads/");
            #endif

            // Save the data to disk.
            Writefile(va("./Downloads/%s", Versionstring.c_str()), Response);

            // Update the symlink to the latest version.
            std::remove("./Downloads/Latest.zip");
            #if defined(_WIN32)
            CreateSymbolicLinkA("./Downloads/Latest.zip", Versionstring.c_str(), NULL);
            #else
            symlink("./Downloads/Latest.zip", Versionstring.c_str();
            #endif

            // Extract the manifests and store them to disk.
            for (const auto &Item : Package::Findfiles("/Manifests/"))
            {
                auto Filebuffer = Package::Readfile(Item);
                Writefile(va("./Manifests/%s", Item.substr(Item.find_last_of('/')).c_str()), Filebuffer);
            }

            // Trigger and internal update.
            Initializemanifeststorage(true);
            return Updateresult_t::Sucess;
        }

        return Updateresult_t::Missingdata;
    }

    // Access the manifests from other modules.
    const Manifest_t *Fetchmanifest(size_t Index)
    {
        Initializemanifeststorage();

        if (Index >= Manifeststorage.size()) return nullptr;
        return &Manifeststorage[Index];
    }

    // Search for manifests by criteria.
    namespace Findmanifests
    {
        std::vector<Searchresult_t> byDescription(std::string_view Criteria)
        {
            std::unordered_map<size_t /* ID */, size_t /* Relevancy */> Positives;
            Initializemanifeststorage();

            // Tokenize the input.
            std::vector<std::string> Tokens{ "" };
            for (const auto &Item : Criteria)
            {
                if (Item == ' ' || Item == '.' || Item == ',')
                {
                    // New token.
                    Tokens.emplace_back();
                    continue;
                }

                // Append to last token.
                Tokens.back().push_back(Item);
            }

            // Find how relevant each plugin is.
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

            // Sort the map by relevancy.
            std::vector<Searchresult_t> Results; Results.reserve(Positives.size());
            for (const auto &Item : Positives) Results.push_back({ Item.first, Item.second });
            std::sort(Results.begin(), Results.end(), [](const auto &a, const auto &b) { return a.second > b.second; });

            return Results;
        }
        std::vector<Searchresult_t> byAuthor(std::string_view Criteria)
        {
            std::unordered_map<size_t /* ID */, size_t /* Relevancy */> Positives;
            Initializemanifeststorage();

            // Find how relevant each plugin is.
            std::for_each(Manifeststorage.cbegin(), Manifeststorage.cend(), [&](const auto &Item) -> void
            {
                if (std::strstr(Item.Author.c_str(), Criteria.data()))
                {
                    Positives[Item.Index]++;
                }
            });

            // Sort the map by relevancy.
            std::vector<Searchresult_t> Results; Results.reserve(Positives.size());
            for (const auto &Item : Positives) Results.push_back({ Item.first, Item.second });
            std::sort(Results.begin(), Results.end(), [](const auto &a, const auto &b) { return a.second > b.second; });

            return Results;
        }
        std::vector<Searchresult_t> byName(std::string_view Criteria)
        {
            std::unordered_map<size_t /* ID */, size_t /* Relevancy */> Positives;
            Initializemanifeststorage();

            // Find how relevant each plugin is.
            std::for_each(Manifeststorage.cbegin(), Manifeststorage.cend(), [&](const auto &Item) -> void
            {
                if (std::strstr(Item.Friendlyname.c_str(), Criteria.data()))
                {
                    Positives[Item.Index]++;
                }
            });

            // Sort the map by relevancy.
            std::vector<Searchresult_t> Results; Results.reserve(Positives.size());
            for (const auto &Item : Positives) Results.push_back({ Item.first, Item.second });
            std::sort(Results.begin(), Results.end(), [](const auto &a, const auto &b) { return a.second > b.second; });

            return Results;
        }
        std::vector<Searchresult_t> byGame(std::string_view Criteria)
        {
            std::unordered_map<size_t /* ID */, size_t /* Relevancy */> Positives;
            Initializemanifeststorage();

            // Find how relevant each plugin is.
            std::for_each(Manifeststorage.cbegin(), Manifeststorage.cend(), [&](const auto &Item) -> void
            {
                for (const auto &Entry : Item.Games)
                {
                    if (std::strstr(Entry.c_str(), Criteria.data()))
                    {
                        Positives[Item.Index]++;
                    }
                }
            });

            // Sort the map by relevancy.
            std::vector<Searchresult_t> Results; Results.reserve(Positives.size());
            for (const auto &Item : Positives) Results.push_back({ Item.first, Item.second });
            std::sort(Results.begin(), Results.end(), [](const auto &a, const auto &b) { return a.second > b.second; });

            return Results;
        }
    }
}
