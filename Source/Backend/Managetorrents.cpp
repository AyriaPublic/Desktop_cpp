/*
    Initial author: Convery (tcn@ayria.se)
    Started: 05-05-2018
    License: MIT
    Notes:
        Provides downloading of magnet-links.
*/

#include "../Stdinclude.hpp"
#include <libtorrent/session.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/error_code.hpp>
#include <libtorrent/magnet_uri.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/torrent_status.hpp>
#include <libtorrent/add_torrent_params.hpp>

libtorrent::session *Session;
std::atomic<size_t> Torrenterrors{};
std::atomic<bool> hasTorrents{ false };
std::unordered_map<uint32_t, bool> Torrenthandles;

// Initialize the session and start processing torrents.
void Initializetorrenting()
{
    static bool Initialized = false;
    if (Initialized) return;
    Initialized = true;

    // The default settings for libtorrent.
    libtorrent::settings_pack Settings;
    Settings.set_int(libtorrent::settings_pack::alert_mask, libtorrent::alert::error_notification | libtorrent::alert::status_notification);

    // Initialize the session.
    Session = new libtorrent::session(Settings);

    // The main torrent loop.
    auto Lambda = []()
    {
        while (true)
        {
            std::vector<libtorrent::alert *> Alerts;
            Session->pop_alerts(&Alerts);

            for (const auto &Item : Alerts)
            {
                if (auto Torrent = libtorrent::alert_cast<libtorrent::add_torrent_alert>(Item))
                {
                    Torrenthandles[Torrent->handle.id()] = true;
                }

                if (libtorrent::alert_cast<libtorrent::torrent_error_alert>(Item))
                {
                    Infoprint(va("Torrenting issue: %s", Item->message().c_str()));
                }

                if (auto Torrent = libtorrent::alert_cast<libtorrent::torrent_finished_alert>(Item))
                {
                    Torrenthandles[Torrent->handle.id()] = false;
                }

                if (auto Torrent = libtorrent::alert_cast<libtorrent::torrent_error_alert>(Item))
                {
                    Torrenthandles[Torrent->handle.id()] = false;
                    Torrenterrors++;
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            Session->post_torrent_updates();
        }

    };
    std::thread(Lambda).detach();
}

// Exported methods.
namespace Backend
{
    // Add new torrents to the queue.
    void Addtorrent(std::string Magnetlink)
    {
        libtorrent::error_code Error;
        libtorrent::add_torrent_params Param;
        libtorrent::parse_magnet_uri(Magnetlink, Param, Error);
        Param.save_path = ".\\Downloads\\";
        Param.upload_limit = 42000;

        hasTorrents = true;
        Initializetorrenting();
        Session->async_add_torrent(Param);
    }

    // Fetch the active number of torrents.
    size_t Torrentcount()
    {
        size_t Result{};

        for (const auto &Item : Torrenthandles)
            if (Item.second)
                Result++;

        return Result;
    }

    // Check if we have added torrents.
    bool Activetorrents()
    {
        return hasTorrents;
    }
}
