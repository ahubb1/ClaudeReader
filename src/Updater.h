//--------------------------------------------------------------------------
//
//    This file is part of ClaudeReader (based on BDReader, Copyright (C) 2014-2016  Vincent Crocher).
//
//    ClaudeReader is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    ClaudeReader is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with ClaudeReader. If not, see <http://www.gnu.org/licenses/>.
//
//---------------------------------------------------------------------------
/**
 * \file Updater.h
 * \brief Updates from the GitHub releases: check at startup (or on demand), download, install and restart.
 *
 * Installed with the installer: the new installer runs silently and starts ClaudeReader again.
 * Portable exe (Windows): the new exe replaces it (the running exe is renamed .old, removed at the next start).
 * Elsewhere (Linux): the release page is opened.
 * The address of the latest release can be changed with the CLAUDEREADER_UPDATE_URL environment variable (tests).
 */
#ifndef UPDATER_H
#define UPDATER_H

#include <string>
#include <functional>

namespace Updater
{
    //! A release of ClaudeReader
    struct Release
    {
        std::string Version;        //!< "1.2" (the tag without its "v")
        std::string Tag, Page;      //!< Tag and web page of the release
        std::string Notes;          //!< What changed since the running version
        std::string AssetName, AssetUrl, Sha256;   //!< The file for this computer (empty: from the release page)
        long long AssetSize=0;
    };

    std::string CurrentVersion();
    //! <0, 0 or >0 as strcmp: "1.10" > "1.9", "1.1" > "1.1-beta.2"
    int CompareVersions(const std::string &a, const std::string &b);
    //! Installed with the installer (its uninstaller is next to the exe), or else a portable exe
    bool IsInstalled();
    //! The latest release (network: not from the user interface thread)
    bool Latest(Release *release, std::string *error);

    //! Preference: check at startup (on by default)
    bool CheckAtStartup();
    void SetCheckAtStartup(bool check);

    //! At startup: removes what a previous update left, then checks in the background if enabled.
    //! quit: closes ClaudeReader (preferences saved) so that the update can replace it
    void Start(std::function<void()> quit);
    //! Help > Check for updates, Settings: the result is always shown (even "up to date")
    void CheckNow();
}

#endif // UPDATER_H
