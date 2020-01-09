/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

// Brand-specific constants and install modes for Brave.

#include <stdlib.h>

#include "chrome/common/chrome_icon_resources_win.h"
#include "chrome/app/chrome_dll_resource.h"
#include "chrome/install_static/install_modes.h"

namespace install_static {

const wchar_t kCompanyPathName[] = L"GabAI";

#if defined(OFFICIAL_BUILD)
const wchar_t kProductPathName[] = L"The Hive";
#else
const wchar_t kProductPathName[] = L"The Hive-Development";
#endif

const size_t kProductPathNameLength = _countof(kProductPathName) - 1;

#if defined(OFFICIAL_BUILD)
const wchar_t kBinariesAppGuid[] = L"{DA2C89DD-D3D6-4E1C-9534-27E0F66759A1}";
#else
const wchar_t kBinariesAppGuid[] = L"";
#endif

#if defined(OFFICIAL_BUILD)
// Brave integrates with Brave Update, so the app GUID above is used.
const wchar_t kBinariesPathName[] = L"";
#else
const wchar_t kBinariesPathName[] = L"The Hive Binaries";
#endif

const char kSafeBrowsingName[] = "chromium";

#if defined(OFFICIAL_BUILD)
// Regarding to install switch, use same value in
// chrome/installer/mini_installer/configuration.cc
const InstallConstants kInstallModes[] = {
    // The primary install mode for stable Brave.
    {
        sizeof(kInstallModes[0]),
        STABLE_INDEX,  // The first mode is for stable/beta/dev.
        "",            // No install switch for the primary install mode.
        L"",           // Empty install_suffix for the primary install mode.
        L"",           // No logo suffix for the primary install mode.
        L"{0C12B489-0B55-4F8B-8EB7-640256ADBE54}",
        L"The Hive",                           // A distinct base_app_name.
        L"The Hive",                           // A distinct base_app_id.
        L"TheHiveHTML",                              // ProgID prefix.
        L"The Hive HTML Document",                    // ProgID description.
        L"{0C12B489-0B55-4F8B-8EB7-640256ADBE54}",  // Active Setup GUID.
        L"{C92FB778-A27E-4FD7-947F-21DA23323F88}",  // CommandExecuteImpl CLSID.
        { 0x6c9646d,
          0x2807,
          0x44c0,
          { 0x97, 0xd2, 0x6d, 0xa0, 0xdb, 0x62, 0x3d,
            0xb4 } },  // Toast activator CLSID.
        { 0x576b31af,
          0x6369,
          0x4b6b,
          { 0x85, 0x60, 0xe4, 0xb2, 0x3, 0xa9, 0x7a,
            0x8b } },  // Elevator CLSID.
        { 0xb7965c30, 0x7d58, 0x4d86, {
          0x9e, 0x18, 0x47, 0x94, 0x25, 0x64, 0x9, 0xee } },
        L"",  // The empty string means "stable".
        ChannelStrategy::ADDITIONAL_PARAMETERS,
        true,  // Supports system-level installs.
        true,  // Supports in-product set as default browser UX.
        true,  // Supports retention experiments.
        true,  // Supported multi-install.
        icon_resources::kApplicationIndex,  // App icon resource index.
        IDR_MAINFRAME,                      // App icon resource id.
        L"S-1-15-2-3251537155-1984446955-2931258699-841473695-1938553385-"
        L"934012149-",  // App container sid prefix for sandbox.
    },
    // A secondary install mode for Brave Beta
    {
        sizeof(kInstallModes[0]),
        BETA_INDEX,     // The mode for the side-by-side beta channel.
        "chrome-beta",  // Install switch.
        L"-Beta",       // Install suffix.
        L"Beta",        // Logo suffix.
        L"",  // A distinct app GUID.
        L"The Hive Beta",                      // A distinct base_app_name.
        L"The HiveBeta",                              // A distinct base_app_id.
        L"TheHiveBTML",                             // ProgID prefix.
        L"The Hive Beta HTML Document",               // ProgID description.
        L"{911586F3-317E-4498-868B-26BFCBD4F5FB}",  // Active Setup GUID.
        L"",                                        // CommandExecuteImpl CLSID.
        { 0x9560028d,
          0xcca,
          0x49f0,
          { 0x8d, 0x47, 0xef, 0x22, 0xbb, 0xc4, 0xb,
            0xa7 } },  // Toast activator CLSID.
        { 0x2313f1cd,
          0x41f3,
          0x4347,
          { 0xbe, 0xc0, 0xd7, 0x22, 0xca, 0x41, 0x2c,
            0x75 } },  // Elevator CLSID.
        { 0xd9d7b102, 0xfc8a, 0x4843, {
          0xac, 0x35, 0x1e, 0xbc, 0xc7, 0xed, 0x12,
          0x3d } },
        L"beta",                                    // Forced channel name.
        ChannelStrategy::FIXED,
        true,   // Supports system-level installs.
        true,   // Supports in-product set as default browser UX.
        true,   // Supports retention experiments.
        false,  // Did not support multi-install.
        icon_resources::kBetaApplicationIndex,  // App icon resource index.
        IDR_X005_BETA,                          // App icon resource id.
        L"S-1-15-2-3251537155-1984446955-2931258699-841473695-1938553385-"
        L"934012150-",  // App container sid prefix for sandbox.
    },
    // A secondary install mode for Brave Dev
    {
        sizeof(kInstallModes[0]),
        DEV_INDEX,     // The mode for the side-by-side dev channel.
        "chrome-dev",  // Install switch.
        L"-Dev",       // Install suffix.
        L"Dev",        // Logo suffix.
        L"",  // A distinct app GUID.
        L"The Hive Dev",                       // A distinct base_app_name.
        L"The HiveDev",                               // A distinct base_app_id.
        L"TheHivetDTML",                             // ProgID prefix.
        L"The Hive Dev HTML Document",                // ProgID description.
        L"{2F0CC5B2-B924-40EA-A441-175E25CD51AA}",  // Active Setup GUID.
        L"",                                        // CommandExecuteImpl CLSID.
        { 0x20b22981,
          0xf63a,
          0x47a6,
          { 0xa5, 0x47, 0x69, 0x1c, 0xc9, 0x4c, 0xae,
            0xe0 } },  // Toast activator CLSID.
        { 0x9129ed6a,
          0x11d3,
          0x43b7,
          { 0xb7, 0x18, 0x8f, 0x82, 0x61, 0x45, 0x97,
            0xa3 } },  // Elevator CLSID.
        { 0x9cf6868c, 0x8c9f, 0x4220,
          { 0x95, 0xbe, 0x13, 0x99, 0x9d, 0xd9, 0x9b,
            0x48 } },
        L"dev",                                     // Forced channel name.
        ChannelStrategy::FIXED,
        true,   // Supports system-level installs.
        true,   // Supports in-product set as default browser UX.
        true,   // Supports retention experiments.
        false,  // Did not support multi-install.
        icon_resources::kDevApplicationIndex,  // App icon resource index.
        IDR_X004_DEV,                          // App icon resource id.
        L"S-1-15-2-3251537155-1984446955-2931258699-841473695-1938553385-"
        L"934012151-",  // App container sid prefix for sandbox.
    },
    // A secondary install mode for Brave SxS (canary).
    {
        sizeof(kInstallModes[0]),
        NIGHTLY_INDEX,  // The mode for the side-by-side nightly channel.
        "chrome-sxs",   // Install switch.
        L"-Nightly",    // Install suffix.
        L"Canary",      // Logo suffix.
        L"",  // A distinct app GUID.
        L"The Hive Nightly",                    // A distinct base_app_name.
        L"The HiveNightly",                            // A distinct base_app_id.
        L"TheHiveSTML",                             // ProgID prefix.
        L"The Hive Nightly HTML Document",             // ProgID description.
        L"{7D06B97B-CBC7-49E6-8440-4A6750F016A6}",  // Active Setup GUID.
        L"{9C42E68C-3620-43B8-A5A7-0C8442B1F36F}",  // CommandExecuteImpl CLSID.
        { 0xf2edbc59,
          0x7217,
          0x4da5,
          { 0xa2, 0x59, 0x3, 0x2, 0xda, 0x6a, 0x0,
            0xe1 } },  // Toast activator CLSID.
        { 0x1ce2f84f,
          0x70cb,
          0x4389,
          { 0x87, 0xdb, 0xd0, 0x99, 0x48, 0x30, 0xbb,
            0x17 } },  // Elevator CLSID.
        { 0x648b3c2b, 0xe53, 0x4085,
          { 0xa9, 0x75, 0x11, 0x18, 0x1, 0x75, 0x8f,
            0xe7 } },
        L"nightly",                                  // Forced channel name.
        ChannelStrategy::FIXED,
        true,   // Support system-level installs.
        true,   // Support in-product set as default browser UX.
        true,   // Supports retention experiments.
        false,  // Did not support multi-install.
        icon_resources::kSxSApplicationIndex,  // App icon resource index.
        IDR_SXS,                               // App icon resource id.
        L"S-1-15-2-3251537155-1984446955-2931258699-841473695-1938553385-"
        L"934012152-",  // App container sid prefix for sandbox.
    },
};
#else
const InstallConstants kInstallModes[] = {
    // The primary (and only) install mode for Brave developer build.
    {
        sizeof(kInstallModes[0]),
        DEVELOPER_INDEX,  // The one and only mode for developer mode.
        "",               // No install switch for the primary install mode.
        L"",              // Empty install_suffix for the primary install mode.
        L"",              // No logo suffix for the primary install mode.
        L"",            // Empty app_guid since no integraion with Brave Update.
        L"The Hive Development",  // A distinct base_app_name.
        L"The HiveDevelopment",   // A distinct base_app_id.
        L"TheHiveDTML",                             // ProgID prefix.
        L"The Hive Development HTML Document",           // ProgID description.
        L"{F7C9648D-ECE3-4BED-B393-BEC1DAB689C5}",  // Active Setup GUID.
        L"{CE2CDB7D-9C25-4223-ABDD-055A08BA4898}",  // CommandExecuteImpl CLSID.
        { 0xeb41c6e8,
          0xba35,
          0x4c06,
          { 0x96, 0xe8, 0x6f, 0x30, 0xf1, 0x8c, 0xa5,
            0x5c } },  // Toast activator CLSID.
        { 0x5693e62d,
          0xd6,
          0x4421,
          { 0xaf, 0xe8, 0x58, 0xf3, 0xc9, 0x47, 0x43,
            0x6a } },  // Elevator CLSID.
        { 0xedf6b466, 0x4efc, 0x4c88,
          { 0x83, 0x8c, 0x96, 0xb4, 0x39, 0x1c, 0x6a,
            0xe } },
        L"",    // Empty default channel name since no update integration.
        ChannelStrategy::UNSUPPORTED,
        true,   // Supports system-level installs.
        true,   // Supports in-product set as default browser UX.
        false,  // Does not support retention experiments.
        true,   // Supported multi-install.
        icon_resources::kApplicationIndex,  // App icon resource index.
        IDR_MAINFRAME,                      // App icon resource id.
        L"S-1-15-2-3251537155-1984446955-2931258699-841473695-1938553385-"
        L"934012148-",  // App container sid prefix for sandbox.
    },
};
#endif

static_assert(_countof(kInstallModes) == NUM_INSTALL_MODES,
              "Imbalance between kInstallModes and InstallConstantIndex");

}  // namespace install_static
