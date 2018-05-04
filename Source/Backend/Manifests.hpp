/*
    Initial author: Convery (tcn@ayria.se)
    Started: 02-05-2018
    License: MIT
    Notes:
        Provides parsing of the manifests directory.
*/

#pragma once
#include "../Stdinclude.hpp"

namespace Backend
{
    struct Manifest_t
    {
        size_t Index;
        std::string Author;
        std::string Description;
        std::string Friendlyname;
        std::string Downloadlink;
        std::string Sourcecodelink;
        std::vector<std::string> Games;
        std::vector<std::string> Dependencies;
    };

    // Read from the disk and internal map.
    void Initializemanifeststorage();
    const Manifest_t *Fetchmanifest(size_t Index);

    // Update the on-disk storage.
    enum Updateresult_t { Done = 0, Noversion = 1, Nodata = 2 };
    Updateresult_t Updatemanifeststorage();

    // Search for manifests by criteria.
    namespace Findmanifests
    {
        using Searchresult_t = std::pair<size_t /* ID */, size_t /* Relevancy */>;
        std::vector<Searchresult_t> byDescription(std::string_view Criteria);
        std::vector<Searchresult_t> byAuthor(std::string_view Criteria);
        std::vector<Searchresult_t> byGame(std::string_view Criteria);
        std::vector<Searchresult_t> byName(std::string_view Criteria);
    }
}
