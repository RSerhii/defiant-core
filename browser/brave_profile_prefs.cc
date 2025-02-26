/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/brave_profile_prefs.h"

#include "brave/common/pref_names.h"
#include "brave/components/brave_shields/browser/brave_shields_web_contents_observer.h"
#include "brave/components/brave_sync/brave_sync_prefs.h"
#include "brave/components/brave_webtorrent/browser/buildflags/buildflags.h"
#include "chrome/browser/net/prediction_options.h"
#include "chrome/browser/prefs/session_startup_pref.h"
#include "chrome/common/pref_names.h"
#include "components/content_settings/core/common/pref_names.h"
#include "components/pref_registry/pref_registry_syncable.h"
#include "components/safe_browsing/common/safe_browsing_prefs.h"
#include "components/signin/public/base/signin_pref_names.h"
#include "components/sync/base/pref_names.h"
#include "extensions/buildflags/buildflags.h"
#include "extensions/common/feature_switch.h"
#include "third_party/widevine/cdm/buildflags.h"

#if BUILDFLAG(BUNDLE_WIDEVINE_CDM)
#include "brave/browser/widevine/brave_widevine_bundle_manager.h"
#endif

#if BUILDFLAG(ENABLE_BRAVE_WEBTORRENT)
#include "brave/components/brave_webtorrent/browser/webtorrent_util.h"
#endif

#if !defined(OS_ANDROID)
#include "chrome/browser/first_run/first_run.h"
#endif

using extensions::FeatureSwitch;

namespace brave {

void RegisterProfilePrefs(user_prefs::PrefRegistrySyncable* registry) {
  brave_shields::BraveShieldsWebContentsObserver::RegisterProfilePrefs(
      registry);

  // appearance
  registry->RegisterBooleanPref(kLocationBarIsWide, false);
  registry->RegisterBooleanPref(kHideBraveRewardsButton, false);

  brave_sync::prefs::Prefs::RegisterProfilePrefs(registry);

  registry->RegisterBooleanPref(kWidevineOptedIn, false);
  registry->RegisterBooleanPref(kAskWidevineInstall, true);
#if BUILDFLAG(BUNDLE_WIDEVINE_CDM)
  BraveWidevineBundleManager::RegisterProfilePrefs(registry);
#endif

  // Default Brave shields
  registry->RegisterBooleanPref(kHTTPSEVerywhereControlType, true);
  registry->RegisterBooleanPref(kNoScriptControlType, false);
  registry->RegisterBooleanPref(kAdControlType, true);
  // > advanced view is defaulted to true for EXISTING users; false for new
  bool is_new_user = false;

  #if !defined(OS_ANDROID)
  is_new_user = first_run::IsChromeFirstRun();
  #endif

  registry->RegisterBooleanPref(kShieldsAdvancedViewEnabled,
                                is_new_user == false);
  registry->RegisterBooleanPref(kFBEmbedControlType, true);
  registry->RegisterBooleanPref(kTwitterEmbedControlType, true);
  registry->RegisterBooleanPref(kLinkedInEmbedControlType, false);

  // WebTorrent
#if BUILDFLAG(ENABLE_BRAVE_WEBTORRENT)
  webtorrent::RegisterProfilePrefs(registry);
#endif

  // Hangouts
  registry->RegisterBooleanPref(kHangoutsEnabled, true);

  // Media Router
  registry->SetDefaultPrefValue(prefs::kEnableMediaRouter, base::Value(false));

  // 1. We do not want to enable the MediaRouter pref directly, so
  // using a proxy pref to handle Media Router setting
  // 2. On upgrade users might have enabled Media Router and the pref should
  // be set correctly, so we use feature switch to set the initial value
#if BUILDFLAG(ENABLE_EXTENSIONS)
  registry->RegisterBooleanPref(kBraveEnabledMediaRouter,
      FeatureSwitch::load_media_router_component_extension()->IsEnabled());
#endif

  // Restore last profile on restart
  registry->SetDefaultPrefValue(
      prefs::kRestoreOnStartup,
      base::Value(SessionStartupPref::kPrefValueLast));

  // Show download prompt by default
  registry->SetDefaultPrefValue(prefs::kPromptForDownload, base::Value(true));

  // Not using chrome's web service for resolving navigation errors
  registry->SetDefaultPrefValue(prefs::kAlternateErrorPagesEnabled,
                                base::Value(false));

  // Disable safebrowsing reporting
  registry->SetDefaultPrefValue(
      prefs::kSafeBrowsingExtendedReportingOptInAllowed, base::Value(false));

  // Disable search suggestion
  registry->SetDefaultPrefValue(prefs::kSearchSuggestEnabled,
                                base::Value(false));

  // Disable "Use a prediction service to load pages more quickly"
  registry->SetDefaultPrefValue(
      prefs::kNetworkPredictionOptions,
      base::Value(chrome_browser_net::NETWORK_PREDICTION_NEVER));

  // Make sync managed to dsiable some UI after password saving.
  registry->SetDefaultPrefValue(syncer::prefs::kSyncManaged, base::Value(true));

  // Disable cloud print
  // Cloud Print: Don't allow this browser to act as Cloud Print server
  registry->SetDefaultPrefValue(prefs::kCloudPrintProxyEnabled,
                                base::Value(false));
  // Cloud Print: Don't allow jobs to be submitted
  registry->SetDefaultPrefValue(prefs::kCloudPrintSubmitEnabled,
                                base::Value(false));

  // Importer: selected data types
  registry->RegisterBooleanPref(prefs::kImportDialogCookies, true);
  registry->RegisterBooleanPref(prefs::kImportDialogStats, true);
  registry->RegisterBooleanPref(prefs::kImportDialogLedger, true);
  registry->RegisterBooleanPref(prefs::kImportDialogWindows, true);
  // Importer: ledger (used for Brave Rewards pinned => tips)
  registry->RegisterIntegerPref(kBravePaymentsPinnedItemCount, 0);

  // IPFS companion extension
  registry->RegisterBooleanPref(kIPFSCompanionEnabled, false);

  // New Tab Page
  registry->RegisterBooleanPref(kNewTabPageShowBackgroundImage, true);
  registry->RegisterBooleanPref(kNewTabPageShowClock, true);
  registry->RegisterBooleanPref(kNewTabPageShowTopSites, true);
  registry->RegisterBooleanPref(kNewTabPageShowStats, true);

  // Brave Wallet
  registry->RegisterStringPref(kBraveWalletAES256GCMSivNonce, "");
  registry->RegisterStringPref(kBraveWalletEncryptedSeed, "");
  registry->RegisterBooleanPref(kBraveWalletEnabled, true);
}

}  // namespace brave
