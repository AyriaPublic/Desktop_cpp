/*
    Initial author: Convery (tcn@ayria.se)
    Started: 05-05-2018
    License: MIT
    Notes:
        Provides access to the backend workers.
*/

#pragma once
#include "../Stdinclude.hpp"

namespace Backend
{
    // Manifest management.
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
    enum class Updateresult_t
    {
        Sucess,
        Missingversion,
        Missingwork,
        Missingdata,
        Max
    };
    Updateresult_t Updatemanifests();
    const Manifest_t *Fetchmanifest(size_t Index);

    // Find a manifest by criteria.
    namespace Findmanifests
    {
        using Searchresult_t = std::pair<size_t /* ID */, size_t /* Relevancy */>;
        std::vector<Searchresult_t> byDescription(std::string_view Criteria);
        std::vector<Searchresult_t> byAuthor(std::string_view Criteria);
        std::vector<Searchresult_t> byGame(std::string_view Criteria);
        std::vector<Searchresult_t> byName(std::string_view Criteria);
    }

    // Torrent management.
    void Addtorrent(std::string Magnetlink);
    size_t Torrentcount();
    bool Activetorrents();
}
