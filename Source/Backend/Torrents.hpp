/*
    Initial author: Convery (tcn@ayria.se)
    Started: 05-05-2018
    License: MIT
    Notes:
        Provides downloads from magnet-links.
*/

#pragma once
#include "../Stdinclude.hpp"

namespace Backend
{
    // Add new torrents to the queue.
    void Addtorrent(std::string Magnetlink);

    // Fetch the active number of torrents.
    size_t Activetorrents();
}
