/*
    Initial author: Convery (tcn@ayria.se)
    Started: 02-05-2018
    License: MIT
    Notes:
        Provides basic filesystem IO.
*/

#pragma once
#include "../Stdinclude.hpp"

inline std::string Readfile(const std::string Path)
{
    if (const auto Filehandle = std::fopen(Path.c_str(), "rb"))
    {
        std::fseek(Filehandle, 0, SEEK_END);
        auto Length = std::ftell(Filehandle);
        std::fseek(Filehandle, 0, SEEK_SET);

        auto Buffer = std::make_unique<char[]>(Length);
        std::fread(Buffer.get(), Length, 1, Filehandle);
        std::fclose(Filehandle);

        return std::string(Buffer.get(), Length);
    }

    return {};
}
inline bool Writefile(const std::string Path, const std::string &Buffer)
{
    if (const auto Filehandle = std::fopen(Path.c_str(), "wb"))
    {
        std::fwrite(Buffer.data(), Buffer.size(), 1, Filehandle);
        std::fclose(Filehandle);
        return true;
    }

    return false;
}
inline bool Fileexists(const std::string Path)
{
    if (const auto Filehandle = std::fopen(Path.c_str(), "rb"))
    {
        std::fclose(Filehandle);
        return true;
    }

    return false;
}

// List all files in a directory.
#if defined(_WIN32)
inline std::vector<std::string> Listfiles(std::string Searchpath, std::string_view Extension)
{
    std::vector<std::string> Filenames{};
    WIN32_FIND_DATAA Filedata;
    HANDLE Filehandle;

    // Append trailing slash, asterisk and extension.
    if (Searchpath.back() != '/') Searchpath.append("/");
    Searchpath.append("*");
    if(Extension.size()) Searchpath.append(Extension);

    // Iterate through the directory.
    Filehandle = FindFirstFileA(Searchpath.c_str(), &Filedata);
    if (Filehandle == (void *)INVALID_HANDLE_VALUE)
    {
        FindClose(Filehandle);
        return Filenames;
    }

    do
    {
        // Respect hidden files and folders.
        if (Filedata.cFileName[0] == '.')
            continue;

        // Add the file to the list.
        if (!(Filedata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            Filenames.push_back(Filedata.cFileName);

    } while (FindNextFileA(Filehandle, &Filedata));

    FindClose(Filehandle);
    return Filenames;
}
#else
inline std::vector<std::string> Listfiles(std::string Searchpath, std::string_view Extension)
{
    std::vector<std::string> Filenames{};
    struct stat Fileinfo;
    dirent *Filedata;
    DIR *Filehandle;

    // Iterate through the directory.
    Filehandle = opendir(Searchpath.c_str());
    while ((Filedata = readdir(Filehandle)))
    {
        // Respect hidden files and folders.
        if (Filedata->d_name[0] == '.')
            continue;

        // Get extended fileinfo.
        std::string Filepath = Searchpath + "/" + Filedata->d_name;
        if (stat(Filepath.c_str(), &Fileinfo) == -1) continue;

        // Add the file to the list.
        if (!(Fileinfo.st_mode & S_IFDIR))
            if (!Extension.size())
                Filenames.push_back(Filedata->d_name);
            else
                if (std::strstr(Filedata->d_name, Extension.data()))
                    Filenames.push_back(Filedata->d_name);
    }
    closedir(Filehandle);

    return std::move(Filenames);
}
#endif
