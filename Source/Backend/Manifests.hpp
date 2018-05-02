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
        std::vector<std::string> Games;
        std::vector<std::string> Dependencies;
    };

    void Initializemanifeststorage();
    Manifest_t Fetchmanifest(size_t Index);

    namespace Findmanifests
    {
        std::vector<size_t> byGame(std::string_view Criteria);
        std::vector<size_t> byName(std::string_view Criteria);
        std::vector<size_t> byAuthor(std::string_view Criteria);
        std::vector<size_t> byDescription(std::string_view Criteria);
    }
}
