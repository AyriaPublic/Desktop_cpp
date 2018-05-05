/*
    Initial author: Convery (tcn@ayria.se)
    Started: 05-05-2018
    License: MIT
    Notes:
        Provides an interface for updating the manifests.
*/

#include "../Stdinclude.hpp"

// Download the latest version of the manifests.
void Update(std::vector<Frontend::Argument_t> Arguments)
{
    switch(Backend::Updatemanifests())
    {
        case Backend::Updateresult_t::Sucess: return Infoprint("Updated the storage.");
        case Backend::Updateresult_t::Missingwork: return Infoprint("Already up to date.");
        case Backend::Updateresult_t::Missingdata: return Infoprint("Failed to download the latest archive.");
        case Backend::Updateresult_t::Missingversion: return Infoprint("Failed to download the version ID.");
    }
}

// Add the commands on startup.
namespace
{
    static struct Startup
    {
        Startup()
        {
            Frontend::Registercommand("update", { "Update the plugininformation to the latest version", Update });
        }
    } Loader{};
}
